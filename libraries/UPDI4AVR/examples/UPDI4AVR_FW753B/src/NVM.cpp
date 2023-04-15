/**
 * @file NVM.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "Prototypes.h"
#include <api/capsule.h>

namespace NVM {
  volatile uint32_t nvm_flash_offset    = 0;
  volatile uint32_t nvm_eeprom_offset   = 0;
  volatile uint32_t nvm_user_sig_offset = 0;
  volatile uint16_t flash_page_size     = 0;
  struct fuse_packet_t { uint16_t data; uint16_t addr; };
  static uint8_t set_ptr[] = {
      UPDI::UPDI_SYNCH
    , UPDI::UPDI_ST | UPDI::UPDI_PTR_REG | UPDI::UPDI_DATA3
    , 0, 0, 0, 0        // $02:long address
  };
  static uint8_t set_repeat[] = {
      UPDI::UPDI_SYNCH
    , UPDI::UPDI_REPEAT | UPDI::UPDI_DATA1
    , 0                 // $03:repeat count
    , UPDI::UPDI_SYNCH
    , UPDI::UPDI_ST | UPDI::UPDI_PTR_INC | UPDI::UPDI_DATA1
  };
  static uint8_t set_repeat_rsd[] = {
      UPDI::UPDI_SYNCH
    , UPDI::UPDI_STCS    | UPDI::UPDI_CS_CTRLA
    , UPDI::UPDI_SET_RSD | UPDI::UPDI_SET_GTVAL_2
    , UPDI::UPDI_SYNCH
    , UPDI::UPDI_REPEAT  | UPDI::UPDI_DATA1
    , 0                 // $05:repeat count
    , UPDI::UPDI_SYNCH
    , UPDI::UPDI_ST | UPDI::UPDI_PTR_INC | UPDI::UPDI_DATA2
  };
}

/*
 * メモリ読込中核
 */

bool NVM::read_memory (void) {
  uint8_t  mem_type   =         JTAG2::packet.body[1];
  size_t   byte_count = _CAPS16(JTAG2::packet.body[2])->word;
  uint32_t start_addr = _CAPS32(JTAG2::packet.body[6])->dword;
  JTAG2::packet.body[0] = JTAG2::RSP_MEMORY;
  JTAG2::packet.size_word[0] = byte_count + 1;
  /* 奇数量なら絶対アドレス（データ長は常に1） */
  /* 偶数量なら間接アドレス指定 */
  /* Flash領域は常に偶数量 */
  if (byte_count >= 2) {
    switch (mem_type) {
      /* Data 領域 */
      /* これは常に絶対アドレス */
      case JTAG2::MTYPE_XMEGA_FLASH : {   // 0xC0
        start_addr -= NVM::nvm_flash_offset;
      }
      /* Flash 領域 */
      case JTAG2::MTYPE_BOOT_FLASH : {    // 0xC1
        start_addr += NVM::nvm_flash_offset;
        return NVM::read_flash(start_addr, byte_count);
      }
      /* EEPROM 領域 */
      case JTAG2::MTYPE_EEPROM :          // 0x22
      case JTAG2::MTYPE_EEPROM_XMEGA : {  // 0xC4
        start_addr += NVM::nvm_eeprom_offset;
        break;
      }
      /* USERROW/USERSIG */
      case JTAG2::MTYPE_USERSIG : {       // 0xC5
        start_addr += NVM::nvm_user_sig_offset;
        break;
      }
      /* FUSE他 */
      /* 常に絶対アドレス */
      case JTAG2::MTYPE_FUSE_BITS :       // 0xB2
      case JTAG2::MTYPE_LOCK_BITS :       // 0xB3
      case JTAG2::MTYPE_SIGN_JTAG :       // 0xB4
      case JTAG2::MTYPE_OSCCAL_BYTE :     // 0xB5
      case JTAG2::MTYPE_PRODSIG : {       // 0xC6
        break;
      }
      default : return false;
    }
  }
  return NVM::read_data(start_addr, byte_count);
}

/*
 * メモリ書込中核
 */

bool NVM::write_memory (void) {
  uint8_t  mem_type   =         JTAG2::packet.body[1];
  size_t   byte_count = _CAPS16(JTAG2::packet.body[2])->word;
  uint32_t start_addr = _CAPS32(JTAG2::packet.body[6])->dword;
  /* 奇数量なら絶対アドレス（データ長は常に1） */
  /* 偶数量なら間接アドレス指定 */
  /* Flash領域は常に偶数量 */
  if (byte_count >= 2) {
    switch (mem_type) {
      /* Flash 領域 */
      /* 奇数量指定は Data領域絶対アドレス指定になる */
      case JTAG2::MTYPE_XMEGA_FLASH :   // 0xC0
      case JTAG2::MTYPE_BOOT_FLASH : {  // 0xC1
        start_addr += NVM::nvm_flash_offset;
        if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN3_bp))
          return NVM::write_flash_v3(start_addr, byte_count);
        if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp))
          return NVM::write_flash_v2(start_addr, byte_count);
        return NVM::write_flash(start_addr, byte_count);
      }
      /* EEPROM 領域 */
      case JTAG2::MTYPE_EEPROM :        // 0x22
      case JTAG2::MTYPE_EEPROM_XMEGA :  // 0xC4
      case JTAG2::MTYPE_LOCK_BITS : {   // 0xB3
        start_addr += NVM::nvm_eeprom_offset;
        break;
      }
      /* USERROW/USERSIG 領域 */
      case JTAG2::MTYPE_USERSIG : {     // 0xC5
        start_addr += NVM::nvm_user_sig_offset;
        /* AVR_DA/DB/DD/EA is Flash */
        /* この系統は Flash として実装されている */

        /* NVMCTRL v3 */
        if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN3_bp)) {
          /* 書く前にページ消去 */
            NVM::nvm_wait_v3();
          if (!UPDI::st8(start_addr, 0xFF)) return false;
          if (!NVM::nvm_ctrl_v3(NVM::NVM_V2_CMD_FLPER)) return false;
          UPDI_CONTROL |= _BV(UPDI::UPDI_ERFM_bp);
          return NVM::write_flash_v3(start_addr, byte_count);
        }

        /* NVMCTRL v2 */
        if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp)) {
          /* 書く前にページ消去 */
          if (!NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_FLPER)) return false;
          if (!UPDI::st8(start_addr, 0xFF)) return false;
          UPDI_CONTROL |= _BV(UPDI::UPDI_ERFM_bp);
          return NVM::write_flash_v2(start_addr, byte_count);
        }
        /* megaAVR/tinyAVR is EEPROM */
        /* この系統は EEPROM として実装されている */
        break;
      }
      default : return false;
    }
  }
  else {
    /* ここは 1byte単位書込で通る */
    uint8_t data = JTAG2::packet.body[10];
    switch (mem_type) {
      /* FUSES/LOCKBITS 領域 */
      /* 常に奇数量絶対アドレス指定の特殊書込 */
      case JTAG2::MTYPE_LOCK_BITS :     // 0xB3
      case JTAG2::MTYPE_FUSE_BITS : {   // 0xB2
        if (UPDI::ld8(start_addr) == data && UPDI_LASTH == 0) return true;
        if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN3_bp))
          return NVM::write_fuse_v3(start_addr, data);
        if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp))
          return NVM::write_fuse_v2(start_addr, data);
        return NVM::write_fuse(start_addr, data);
      }
      /* USERROW/USERSIG 領域 */
      case JTAG2::MTYPE_USERSIG : {     // 0xC5
        /* ここは terminal mode の場合のみ通過するはず */
        /* AVR_DA/DB/DD/EA is Flash */
        /* この系統は Flash として実装されている */

        /* NVMCTRL v3 */
        if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN3_bp)) {
          /* アドレス先頭かつ0xFF書込ならページ消去 */
          if (((uint8_t)start_addr & 63) == 0 && data == 0xFF) {
            NVM::nvm_wait_v3();
            if (!UPDI::st8(start_addr, 0xFF)) return false;
            if (!NVM::nvm_ctrl_v3(NVM::NVM_V3_CMD_FLPER)) return false;
            return true;
          }
          /* Flash は偶数単位でしか書けないため偶数化する */
          /* 他方のバイトには0xFFを補完 */
          if ((uint8_t)start_addr & 1) {
            JTAG2::packet.body[11] = data;
            JTAG2::packet.body[10] = 0xFF;
            (uint8_t)start_addr--;
          }
          else {
            JTAG2::packet.body[11] = 0xFF;
          }
          byte_count = 2;
          UPDI_CONTROL |= _BV(UPDI::UPDI_ERFM_bp);
          return NVM::write_flash_v3(start_addr, byte_count);
        }

        /* NVMCTRL v2 */
        if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp)) {
          /* アドレス先頭かつ0xFF書込ならページ消去 */
          if (((uint8_t)start_addr & 63) == 0 && data == 0xFF) {
            if (!NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_FLPER)) return false;
            if (!UPDI::st8(start_addr, 0xFF)) return false;
            return true;
          }
          /* Flash は偶数単位でしか書けないため偶数化する */
          /* 他方のバイトには0xFFを補完 */
          if ((uint8_t)start_addr & 1) {
            JTAG2::packet.body[11] = data;
            JTAG2::packet.body[10] = 0xFF;
            (uint8_t)start_addr--;
          }
          else {
            JTAG2::packet.body[11] = 0xFF;
          }
          byte_count = 2;
          UPDI_CONTROL |= _BV(UPDI::UPDI_ERFM_bp);
          return NVM::write_flash_v2(start_addr, byte_count);
        }
        /* megaAVR/tinyAVR is EEPROM */
        /* この系統は EEPROM として実装されている */
      }
    }
  }
  if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN3_bp))
    return NVM::write_eeprom_v3(start_addr, byte_count);
  if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp))
    return NVM::write_eeprom_v2(start_addr, byte_count);
  return NVM::write_eeprom(start_addr, byte_count);
}

/*
 * Flash領域ワード読込
 */

bool NVM::read_flash (uint32_t start_addr, size_t byte_count) {
  uint8_t* p = &JTAG2::packet.body[1];
  byte_count >>= 1;
  if (byte_count == 0 || byte_count > 256) return false;
  if (!UPDI::send_repeat_header(
    (UPDI::UPDI_LD | UPDI::UPDI_DATA2),
    start_addr,
    byte_count
  )) return false;
  do {
    *p++ = UPDI::RECV();
    *p++ = UPDI::RECV();
  } while (--byte_count);
  return true;
}

/*
 * データ領域バイト読込
 */

bool NVM::read_data (uint32_t start_addr, size_t byte_count) {
  uint8_t* p = &JTAG2::packet.body[1];
  if (byte_count == 0 || byte_count > 256) return false;
  if (!UPDI::send_repeat_header(
    (UPDI::UPDI_LD | UPDI::UPDI_DATA1),
    start_addr, byte_count)) return false;
  do { *p++ = UPDI::RECV(); } while (--byte_count);
  return true;
}

/*
 * NVMCTRL処理待機
 */

/* NVMCTRL v0 */
/* NVMCTRL v2 */
uint8_t NVM::nvm_wait (void) {
  while (UPDI::ld8(NVM::NVMCTRL_REG_STATUS) & 3) delay_micros(50);
  return UPDI_LASTL;
}

/* NVMCTRL v3 */
uint8_t NVM::nvm_wait_v3 (void) {
  while (UPDI::ld8(NVM::NVMCTRL_V3_REG_STATUS) & 3) delay_micros(50);
  return UPDI_LASTL;
}

/*
 * NVMCTRL制御
 */

/* NVMCTRL v0 */
bool NVM::nvm_ctrl (uint8_t nvmcmd) {
  return UPDI::st8(NVM::NVMCTRL_REG_CTRLA, nvmcmd);
}

bool nvm_ctrl_change (uint8_t nvmcmd) {
  if (UPDI::ld8(NVM::NVMCTRL_REG_CTRLA) == nvmcmd) return true;
  if (!NVM::nvm_ctrl(NVM::NVM_V2_CMD_NOCMD)) return false;
  if (NVM::NVM_V2_CMD_NOCMD != nvmcmd) return NVM::nvm_ctrl(nvmcmd);
  return true;
}

/* NVMCTRL v2 */
bool NVM::nvm_ctrl_v2 (uint8_t nvmcmd) {
  NVM::nvm_wait();
  return nvm_ctrl_change(nvmcmd);
}

/* NVMCTRL v3 */
bool NVM::nvm_ctrl_v3 (uint8_t nvmcmd) {
  NVM::nvm_wait_v3();
  return nvm_ctrl_change(nvmcmd);
}

/*
 * FUSE書込
 */

/* NVMCTRL v0 */
bool NVM::write_fuse (uint16_t addr, uint8_t data) {
  fuse_packet_t fuse_packet;
  fuse_packet.data = data;
  fuse_packet.addr = addr;
  NVM::nvm_wait();
  if (!UPDI::sts8(NVM::NVMCTRL_REG_DATA,
    (uint8_t*)&fuse_packet, sizeof(fuse_packet))) return false;
  if (!NVM::nvm_ctrl(NVM::NVM_CMD_WFU)) return false;
  return ((NVM::nvm_wait() & 7) == 0);
}

/* NVMCTRL v2 */
bool NVM::write_fuse_v2 (uint16_t addr, uint8_t data) {
  if (!NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_EEERWR)) return false;
  if (!UPDI::st8(addr, data)) return false;
  bool _r = (NVM::nvm_wait() & 0x70) == 0;
  if (!NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_NOCMD)) return false;
  return _r;
}

/* NVMCTRL v3 */
bool NVM::write_fuse_v3 (uint16_t addr, uint8_t data) {
  if (!NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_NOCMD)) return false;
  if (!UPDI::st8(addr, data)) return false;
  bool _r = (NVM::nvm_wait_v3() & 0x70) == 0;
  if (!NVM::nvm_ctrl_v3(NVM::NVM_V3_CMD_EEPERW)) return false;
  return _r;
}

/*
 * EEPROM領域バイト書込
 */

/* NVMCTRL v0 */
bool NVM::write_eeprom (uint32_t start_addr, size_t byte_count) {
  if (byte_count == 0 || byte_count > 256) return false;

  /* NVMCTRL page buffer clear */
  NVM::nvm_wait();
  if (!NVM::nvm_ctrl(NVM::NVM_CMD_PBC)) return false;
  NVM::nvm_wait();

  /* setting register pointer */
  _CAPS32(set_ptr[2])->dword = start_addr;
  set_repeat[2] = (uint8_t)byte_count - 1;
  if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) return false;
  if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  if (!UPDI::send_bytes(set_repeat, sizeof(set_repeat))) return false;

  /* page buffer stored */
  uint8_t* p = &JTAG2::packet.body[10];
  do {
    if (!UPDI::SEND(*p++)) return false;
    if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  } while (--byte_count);

  /* NVMCTRL write page and complete */
  if (!NVM::nvm_ctrl(NVM::NVM_CMD_ERWP)) return false;
  return NVM::nvm_wait() == 0;
}

/* NVMCTRL v2 */
bool NVM::write_eeprom_v2 (uint32_t start_addr, size_t byte_count) {
  if (byte_count == 0 || byte_count > 256) return false;

  if (!NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_EEERWR)) return false;

  /* setting register pointer */
  _CAPS32(set_ptr[2])->dword = start_addr;
  set_repeat[2] = (uint8_t)byte_count - 1;
  if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) return false;
  if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  if (!UPDI::send_bytes(set_repeat, sizeof(set_repeat))) return false;

  /* page buffer stored */
  uint8_t* p = &JTAG2::packet.body[10];
  do {
    if (!UPDI::SEND(*p++)) return false;
    if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  } while (--byte_count);

  return NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_NOCMD);
}

/* NVMCTRL v3 */
bool NVM::write_eeprom_v3 (uint32_t start_addr, size_t byte_count) {
  if (byte_count == 0 || byte_count > 256) return false;

  if (!NVM::nvm_ctrl_v3(NVM::NVM_V3_CMD_NOCMD)) return false;

  /* setting register pointer */
  _CAPS32(set_ptr[2])->dword = start_addr;
  set_repeat[2] = (uint8_t)byte_count - 1;
  if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) return false;
  if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  if (!UPDI::send_bytes(set_repeat, sizeof(set_repeat))) return false;

  /* page buffer stored */
  uint8_t* p = &JTAG2::packet.body[10];
  do {
    if (!UPDI::SEND(*p++)) return false;
    if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  } while (--byte_count);

  return NVM::nvm_ctrl_v3(NVM::NVM_V3_CMD_EEPERW);
}

/*
 * Flash領域ワード書込
 */

/* NVMCTRL v0 */
bool NVM::write_flash (uint32_t start_addr, size_t byte_count) {
  byte_count >>= 1;
  if (byte_count == 0 || byte_count > 256) return false;

  /* この系統ではページ消去を書込と同時に行える */
  /* NVMCTRL page buffer clear */
  NVM::nvm_wait();
  if (!NVM::nvm_ctrl(NVM::NVM_CMD_PBC)) return false;
  NVM::nvm_wait();

  /* setting register pointer and enable RSD mode */
  _CAPS32(set_ptr[2])->dword = start_addr;
  set_repeat_rsd[5] = byte_count - 1;
  if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) return false;
  if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  if (!UPDI::send_bytes(set_repeat_rsd, sizeof(set_repeat_rsd))) return false;

  /* page buffer stored */
  uint8_t* p = &JTAG2::packet.body[10];
  do {
    UPDI::SEND(*p++);
    UPDI::SEND(*p++);
  } while (--byte_count);
  // UPDI::guardtime();

  /* disable RSD mode */
  if (!UPDI::set_cs_ctra(UPDI::UPDI_SET_GTVAL_2)) return false;

  /* NVMCTRL write page and complete */
  if (!NVM::nvm_ctrl(NVM::NVM_CMD_ERWP)) return false;
  return NVM::nvm_wait() == 0;
}

/* NVMCTRL v2 */
bool NVM::write_flash_v2 (uint32_t start_addr, size_t byte_count) {
  byte_count >>= 1;
  if (byte_count == 0 || byte_count > 256) return false;

  /* チップ消去していない場合はセクター消去 */
  /* ただしページ境界先頭がアドレスされた場合に限る */
  if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_ERFM_bp)
  && ((uint16_t)start_addr & (NVM::flash_page_size - 1)) == 0) {
    if (!NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_FLPER)) return false;
    if (!UPDI::st8(start_addr, 0xFF)) return false;
  }
  if (!NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_FLWR)) return false;

  /* setting register pointer and enable RSD mode */
  _CAPS32(set_ptr[2])->dword = start_addr;
  set_repeat_rsd[5] = byte_count - 1;
  if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) return false;
  if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  if (!UPDI::send_bytes(set_repeat_rsd, sizeof(set_repeat_rsd))) return false;

  /* page buffer stored */
  uint8_t* p = &JTAG2::packet.body[10];
  do {
    UPDI::SEND(*p++);
    UPDI::SEND(*p++);
  } while (--byte_count);
  // UPDI::guardtime();

  /* disable RSD mode */
  if (!UPDI::set_cs_ctra(UPDI::UPDI_SET_GTVAL_2)) return false;

  return NVM::nvm_ctrl_v2(NVM::NVM_V2_CMD_NOCMD);
}

/* NVMCTRL v3 */
bool NVM::write_flash_v3 (uint32_t start_addr, size_t byte_count) {
  byte_count >>= 1;
  if (byte_count == 0 || byte_count > 256) return false;

  /* チップ消去していない場合はセクター消去 */
  /* ただしページ境界先頭がアドレスされた場合に限る */
  if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_ERFM_bp)
  && ((uint16_t)start_addr & (NVM::flash_page_size - 1)) == 0) {
    NVM::nvm_wait_v3();
    if (!UPDI::st8(start_addr, 0xFF)) return false;
    if (!NVM::nvm_ctrl_v3(NVM::NVM_V3_CMD_FLPER)) return false;
  }
  if (!NVM::nvm_ctrl_v3(NVM::NVM_V3_CMD_FLPBCLR)) return false;

  /* setting register pointer and enable RSD mode */
  _CAPS32(set_ptr[2])->dword = start_addr;
  set_repeat_rsd[5] = byte_count - 1;
  if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) return false;
  if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  if (!UPDI::send_bytes(set_repeat_rsd, sizeof(set_repeat_rsd))) return false;

  /* page buffer stored */
  uint8_t* p = &JTAG2::packet.body[10];
  do {
    UPDI::SEND(*p++);
    UPDI::SEND(*p++);
  } while (--byte_count);
  // UPDI::guardtime();

  /* disable RSD mode */
  if (!UPDI::set_cs_ctra(UPDI::UPDI_SET_GTVAL_2)) return false;

  return NVM::nvm_ctrl_v3(NVM::NVM_V3_CMD_FLPW);
}

// end of code

/**
 * @file NVM.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.3
 * @date 2023-12-24
 *
 * @copyright Copyright (c) 2023 askn37 at github.com
 *
 */
#include "Prototypes.h"
#include <api/capsule.h>

namespace NVM {
  /*** Local functions ***/

  bool nvm_ctrl_change (uint8_t nvmcmd);
  bool nvm_ctrl (uint8_t nvmcmd);
  bool nvm_ctrl_v2 (uint8_t nvmcmd);
  bool nvm_ctrl_v3 (uint8_t nvmcmd);
  uint8_t nvm_wait (void);
  uint8_t nvm_wait_v3 (void);

  bool write_fuse (uint16_t addr, uint8_t data);
  uint16_t before_addr = ~0;

  bool check_pagesize (uint16_t seed, uint16_t test) {
    while (test != seed) {
      seed >>= 1;
      if (seed < 2) return false;
    }
    return true;
  }

  /*********************
   * NVMCTRL operation *
   *********************/

  /* NVMCTRL version 0,2 */
  uint8_t nvm_wait (void) {
    #ifdef ENABLE_DEBUG_UPDI_SENDER
    uint16_t _back = UPDI::_send_ptr;
    #endif
    while (UPDI::ld8(NVMCTRL_REG_STATUS) & 3) {
      #ifdef ENABLE_DEBUG_UPDI_SENDER
      UPDI::_send_ptr = _back;
      #endif
      TIM::delay_50us();
    }
    return UPDI_LASTL;
  }

  /* NVMCTRL version 3,4,5 */
  uint8_t nvm_wait_v3 (void) {
    #ifdef ENABLE_DEBUG_UPDI_SENDER
    uint16_t _back = UPDI::_send_ptr;
    #endif
    while (UPDI::ld8(NVMCTRL_V3_REG_STATUS) & 3) {
      #ifdef ENABLE_DEBUG_UPDI_SENDER
      UPDI::_send_ptr = _back;
      #endif
      TIM::delay_50us();
    }
    return UPDI_LASTL;
  }

  /* NVMCTRL version 0,2,3,4,5 */
  bool nvm_ctrl (uint8_t nvmcmd) {
    return UPDI::st8(NVMCTRL_REG_CTRLA, nvmcmd);
  }

  /* NVMCTRL version 2,3,4,5 */
  bool nvm_ctrl_change (uint8_t nvmcmd) {
    if (UPDI::ld8(NVMCTRL_REG_CTRLA) == nvmcmd) return true;
    if (!nvm_ctrl(NVM_CMD_NOCMD)) return false;
    if (NVM_CMD_NOCMD != nvmcmd) return nvm_ctrl(nvmcmd);
    return true;
  }

  /* NVMCTRL version 2 */
  bool nvm_ctrl_v2 (uint8_t nvmcmd) {
    nvm_wait();
    return nvm_ctrl_change(nvmcmd);
  }

  /* NVMCTRL version 3,4,5 */
  bool nvm_ctrl_v3 (uint8_t nvmcmd) {
    nvm_wait_v3();
    return nvm_ctrl_change(nvmcmd);
  }

  /**********************************************
   * FUSE region write (NVMCTRL version 0 only) *
   **********************************************/

  bool write_fuse (uint16_t addr, uint8_t data) {
    /* Version 0 of FUSE can write one byte at a time in a special way */
    struct fuse_packet_t { uint16_t data; uint16_t addr; } fuse_packet;
    fuse_packet.data = data;
    fuse_packet.addr = addr;
    nvm_wait();
    if (!UPDI::sts8(NVMCTRL_REG_DATA,
      (uint8_t*)&fuse_packet, sizeof(fuse_packet))) return false;
    if (!nvm_ctrl(NVM_CMD_WFU)) return false;
    return ((nvm_wait() & 7) == 0);
  }

  /***********************************
   * EEPROM region word type writing *
   ***********************************/

  bool write_eeprom_v4 (uint32_t start_addr, uint8_t *data, size_t byte_count) {
    /* NVMCTRL version 4 */
    /* This version cannot be written in bulk transfer */
    /* Only 2 bytes (1 word) can be written at a time */
    if (byte_count > 2) {
      /* If the limit is exceeded, it will fall back and force a Single-byte write */
      set_response(JTAG2::RSP_ILLEGAL_MEMORY_RANGE);
      return true;
    }
    if (!nvm_ctrl_v3(NVM_V2_CMD_EEERWR)) return false;

    if (byte_count == 1) UPDI::st8(start_addr, *data);
    else UPDI::sts8(start_addr, data, byte_count);

    return nvm_ctrl_v3(NVM_V2_CMD_NOCMD);
  }

  bool write_eeprom_v3 (uint32_t start_addr, uint8_t *data, size_t byte_count) {
    /* NVMCTRL version 3 or 5 */
    /* This version can write 8 bytes in bulk */
    if (byte_count > 8) {
      /* If the limit is exceeded, it will fall back and force a Single-byte write */
      set_response(JTAG2::RSP_ILLEGAL_MEMORY_RANGE);
      return true;
    }
    if (!nvm_ctrl_v3(NVM_V3_CMD_EEPBCLR)) return false;

    if (byte_count == 1) UPDI::st8(start_addr, *data);
    else UPDI::sts8(start_addr, data, byte_count);

    return nvm_ctrl_v3(NVM_V3_CMD_EEPERW);
  }

  bool write_eeprom_v2 (uint32_t start_addr, uint8_t *data, size_t byte_count) {
    /* NVMCTRL version 2 */
    /* This version cannot be written in bulk transfer */
    /* Only 2 bytes (1 word) can be written at a time */
    if (byte_count > 2) {
      /* If the limit is exceeded, it will fall back and force a Single-byte write */
      set_response(JTAG2::RSP_ILLEGAL_MEMORY_RANGE);
      return true;
    }
    if (!nvm_ctrl_v2(NVM_V2_CMD_EEERWR)) return false;

    if (byte_count == 1) UPDI::st8(start_addr, *data);
    else UPDI::sts8(start_addr, data, byte_count);

    return nvm_ctrl_v2(NVM_V2_CMD_NOCMD);
  }

  bool write_eeprom_v0 (uint32_t start_addr, uint8_t *data, size_t byte_count) {
    /* NVMCTRL version 0 */
    /* This version allows fast bulk writes of 32 or 64 bytes */
    if (byte_count > 64) {
      /* If the limit is exceeded, it will fall back and force a Single-byte write */
      set_response(JTAG2::RSP_ILLEGAL_MEMORY_RANGE);
      return true;
    }
    nvm_wait();

    if (byte_count == 1) UPDI::st8(start_addr, *data);
    else UPDI::sts8rsd(start_addr, data, byte_count);

    return nvm_ctrl(NVM_CMD_ERWP);
  }

  /**********************************
   * Flash region word type writing *
   **********************************/

  /*** No version guarantees the results of non-word writes that cross page boundaries ***/

  bool write_flash_v4 (uint32_t start_addr, uint8_t *data, size_t byte_count, bool is_bound) {
    /* NVMCTRL version 4 */
    /* If the chip is not erased, erase the page. */
    /* However, only when the beginning of the page boundary is addressed */
    if (is_bound) {
      if (!nvm_ctrl_v3(NVM_V2_CMD_FLPER)) return false;
      if (!UPDI::st8(start_addr, 0xFF)) return false;
    }
    if (!nvm_ctrl_v3(NVM_V2_CMD_FLWR)) return false;

    /* This version allows bulk writes of 512 bytes */
    if (byte_count == 1) UPDI::st8(start_addr, *data);
    else if ((byte_count - 1) >> 8) UPDI::sts16rsd(start_addr, data, byte_count);
    else UPDI::sts8rsd(start_addr, data, byte_count);

    return nvm_ctrl_v3(NVM_V2_CMD_NOCMD);
  }

  bool write_flash_v3 (uint32_t start_addr, uint8_t *data, size_t byte_count, bool is_bound) {
    /* NVMCTRL version 3 or 5 */
    /* If the chip is not erased, erase the page. */
    /* However, only when the beginning of the page boundary is addressed */
    if (is_bound) {
      nvm_wait_v3();
      if (!UPDI::st8(start_addr, 0xFF)) return false;
      if (!nvm_ctrl_v3(NVM_V3_CMD_FLPER)) return false;
    }
    else if (!nvm_ctrl_v3(NVM_V3_CMD_FLPBCLR)) return false;
    nvm_wait_v3();

    if (byte_count == 1) UPDI::st8(start_addr, *data);
    else UPDI::sts8rsd(start_addr, data, byte_count);

    return nvm_ctrl_v3(NVM_V3_CMD_FLPW);
  }

  bool write_flash_v2 (uint32_t start_addr, uint8_t *data, size_t byte_count, bool is_bound) {
    /* NVMCTRL version 2 */
    /* If the chip is not erased, erase the page. */
    /* However, only when the beginning of the page boundary is addressed */
    if (is_bound) {
      if (!nvm_ctrl_v2(NVM_V2_CMD_FLPER)) return false;
      if (!UPDI::st8(start_addr, 0xFF)) return false;
    }
    if (!nvm_ctrl_v2(NVM_V2_CMD_FLWR)) return false;

    /* This version allows bulk writes of 512 bytes */
    if (byte_count == 1) UPDI::st8(start_addr, *data);
    else if ((byte_count - 1) >> 8) UPDI::sts16rsd(start_addr, data, byte_count);
    else UPDI::sts8rsd(start_addr, data, byte_count);

    return nvm_ctrl_v2(NVM_V2_CMD_NOCMD);
  }

  bool write_flash_v0 (uint32_t start_addr, uint8_t *data, size_t byte_count, bool is_bound) {
    /* NVMCTRL version 0 */
    /* This version does not require page erasure. */
    /* Just clearing the buffer buffer is sufficient. */
    if (is_bound) {
      nvm_wait();
      if (!nvm_ctrl(NVM_CMD_PBC)) return false;
    }
    nvm_wait();

    if (byte_count == 1) UPDI::st8(start_addr, *data);
    else UPDI::sts8rsd(start_addr, data, byte_count);

    return nvm_ctrl(NVM_CMD_ERWP);
  }
}

/*** Global functions ***/

/***********************
 * Memory reading core *
 ***********************/

bool NVM::read_memory (uint32_t start_addr, size_t byte_count) {
  JTAG2::packet.body[JTAG2::MESSAGE_ID] = JTAG2::RSP_MEMORY;
  uint8_t *data = &JTAG2::packet.body[JTAG2::RSP_DATA];

  /* Reads from 1 to 256 bytes and even bytes 258 to 512 are allowed */
  if (byte_count == 0 || byte_count > 512 || (byte_count > 256 && byte_count & 1)) {
    set_response(JTAG2::RSP_ILLEGAL_MEMORY_RANGE);
    return true;
  }
  JTAG2::packet.size_word[0] = byte_count + 1;

  /* Reading only 1 byte may be special */
  if (byte_count == 1) {
    #ifdef ENABLE_ADDFEATS_LOCK_SIG
    if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_PROG_bp)
      && JTAG2::packet.body[JTAG2::MEM_TYPE] == JTAG2::MTYPE_SIGN_JTAG) {
      /* Signature reading branches to special processing */
      JTAG2::packet.body[JTAG2::RSP_DATA] = JTAG2::updi_desc.signature[(uint8_t)start_addr & 3];
      return true;
    }
    #endif
    #ifdef ENABLE_ADDFEATS_DUMP_SIB
    if (bit_is_set(UPDI_CONTROL, UPDI::UPDI_INFO_bp)
      && (_CAPS32(start_addr)->bytes[2] & 0x80) == 0
      && (uint16_t)start_addr < sizeof(JTAG2::updi_desc.sib)) {
      /* If the specified address is the lowest 32 bytes, return SIB */
      *data = JTAG2::updi_desc.sib[(uint8_t)start_addr];
      return true;
    }
    #endif
  }

  if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_PROG_bp)) {
    /* A normal read when the device is locked returns a dummy. */
    do { *data++ = 0xFF; } while (--byte_count);
    return true;
  }

  if ((byte_count - 1) >> 8)
    return UPDI::lds16(start_addr, data, byte_count);
  else
    return UPDI::lds8(start_addr, data, byte_count);
}

/******************
 * NVM write core *
 ******************/

bool NVM::write_memory (void) {
  uint8_t mem_type = JTAG2::packet.body[JTAG2::MEM_TYPE];
  uint8_t *data = &JTAG2::packet.body[JTAG2::DATA_START];
  size_t byte_count = _CAPS16(JTAG2::packet.body[JTAG2::DATA_LENGTH])->word;
  uint32_t start_addr = _CAPS32(JTAG2::packet.body[JTAG2::DATA_ADDRESS])->dword;

  /* Address specification outside the processing range is considered an IO area operation */
  if (start_addr >> 24) {
    start_addr &= 0xFFFF;
    mem_type = JTAG2::MTYPE_SRAM;
  }

  /* Can only be written to USERROW on locked devices */
  /* This write is only allowed in multiples of 32 bytes */
  if (bit_is_set(UPDI_CONTROL, UPDI::UPDI_INFO_bp)
   && mem_type == JTAG2::MTYPE_XMEGA_USERSIG  // 0xC5
   && start_addr <  NVM::BASE45_BOOTROW       // 0x10ff
   && start_addr >= NVM::BASE45_USERROW)      // 0x1200
    return UPDI::write_userrow(start_addr, data, byte_count);

  /* From this point on, only program mode is allowed. */
  if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_PROG_bp)) return false;

  /* About flash regions */
  switch (mem_type) {
    case JTAG2::MTYPE_FLASH_PAGE :            // 0xB0
    case JTAG2::MTYPE_XMEGA_APP_FLASH :       // 0xC0
    case JTAG2::MTYPE_XMEGA_BOOT_FLASH :      // 0xC1
    case JTAG2::MTYPE_XMEGA_USERSIG : {       // 0xC5

      /* Instructions with mismatched page sizes are rejected */
      if (!check_pagesize(JTAG2::updi_desc.flash_page_size, byte_count)) {
        /* Kill the process with a strong error */
        set_response(JTAG2::RSP_FAILED);
        return true;
      }

      /* A page block must be erased before writing to a new page block.
         The new AVRDUDE splits large page blocks into multiple queries to read-modify-write.
         This prevents atomic operations and requires special handling. */
      bool is_bound = bit_is_clear(UPDI_CONTROL, UPDI::UPDI_ERFM_bp);
      if (is_bound) {
        uint16_t block_addr = (start_addr >> 1) & ~((JTAG2::updi_desc.flash_page_size - 1) >> 1);
        is_bound = before_addr != block_addr;
        before_addr = block_addr;
      }

      /* NVMCTRL processing steps vary depending on the version. */
      if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN4_bp))
        return write_flash_v4(start_addr, data, byte_count, is_bound);
      else if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN3_bp))
        return write_flash_v3(start_addr, data, byte_count, is_bound);
      else if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp))
        return write_flash_v2(start_addr, data, byte_count, is_bound);
      else
        return write_flash_v0(start_addr, data, byte_count, is_bound);
    }
  }

  /* Other writes are allowed from 1 to 256 bytes */
  if (byte_count == 0 || byte_count > 256) {
    set_response(JTAG2::RSP_ILLEGAL_MEMORY_RANGE);
    return true;
  }

  switch (mem_type) {
    /* Can write to the IO region as is */
    case JTAG2::MTYPE_SRAM : {                // 0x20
      return UPDI::sts8(start_addr, data, byte_count);
    }
    /* EEPROM region */
    case JTAG2::MTYPE_LOCK_BITS :             // 0xB3
    case JTAG2::MTYPE_FUSE_BITS : {           // 0xB2
      /* The NVMCTRL version 0 implementation is special. */
      if (bit_is_clear(UPDI_NVMCTRL, UPDI::UPDI_GEN3_bp)
      && bit_is_clear(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp)) {
        do {
          if (!write_fuse(start_addr++, *data++)) return false;
        } while (--byte_count);
        break;
      }
      /* FUSES in other implementations is equivalent to EEPROM */
    }
    case JTAG2::MTYPE_XMEGA_EEPROM :          // 0xC4
    case JTAG2::MTYPE_EEPROM_PAGE :           // 0xB1
    case JTAG2::MTYPE_EEPROM : {              // 0x22
      /* NVMCTRL processing steps vary depending on the version. */
      if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN4_bp))
        return write_eeprom_v4(start_addr, data, byte_count);
      else if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN3_bp))
        return write_eeprom_v3(start_addr, data, byte_count);
      else if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp))
        return write_eeprom_v2(start_addr, data, byte_count);
      else
        return write_eeprom_v0(start_addr, data, byte_count);
    }
    default :
      /* Other memory types are rejected */
      set_response(JTAG2::RSP_ILLEGAL_MEMORY_TYPE);
  }
  return true;
}

// end of code

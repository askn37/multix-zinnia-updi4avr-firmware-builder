/**
 * @file UPDI.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "Prototypes.h"
#include <avr/pgmspace.h>
#include <api/capsule.h>

namespace UPDI {
  const static uint8_t nvmprog_key[10] =
    {UPDI::UPDI_SYNCH, UPDI::UPDI_KEY_64, 0x20, 0x67, 0x6F, 0x72, 0x50, 0x4D, 0x56, 0x4E};
  const static uint8_t erase_key[10] =
    {UPDI::UPDI_SYNCH, UPDI::UPDI_KEY_64, 0x65, 0x73, 0x61, 0x72, 0x45, 0x4D, 0x56, 0x4E};
  const static uint8_t urowwrite_key[10] =
    {UPDI::UPDI_SYNCH, UPDI::UPDI_KEY_64, 0x65, 0x74, 0x26, 0x73, 0x55, 0x4D, 0x56, 0x4E};
}

void UPDI::guardtime (void) {
  delay_micros(UPDI_GUARDTIME);
}

void UPDI::setup (void) {
  UPDI_USART.BAUD  = UPDI_BAUD_CALC;
  UPDI_USART.CTRLA = UPDI_USART_CTRLA;
  UPDI_USART.CTRLC = UPDI_USART_CTRLC;
  UPDI_USART.CTRLB = UPDI_USART_ON;
  UPDI_CONTROL &= ~_BV(UPDI::UPDI_CLKU_bp);

  /* ソフトリセット後は ターゲットのリセットを解除 */
  if ( bit_is_set(RSTCTRL_RSTFR, RSTCTRL_SWRF_bp) ) UPDI::Target_Reset(false);
}

/*
 * UPDI受信
 */

uint8_t UPDI::RECV (void) {
  loop_until_bit_is_set(UPDI_USART.STATUS, USART_RXCIF_bp);
  UPDI_LASTH = UPDI_USART.RXDATAH ^ 0x80;
  return UPDI_LASTL = UPDI_USART.RXDATAL;
}

/*
 * UPDI送信
 *
 * ループバック受信で送信値を照合
 */

bool UPDI::SEND (uint8_t _data) {
  bool _r;
  loop_until_bit_is_set(UPDI_USART.STATUS, USART_DREIF_bp);
  UPDI_USART.STATUS = USART_TXCIF_bm;
  UPDI_USART.TXDATAL = _data;
  loop_until_bit_is_set(UPDI_USART.STATUS, USART_TXCIF_bp);
  _r = _data == UPDI::RECV();
  if (!_r) UPDI_LASTH |= 0x20;
  return _r;
}

/* 通常のBREAK長（96キャラクタ分のLOW） */
#if ((UPDI_BAUD_CALC * 100) > 65535)
#define UPDI_BAUD_BREAK 65535
#else
#define UPDI_BAUD_BREAK (UPDI_BAUD_CALC * 100)
#endif

/* HV制御時のBREAK長（10MHz=712?） */
#define UPDI_BAUD_SHORT_BREAK (F_CPU / 10000)

/*
 * BREAKキャラクタ
 *
 * 送信速度を遅くすることで生成する
 */

void UPDI::BREAK (void) {
  UPDI_USART.BAUD = UPDI_BAUD_BREAK;
  UPDI::SEND(UPDI::UPDI_NOP);
  UPDI_USART.BAUD = UPDI_BAUD_CALC;
}

/*
 * 対象リセット
 *
 * BREAKを前置するタイプ
 */

bool UPDI::Target_Reset (bool _enable) {
  static uint8_t set_ptr[] = {
      UPDI::UPDI_SYNCH
    , UPDI::UPDI_STCS | UPDI::UPDI_CS_ASI_RESET_REQ
    , 0
    , UPDI::UPDI_STCS | UPDI::UPDI_CS_CTRLB
    , UPDI::UPDI_SET_UPDIDIS
  };
  UPDI::BREAK();
  set_ptr[2] = _enable ? UPDI::UPDI_RSTREQ : UPDI::UPDI_NOP;
  return UPDI::send_bytes(set_ptr, sizeof(set_ptr));
}

/*
 * バイト塊送信
 */

bool UPDI::send_bytes (const uint8_t *data, size_t len) {
  uint8_t *p = (uint8_t*)(void*)data;
  do {
    if (!UPDI::SEND(*p++)) break;
  } while (--len);
  return len == 0;
}

/*
 * リピートヘッダ送信
 */

bool UPDI::send_repeat_header (uint8_t cmd, uint32_t addr, size_t len) {
  static uint8_t set_ptr[] = {
      UPDI::UPDI_SYNCH
    , UPDI::UPDI_ST | UPDI::UPDI_PTR_REG | UPDI::UPDI_DATA3
    , 0, 0, 0, 0  // qword address
  };
  static uint8_t set_repeat[] = {
      UPDI::UPDI_SYNCH
    , UPDI::UPDI_REPEAT | UPDI::UPDI_DATA1
    , 0                   // repeat count
    , UPDI::UPDI_SYNCH
    , UPDI::UPDI_PTR_INC  // +cmd
  };
  _CAPS32(set_ptr[2])->dword = addr;
  set_repeat[2] = len - 1;
  set_repeat[4] = UPDI::UPDI_PTR_INC | cmd;
  if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) return false;
  if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  return UPDI::send_bytes(set_repeat, sizeof(set_repeat));
}

/*
 * 単バイト送信
 */

bool UPDI::st8 (uint32_t addr, uint8_t data) {
  static uint8_t set_ptr[] = {
      UPDI::UPDI_SYNCH
    , UPDI::UPDI_STS | UPDI::UPDI_ADDR3 | UPDI::UPDI_DATA1
    , 0, 0, 0, 0  // qword address
  };
  _CAPS32(set_ptr[2])->dword = addr;
  if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) return false;
  if (UPDI::UPDI_ACK != UPDI::RECV()) return false;
  if (!UPDI::SEND(data)) return false;
  return UPDI::UPDI_ACK == UPDI::RECV();
}

/*
 * 複バイト送信
 */

bool UPDI::sts8 (uint32_t addr, uint8_t *data, size_t len) {
  if (UPDI::send_repeat_header((UPDI::UPDI_ST | UPDI::UPDI_DATA1), addr, len)) {
    do {
      if (!UPDI::SEND(*data++)) break;
      if (UPDI::UPDI_ACK != RECV()) break;
    } while (--len);
  }
  return len == 0;
}

/*
 * 単バイト受信
 */

uint8_t UPDI::ld8 (uint32_t addr) {
  static uint8_t set_ptr[] = {
      UPDI::UPDI_SYNCH
    , UPDI::UPDI_LDS | UPDI::UPDI_ADDR3 | UPDI::UPDI_DATA1
    , 0, 0, 0, 0  // qword address
  };
  _CAPS16(set_ptr[2])->word = (uint16_t)addr;
  while (!UPDI::send_bytes(set_ptr, sizeof(set_ptr) - 1)) UPDI::BREAK();
  return UPDI::RECV();
}

/*
 * コントロールステータス受信
 */

uint8_t UPDI::get_cs_stat (const uint8_t code) {
  static uint8_t set_ptr[] = { UPDI::UPDI_SYNCH, 0 };
  set_ptr[1] = UPDI::UPDI_LDCS | code;
  while (!UPDI::send_bytes(set_ptr, sizeof(set_ptr))) UPDI::BREAK();
  return UPDI::RECV();
}
inline bool UPDI::is_cs_stat (const uint8_t code, uint8_t check) {
  return check == (UPDI::get_cs_stat(code) & check);
}
inline bool UPDI::is_sys_stat (const uint8_t check) {
  return UPDI::is_cs_stat(UPDI::UPDI_CS_ASI_SYS_STATUS, check);
}
inline bool UPDI::is_key_stat (const uint8_t check) {
  return UPDI::is_cs_stat(UPDI::UPDI_CS_ASI_KEY_STATUS, check);
}
inline bool UPDI::is_rst_stat (void) {
  return UPDI::is_cs_stat(UPDI::UPDI_CS_ASI_RESET_REQ, 1);
}

/*
 * コントロールステータス送信
 */

bool UPDI::set_cs_stat (const uint8_t code, uint8_t data) {
  static uint8_t set_ptr[] = { UPDI::UPDI_SYNCH, 0, 0 };
  set_ptr[1] = UPDI::UPDI_STCS | code;
  set_ptr[2] = data;
  return UPDI::send_bytes(set_ptr, sizeof(set_ptr));
}
inline bool UPDI::set_cs_ctra (const uint8_t data) {
  return UPDI::set_cs_stat(UPDI::UPDI_CS_CTRLA, data);
}
inline bool UPDI::set_cs_asi_ctra (const uint8_t data) {
  return UPDI::set_cs_stat(UPDI::UPDI_CS_ASI_CTRLA, data);
}
inline uint8_t UPDI::get_cs_asi_ctra (void) {
  return UPDI::get_cs_stat(UPDI::UPDI_CS_ASI_CTRLA);
}
inline bool UPDI::updi_reset (bool logic) {
  return UPDI::set_cs_stat(
    UPDI::UPDI_CS_ASI_RESET_REQ,
    (logic ? UPDI::UPDI_RSTREQ : UPDI::UPDI_NOP));
}

/*
 * Dummy SIGNATURE
 */

bool UPDI::check_sig (void) {
  uint8_t mem_type  = JTAG2::packet.body[1];
  uint8_t s = _CAPS32(JTAG2::packet.body[2])->bytes[0];
  uint8_t a = _CAPS32(JTAG2::packet.body[6])->bytes[0]
    - (bit_is_set(UPDI_NVMCTRL, UPDI_BROW_bp) ? (uint8_t)NVM::EB_SIGROW
                                              : (uint8_t)NVM::BASE_SIGROW);
  uint8_t c;
  if (bit_is_clear(UPDI_CONTROL, UPDI_PROG_bp) && mem_type == JTAG2::MTYPE_SIGN_JTAG && s == 1) {
    JTAG2::packet.body[0] = JTAG2::RSP_MEMORY;
    if (bit_is_clear(UPDI_CONTROL, UPDI_INFO_bp)) c = UPDI_LASTH ? 0x00 : 0xFF;
    else if (a == 0) c = 0x1E;
    else if (a == 1) c = bit_is_set(UPDI_NVMCTRL, UPDI_GEN2_bp) ? 'A'
                       : bit_is_set(UPDI_NVMCTRL, UPDI_LOWF_bp) ? 'm' : 't';
    else if (a == 2) c = bit_is_set(UPDI_NVMCTRL, UPDI_GEN3_bp) ? '3'
                       : bit_is_set(UPDI_NVMCTRL, UPDI_GEN2_bp) ? '2' : '0';
    else return false;
    JTAG2::packet.size = 2;
    JTAG2::packet.body[1] = c;
    return true;
  }
  return false;
}

/*
 * USERROW/USERSIG 特殊書込準備
 */
bool UPDI::enter_userrow (void) {
  /* send urowwrite_key */
  if (!UPDI::send_bytes(UPDI::urowwrite_key, sizeof(UPDI::urowwrite_key))) return false;
  /* restart target : change mode */
  if (!UPDI::updi_reset(true) || !UPDI::updi_reset(false)) return false;
  do{ delay_micros(100); } while (!UPDI::is_sys_stat(UPDI::UPDI_SYS_UROWPROG));
  return true;
}

/*
 * HVパルス印加
 */
void HV_Pulse (void) {
  TIM::HV_Pulse_ON();
  openDrainWrite(TRST_PIN, LOW);
  delay_micros(800);
  openDrainWrite(TRST_PIN, HIGH);
  delay_micros(800);
  if (bit_is_set(UPDI_NVMCTRL, UPDI::UPDI_GEN2_bp)) {
    digitalWrite(HV8_PIN, HIGH);
    delay_micros(800);
    digitalWrite(HV8_PIN, LOW);
  }
  else {
    digitalWrite(HV12_PIN, HIGH);
    delay_micros(800);
    digitalWrite(HV12_PIN, LOW);
  }
  TIM::HV_Pulse_OFF();
  UPDI_USART.BAUD = UPDI_BAUD_SHORT_BREAK;
  UPDI::SEND(UPDI::UPDI_NOP);
  UPDI_USART.BAUD = UPDI_BAUD_CALC;
  UPDI_CONTROL |= _BV(UPDI::UPDI_ERHV_bp);
  UPDI_CONTROL &= ~_BV(UPDI::UPDI_CLKU_bp);
}


/*
 * チップ一括消去
 *
 * HV制御はここを通る時のみ行われる
 */

bool UPDI::chip_erase (void) {
  /* Send HV Pulse */
  if (bit_is_clear(UPDI_CONTROL, UPDI_INFO_bp)) HV_Pulse();

  /* send nvmprog_key */
  if (!UPDI::send_bytes(UPDI::nvmprog_key, sizeof(UPDI::nvmprog_key))) return false;

  /* send erase_key */
  if (!UPDI::send_bytes(UPDI::erase_key, sizeof(UPDI::erase_key))) return false;

  /* restart target : change mode */
  if (!UPDI::updi_reset(true) || !UPDI::updi_reset(false)) return false;

  /* wait enable : chip erase mode success */
  delay_millis(10);

  do{ delay_micros(100); } while (UPDI::is_sys_stat(UPDI::UPDI_SYS_LOCKSTATUS));
  UPDI_CONTROL |= _BV(UPDI::UPDI_ERFM_bp);

  if (bit_is_set(UPDI_CONTROL, UPDI::UPDI_INFO_bp)) {
    UPDI_CONTROL &= ~_BV(UPDI::UPDI_PROG_bp);
    return UPDI::enter_prog();
  }
  return UPDI::enter_updi(true) && UPDI::enter_prog();
}

/*
 * UPDI制御開始
 */

bool UPDI::enter_updi (bool skip) {
  static uint8_t set_ptr[] = { UPDI::UPDI_SYNCH, UPDI::UPDI_SIB_128 };
  uint8_t* _p = &JTAG2::packet.body[4];
  size_t _len = 16;
  if (!skip) {
    /* HV制御強制許可 */
    if (bit_is_set(UPDI_CONTROL, UPDI_FCHV_bp)) {
      HV_Pulse();

      /* send nvmprog_key */
      if (!UPDI::send_bytes(UPDI::nvmprog_key, sizeof(UPDI::nvmprog_key))) return false;

      /* restart target : change mode */
      if (!UPDI::updi_reset(true) || !UPDI::updi_reset(false)) return false;

      /* wait enable : chip erase mode success */
      delay_millis(50);
    }
    else
      UPDI::BREAK();
  }
  if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_INFO_bp)) {
    if (!UPDI::set_cs_asi_ctra(UPDI::UPDI_SET_UPDICLKSEL_8M)) return false;
    if (!UPDI::set_cs_ctra(UPDI::UPDI_SET_GTVAL_2)) return false;
    if (!UPDI::send_bytes(set_ptr, sizeof(set_ptr))) return false;
    while (_len--) *_p++ = UPDI::RECV();
    switch (JTAG2::packet.body[4]) {
      case 'm' : {              // 'megaAVR' series
        /* megaAVR SIB = 'megaAVR P:0D:1-3' */
        UPDI_NVMCTRL |= _BV(UPDI::UPDI_LOWF_bp);
        NVM::nvm_flash_offset = 0x4000;
        break;
      }
      case 't' : {              // 'tinyAVR' series
        /* tinyAVR SIB = 'tinyAVR P:0D:1-3' */
        NVM::nvm_flash_offset = 0x8000;
        break;
      }
      case ' ' :
      case 'A' : {              // 'AVR_Dx' series
        /* AVR Dx SIB = 'AVR     P:2D:1-3' */
        /* AVR Ex SIB = 'AVR     P:3D:1-3' */
        /* AVR DA SIB = '    AVR P:2D:1-3' (最初期ロット) */
        if (JTAG2::packet.body[14] == '3') {
          // 'AVR_Ex' series
          UPDI_NVMCTRL |= _BV(UPDI::UPDI_GEN3_bp);
        }
        UPDI_NVMCTRL |= _BV(UPDI::UPDI_GEN2_bp);
        NVM::nvm_flash_offset = 0x800000;
        break;
      }
      default : {
        return false;
      }
    }
    UPDI_CONTROL |= _BV(UPDI::UPDI_INFO_bp);
  }
  if ((UPDI::get_cs_asi_ctra() & UPDI::UPDI_SET_UPDICLKSEL_bm) 
                              == UPDI::UPDI_SET_UPDICLKSEL_8M) {
    UPDI_CONTROL |= _BV(UPDI::UPDI_CLKU_bp);
    UPDI_USART.BAUD = UPDI_BAUD_CALC >> 1;
  }
  return true;
}

/*
 * プログラミング制御開始
 *
 */

bool UPDI::enter_prog (void) {
  if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_PROG_bp)) {
    while (UPDI::is_sys_stat(UPDI::UPDI_SYS_RSTSYS));
    if (!(UPDI_LASTL & UPDI::UPDI_SYS_NVMPROG)) {
      if (UPDI_LASTL & UPDI::UPDI_SYS_LOCKSTATUS) return false;
      if (!UPDI::is_key_stat(UPDI::UPDI_KEY_NVMPROG)) {
        if (!UPDI::send_bytes(UPDI::nvmprog_key, sizeof(UPDI::nvmprog_key))) return false;
        do{ delay_micros(100); } while (!UPDI::is_key_stat(UPDI::UPDI_KEY_NVMPROG));
      }
      if (!UPDI::updi_reset(true) || !UPDI::updi_reset(false)) return false;
      do{ delay_micros(100); } while (!UPDI::is_sys_stat(UPDI::UPDI_SYS_NVMPROG));
    }
    UPDI_CONTROL |= _BV(UPDI::UPDI_INFO_bp);
    UPDI_CONTROL |= _BV(UPDI::UPDI_PROG_bp);
  }
  return true;
}

/*
 * UPDI許認可
 */

bool UPDI::updi_activate (void) {
  volatile uint8_t count = 3;
  while (--count && bit_is_clear(UPDI_CONTROL, UPDI_PROG_bp)) {
    delay_millis(100);
    if (setjmp(TIM::CONTEXT) == 0) {
      TIM::Timeout_Start(150);
      UPDI::enter_updi(false) && UPDI::enter_prog();
    }
    TIM::Timeout_Stop();

    /* 2周目以降は JPショートならHV制御強制を許可 */
    if (!digitalRead(JP_SENSE_PIN)) UPDI_CONTROL |= _BV(UPDI::UPDI_FCHV_bp);
  }
  return bit_is_set(UPDI_CONTROL, UPDI_PROG_bp);
}

/*
 * UPDI制御プロセス
 */

bool UPDI::runtime (uint8_t updi_cmd) {
  volatile bool _result = false;
  uint16_t limit = 4000;
  if (setjmp(TIM::CONTEXT) == 0) {
    TIM::Timeout_Start(limit);
    switch (updi_cmd) {
      case UPDI::UPDI_CMD_READ_MEMORY : {
        _result = NVM::read_memory();
        break;
      }
      case UPDI::UPDI_CMD_WRITE_MEMORY : {
        if (bit_is_clear(UPDI_CONTROL, UPDI_PROG_bp)) {
          /* USERROW だけは非PROG状態でも処理を通す */
          if (JTAG2::packet.body[1] != JTAG2::MTYPE_USERSIG) break;
        }
        else {
          if (UPDI::is_sys_stat(UPDI::UPDI_SYS_LOCKSTATUS)) break;
          if (!(UPDI_LASTL & UPDI::UPDI_SYS_NVMPROG)) break;
        }
        _result = NVM::write_memory();
        break;
      }
      case UPDI::UPDI_CMD_ERASE : {
        if (JTAG2::packet.body[1] == JTAG2::XMEGA_ERASE_CHIP
          && _CAPS32(JTAG2::packet.body[2])->dword == 0) {
          _result = UPDI::chip_erase();
        }
        break;
      }
      case UPDI::UPDI_CMD_GO : {
        _result = UPDI::Target_Reset(true) && UPDI::Target_Reset(false);
        break;
      }
    }
  }
  TIM::Timeout_Stop();
  UPDI_USART.CTRLB = UPDI_USART_ON;
  wdt_reset();
  return _result;
}

// end of code

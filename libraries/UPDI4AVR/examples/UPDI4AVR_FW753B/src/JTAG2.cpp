/**
 * @file JTAG2.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "Prototypes.h"
#include <avr/io.h>
#include <util/crc16.h>
#include <api/capsule.h>

#define BAUD_REG_VAL(baud) (((F_CPU / (baud >> 3)) + 1) / 2)
#define PARAM_VTARGET_VAL 5000

namespace JTAG2 {
  /* Global valiables */
  jtag_packet_t packet;
  uint8_t PARAM_EMU_MODE_VAL;
  jtag_baud_rate_e PARAM_BAUD_RATE_VAL = BAUD_19200;

  const uint16_t BAUD_TABLE[] PROGMEM = {
      BAUD_NOTUSED          // 0: not used dummy
    , BAUD_REG_VAL(2400)    // 1: F_CPU 9.83Mhz lower limit
    , BAUD_REG_VAL(4800)    // 2: F_CPU 19.67MHz lower limit
    , BAUD_REG_VAL(9600)
    , BAUD_REG_VAL(19200)   // 4: JTAG2 startup default speed
    , BAUD_REG_VAL(38400)   // 5: terminal mode active
    , BAUD_REG_VAL(57600)
    , BAUD_REG_VAL(115200)  // 7: avrdude 6.x max speed
    , BAUD_REG_VAL(14400)   // 8: using avrdude 6.x lower
    , BAUD_REG_VAL(153600)  // 9: using avrdude 7.x upper
    , BAUD_REG_VAL(230400)
    , BAUD_REG_VAL(460800)
    , BAUD_REG_VAL(921600)  // 13: UPDI4AVR standard speed
    , BAUD_REG_VAL(128000)
    , BAUD_REG_VAL(256000)
    , BAUD_NOTUSED          // 16: CH340E not supported
    , BAUD_NOTUSED          // 17: CH340E not supported
    , BAUD_REG_VAL(150000)
    , BAUD_REG_VAL(200000)
    , BAUD_REG_VAL(250000)
    , BAUD_REG_VAL(300000)
    , BAUD_REG_VAL(400000)
    , BAUD_REG_VAL(500000)
    , BAUD_REG_VAL(600000)
    , BAUD_REG_VAL(666666)  // 24: terminal mode active
    , BAUD_REG_VAL(1000000) // 25: F_CPU 10MHz or more
    , BAUD_REG_VAL(1500000) // 26: F_CPU 12MHz or more
    , BAUD_REG_VAL(2000000) // 27: F_CPU 16MHz or more
    , BAUD_REG_VAL(3000000) // 28: F_CPU 24MHz or more
  };

  const uint8_t sign_on_resp[] PROGMEM = {
      JTAG2::RSP_SIGN_ON    // $00: MESSAGE_ID   : $86
    , 0x01                  // $01: COMM_ID      : Communications protocol version
    , 0x01                  // $02: M_MCU_BLDR   : boot-loader FW version
    , VER_M_MIN             // $03: M_MCU_FW_MIN : firmware version (minor) // Custom MIN
    , VER_M_MAJ             // $04: M_MCU_FW_MAJ : firmware version (major) // Custom MAJ
    , 0x01                  // $05: M_MCU_HW     : hardware version
    , 0x01                  // $06: S_MCU_BLDR   : boot-loader FW version
    , VER_S_MIN             // $07: S_MCU_FW_MIN : firmware version (minor)
    , VER_S_MAJ             // $08: S_MCU_FW_MAJ : firmware version (major) using version 7
    , 0x01                  // $09: S_MCU_HW     : hardware version
                            // $0A-$0F: SERIAL_NUMBER[0-5]
    , 0x00                  // $0A: SERIAL_NUMBER0
    , 0x00                  // $0B: SERIAL_NUMBER1
    , 0x00                  // $0C: SERIAL_NUMBER2
    , 0x00                  // $0D: SERIAL_NUMBER3
    , 0x00                  // $0E: SERIAL_NUMBER4
    , 0x00                  // $0F: SERIAL_NUMBER5
                            // $10-$1C: DEVICE_ID_STR : terminate \0
    , 'J', 'T', 'A', 'G', 'I', 'C', 'E', ' ', 'm', 'k', 'I', 'I', 0
  };
}

/* 初期化 */
void JTAG2::setup (void) {
  JTAG_USART.BAUD = pgm_read_word( &BAUD_TABLE[JTAG2::BAUD_19200] );
  JTAG_USART.CTRLA = JTAG_USART_CTRLA;
  JTAG_USART.CTRLC = JTAG_USART_CTRLC;
  JTAG_USART.CTRLB = JTAG_USART_OFF;
}

void JTAG2::transfer_enable (void) {
  SYS::PG_Enable();
  pinMode(JTAG_TXD_PIN, OUTPUT);
  JTAG_USART.CTRLB = JTAG_USART_ON;
}

void JTAG2::transfer_disable (void) {
  JTAG_USART.CTRLB = JTAG_USART_OFF;
  pinMode(JTAG_TXD_PIN, INPUT_PULLUP);
  SYS::PG_Disable();
}

uint8_t JTAG2::get (void) {
  loop_until_bit_is_set(JTAG_USART.STATUS, USART_RXCIF_bp);
  return JTAG_USART.RXDATAL;
}

uint8_t JTAG2::put (uint8_t _data) {
  loop_until_bit_is_set(JTAG_USART.STATUS, USART_DREIF_bp);
  JTAG_USART.STATUS = USART_TXCIF_bm;
  return JTAG_USART.TXDATAL = _data;
}

void JTAG2::flush (void) {
  loop_until_bit_is_set(JTAG_USART.STATUS, USART_TXCIF_bp);
}

uint16_t JTAG2::crc16_update(uint16_t _crc, uint8_t _data) {
  return _crc_ccitt_update(_crc, _data);
}

/* JTAG Receive */
bool JTAG2::packet_receive (void) {
  uint16_t _crc = ~0;
  uint8_t *p = (uint8_t*) &packet.soh;
  uint8_t *q = (uint8_t*) &packet.soh;

  /* 受信待ち（無限ループ）*/
  while (get() != JTAG2::MESSAGE_START);
  (*p++) = JTAG2::MESSAGE_START;

  /* 最初の7byte */
  for (int8_t i = 0; i < 7; i++) *p++ = get();

  /* STX 確認 */
  if (packet.stx != JTAG2::TOKEN) return false;

  /* パケット長確認 */
  if (packet.size > sizeof(packet.body)) return false;

  /* 残りを受信 */
  for (int16_t j = -2; j < packet.size_word[0]; j++) *p++ = get();

  /* 受信バッファが満たされたら CRC 確認 */
  while (p != q) _crc = JTAG2::crc16_update(_crc, *q++);
  return _crc == 0;
}

/* JTAG Answer */
void JTAG2::answer_transfer (void) {
  uint16_t _crc = ~0;
  int16_t _len = packet.size_word[0] + 8;
  uint8_t *_p = (uint8_t*) &packet.soh;
  uint8_t *_q = (uint8_t*) &packet.soh;
  while (_len--) _crc = JTAG2::crc16_update(_crc, *_q++);
  (*_q++) = _CAPS16(_crc)->bytes[0];
  (*_q++) = _CAPS16(_crc)->bytes[1];
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    while (_p != _q) put(*_p++);
  }
}

/* JTAG Response */
void JTAG2::set_response (jtag_response_e response_code) {
  packet.size = 4;
  /* 応答番号 */
  packet.body[0] = response_code;
  /* 動作状態 0:無効 1:認識 2:書込可 */
  packet.body[1] = bit_is_set(UPDI_CONTROL, UPDI::UPDI_PROG_bp) ? 2
                 : bit_is_set(UPDI_CONTROL, UPDI::UPDI_INFO_bp) ? 1 : 0;
  /* 内部状態フラグ */
  packet.body[2] = UPDI_CONTROL;
  packet.body[3] = UPDI_NVMCTRL;
}

/* SIGN_ON Response */
void JTAG2::sign_on_response (void) {
  packet.size = sizeof(sign_on_resp);
  for (uint8_t i = 0; i < sizeof(sign_on_resp); i++)
    packet.body[i] = pgm_read_byte( &sign_on_resp[i] );
  /* SERNUMから Version情報を生成する */
  uint8_t* p = (uint8_t*) &SIGROW_SERNUM0;
  uint8_t* q = (uint8_t*) &SIGROW_SERNUM4;
  uint8_t* r = (uint8_t*) &packet.body[10];
    *r++ = *q++ ^ *p++ ^ *p++;
    *r++ = *q++ ^ *p++ ^ *p++;
    *r++ = *q++;
    *r++ = *q++;
    *r++ = *q++;
    *r++ = *q++;
  JTAG2::answer_transfer();
}

/* CMND_SET_PARAMETER */
bool JTAG2::set_parameter (void) {
  uint8_t param_val = packet.body[2];
  switch (packet.body[1]) {
    /* エミュレーションモード */
    case JTAG2::PARAM_EMU_MODE : {
      PARAM_EMU_MODE_VAL = param_val;
      break;
    }
    /* JTAGインタフェース速度 */
    case JTAG2::PARAM_BAUD_RATE : {
      /* 対応範囲確認 */
      if ((param_val >= JTAG2::BAUD_LOWER) && (param_val <= JTAG2::BAUD_UPPER)) {
        uint16_t baud = pgm_read_word( &BAUD_TABLE[param_val] );
        if (baud) {
          /* 正常なら応答したのち速度変更 */
          PARAM_BAUD_RATE_VAL = (jtag_baud_rate_e) param_val;
          JTAG2::set_response(JTAG2::RSP_OK);
          JTAG2::answer_transfer();
          JTAG2::flush();
          if (baud < 64) {
            JTAG_USART.CTRLB = JTAG_USART_DBLON;
            baud <<= 1;
          }
          JTAG_USART.BAUD = baud;
          /* terminal mode を許可する速度 */
          if (param_val == JTAG2::BAUD_38400 || param_val == JTAG2::BAUD_666666)
            UPDI_CONTROL |= _BV(UPDI::UPDI_TERM_bp);
          return false;
        }
      }
      /* 範囲違反は失敗応答 */
      JTAG2::set_response(JTAG2::RSP_ILLEGAL_VALUE);
      return true;
    }
  }
  JTAG2::set_response(JTAG2::RSP_OK);
  return true;
}

/* CMND_GET_PARAMETER */
void JTAG2::get_parameter (void) {
  volatile uint8_t &param_type = packet.body[1];
  switch (param_type) {
    case JTAG2::PARAM_HW_VER : {
      packet.body[1] = sign_on_resp[5];
      packet.body[2] = sign_on_resp[9];
      packet.size_word[0] = 3;
      break;
    }
    case JTAG2::PARAM_FW_VER : {
      packet.body[1] = sign_on_resp[3];
      packet.body[2] = sign_on_resp[4];
      packet.body[3] = sign_on_resp[7];
      packet.body[4] = sign_on_resp[8];
      packet.size_word[0] = 5;
      break;
    }
    case JTAG2::PARAM_EMU_MODE : {
      packet.body[1] = PARAM_EMU_MODE_VAL;
      packet.size_word[0] = 2;
      break;
    }
    case JTAG2::PARAM_BAUD_RATE : {
      packet.body[1] = PARAM_BAUD_RATE_VAL;
      packet.size_word[0] = 2;
      break;
    }
    case JTAG2::PARAM_VTARGET : {
      _CAPS16(packet.body[1])->word = SYS::get_vcc();
      packet.size_word[0] = 3;
      break;
    }
    default : {
      JTAG2::set_response(JTAG2::RSP_ILLEGAL_PARAMETER);
      return;
    }
  }
  packet.body[0] = JTAG2::RSP_PARAMETER;
}

/* JTAG Process */
inline void JTAG2::process_command (void) {
  wdt_reset();
  switch ( packet.body[0] ) {
    case JTAG2::CMND_GET_SIGN_ON : {
      SYS::RTS_Disable();
      SYS::WDT_ON();
      TIM::LED_Stop();
      JTAG2::transfer_enable();
      JTAG2::sign_on_response();
      return;
    }
    case JTAG2::CMND_SET_PARAMETER : {
      if (!JTAG2::set_parameter()) return;
      break;
    }
    case JTAG2::CMND_GET_PARAMETER : {
      JTAG2::get_parameter();
      break;
    }
    case JTAG2::CMND_ENTER_PROGMODE : {
      uint8_t c = UPDI_CONTROL;
      UPDI::updi_activate();
      JTAG2::set_response(JTAG2::RSP_OK);
      if (!(c & _BV(UPDI::UPDI_INFO_bp)) && bit_is_set(UPDI_CONTROL, UPDI::UPDI_INFO_bp))
        packet.size = 19;
      TIM::LED_Blink();
      /* terminal mode 許可時は WDTを無効にする */
      if (bit_is_set(UPDI_CONTROL, UPDI::UPDI_TERM_bp)) SYS::WDT_OFF();
      break;
    }
    case JTAG2::CMND_READ_MEMORY : {
      /* 非PROG でも SIG要求にはダミー応答する */
      if (UPDI::check_sig()) break;
      /* terminal mode でなければLED高速点滅 */
      if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_TERM_bp)) TIM::LED_Fast();
      if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_PROG_bp)
      || !UPDI::runtime(UPDI::UPDI_CMD_READ_MEMORY)) {
        JTAG2::set_response(JTAG2::RSP_ILLEGAL_MCU_STATE);
      }
      break;
    }
    case JTAG2::CMND_WRITE_MEMORY : {
      JTAG2::set_response(
        UPDI::runtime(UPDI::UPDI_CMD_WRITE_MEMORY)
        ? JTAG2::RSP_OK
        : JTAG2::RSP_ILLEGAL_MEMORY_TYPE
      );
      break;
    }
    case JTAG2::CMND_XMEGA_ERASE : {
      uint8_t c = UPDI_CONTROL;
      JTAG2::set_response(
        UPDI::runtime(UPDI::UPDI_CMD_ERASE)
        ? JTAG2::RSP_OK
        : JTAG2::RSP_ILLEGAL_MCU_STATE
      );
      if (!(c & _BV(UPDI::UPDI_INFO_bp)) && bit_is_set(UPDI_CONTROL, UPDI::UPDI_INFO_bp))
        packet.size = 19;
      break;
    }
    case JTAG2::CMND_GO : {
      if (bit_is_set(UPDI_CONTROL, UPDI::UPDI_PROG_bp)) UPDI::runtime(UPDI::UPDI_CMD_GO);
      JTAG2::set_response(JTAG2::RSP_OK);
      break;
    }
    case JTAG2::CMND_SIGN_OFF : {
      JTAG2::set_response(JTAG2::RSP_OK);
      JTAG2::answer_transfer();
      JTAG2::flush();

      /* JTAG作業完了後は WDTリセット */
      SYS::WDT_REBOOT();
      /* Session Complete */
    }
    // Ex) struct xmega_device_desc
    case JTAG2::CMND_SET_XMEGA_PARAMS : {
      NVM::nvm_eeprom_offset   = _CAPS32(packet.body[12])->dword;
    //NVM::nvm_fuse_offset     = _CAPS32(packet.body[16])->dword;
      NVM::nvm_user_sig_offset = _CAPS32(packet.body[24])->dword;
    //NVM::nvm_data_offset     = _CAPS32(packet.body[32])->dword;
      NVM::flash_page_size     = _CAPS16(packet.body[42])->word;
    //NVM::eeprom_page_size    = packet.body[46];
      // AVR_DA/DB/DD/EA
      if (_CAPS32(NVM::nvm_user_sig_offset)->bytes[0] == 0x80) {
        NVM::nvm_flash_offset  = 0x800000;
        UPDI_NVMCTRL |= _BV(UPDI::UPDI_GEN2_bp);
      }
    }
    /* no support command, dummy response, all ok */
    case JTAG2::CMND_SET_DEVICE_DESCRIPTOR :
    case JTAG2::CMND_GET_SYNC :
    case JTAG2::CMND_RESET :
    case JTAG2::CMND_LEAVE_PROGMODE : {
      JTAG2::set_response(JTAG2::RSP_OK);
      break;
    }
    default : {
      JTAG2::set_response(JTAG2::RSP_FAILED);
    }
  }
  JTAG2::answer_transfer();
}

void JTAG2::wakeup_jtag (void) {
  for (;;) {
    if (JTAG2::packet_receive()) JTAG2::process_command();
  }
}

// end of code

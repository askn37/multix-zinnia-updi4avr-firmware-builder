/**
 * @file JTAG2.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.2
 * @date 2023-11-11
 *
 * @copyright Copyright (c) 2023 askn37 at github.com
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
  updi_device_descriptor updi_desc;
  jtag_packet_t packet;
  jtag_baud_rate_e param_baud_rate_val = BAUD_19200;
  uint16_t before_seqnum = -1;

  const uint16_t BAUD_TABLE[] PROGMEM = {
      BAUD_NOTUSED          // 0: not used dummy
    , BAUD_REG_VAL(2400)    // 1: F_CPU 9.83Mhz lower limit
    , BAUD_REG_VAL(4800)    // 2: F_CPU 19.67MHz lower limit
    , BAUD_REG_VAL(9600)    // 3: supported min
    , BAUD_REG_VAL(19200)   // 4: JTAG2 startup default speed
    , BAUD_REG_VAL(38400)   // 5: terminal mode active
    , BAUD_REG_VAL(57600)
    , BAUD_REG_VAL(115200)  // 7: normal : avrdude 6.x max speed
    , BAUD_REG_VAL(14400)   // 8: using avrdude 6.x lower
    , BAUD_REG_VAL(153600)  // 9: using avrdude 7.x upper
    , BAUD_REG_VAL(230400)  // 10: UPDI4AVR standard speed
    , BAUD_REG_VAL(460800)
    , BAUD_REG_VAL(921600)
    , BAUD_REG_VAL(128000)
    , BAUD_REG_VAL(256000)
    , BAUD_NOTUSED          // 15: CH340E not supported 512000
    , BAUD_NOTUSED          // 16: CH340E not supported 1024000
    , BAUD_REG_VAL(150000)
    , BAUD_REG_VAL(200000)
    , BAUD_REG_VAL(250000)
    , BAUD_REG_VAL(300000)
    , BAUD_REG_VAL(400000)  // 21: Best performing value (16MHz)
    , BAUD_REG_VAL(500000)  // 22: Best performing value (20MHz)
    , BAUD_REG_VAL(600000)  // 23: Best performing value (24MHz)
    , BAUD_REG_VAL(666666)  // 24: terminal mode active
    , BAUD_REG_VAL(1000000) // 25: F_CPU 10MHz or more
    , BAUD_REG_VAL(1500000) // 26: F_CPU 12MHz or more
    , BAUD_REG_VAL(2000000) // 27: F_CPU 16MHz or more
    , BAUD_REG_VAL(3000000) // 28: F_CPU 24MHz or more
  };

  const uint8_t sign_on_resp[] PROGMEM = {
      RSP_SIGN_ON     // $00: MESSAGE_ID   : $86
    , 0x01            // $01: COMM_ID      : Communications protocol version
    , 0x02            // $02: M_MCU_BLDR   : boot-loader FW version
    , VER_M_MIN       // $03: M_MCU_FW_MIN : firmware version (minor) // Custom MIN
    , VER_M_MAJ       // $04: M_MCU_FW_MAJ : firmware version (major) // Custom MAJ
    , 0x02            // $05: M_MCU_HW     : hardware version
    , 0x02            // $06: S_MCU_BLDR   : boot-loader FW version
    , VER_S_MIN       // $07: S_MCU_FW_MIN : firmware version (minor)
    , VER_S_MAJ       // $08: S_MCU_FW_MAJ : firmware version (major) using version 7
    , 0x02            // $09: S_MCU_HW     : hardware version
                      // $0A-$0F: SERIAL_NUMBER[0-5]
    , 0x00            // $0A: SERIAL_NUMBER0
    , 0x00            // $0B: SERIAL_NUMBER1
    , 0x00            // $0C: SERIAL_NUMBER2
    , 0x00            // $0D: SERIAL_NUMBER3
    , 0x00            // $0E: SERIAL_NUMBER4
    , 0x00            // $0F: SERIAL_NUMBER5
                      // $10-$1C: DEVICE_ID_STR : terminate \0
  //, 'J', 'T', 'A', 'G', 'I', 'C', 'E', ' ', 'm', 'k', 'I', 'I', 0
    , 'U', 'P', 'D', 'I', '4', 'A', 'V', 'R', 0, 0, 0, 0, 0
  };

  /*******************
   * Local functions *
   *******************/

  uint8_t get (void) {
    loop_until_bit_is_set(JTAG_USART.STATUS, USART_RXCIF_bp);
    return JTAG_USART.RXDATAL;
  }

  uint8_t put (uint8_t _data) {
    loop_until_bit_is_set(JTAG_USART.STATUS, USART_DREIF_bp);
    JTAG_USART.STATUS = USART_TXCIF_bm;
    return JTAG_USART.TXDATAL = _data;
  }

  void flush (void) {
    loop_until_bit_is_set(JTAG_USART.STATUS, USART_TXCIF_bp);
  }

  uint16_t crc16_update(uint16_t _crc, uint8_t _data) {
    return _crc_ccitt_update(_crc, _data);
  }

  void transfer_enable (void) {
    SYS::PG_Enable();
    pinMode(JTAG_TXD_PIN, OUTPUT);
    JTAG_USART.CTRLB = JTAG_USART_ON;
  }

  void transfer_disable (void) {
    JTAG_USART.CTRLB = JTAG_USART_OFF;
    pinMode(JTAG_TXD_PIN, INPUT_PULLUP);
    SYS::PG_Disable();
  }

  /****************
   * JTAG Receive *
   ****************/

  bool packet_receive (void) {
    uint16_t _crc = ~0;
    uint8_t *p = (uint8_t*) &packet.soh;
    uint8_t *q = (uint8_t*) &packet.soh;

    /* Waiting for reception (infinite loop) */
    while (get() != MESSAGE_START);
    (*p++) = MESSAGE_START;

    /* First 7bytes */
    for (int8_t i = 0; i < 7; i++) *p++ = get();

    /* STX confirmation */
    if (packet.stx != TOKEN) return false;

    /* Check packet length */
    if (packet.size > sizeof(packet.body)) return false;

    /* receive the rest */
    for (int16_t j = -2; j < packet.size_word[0]; j++) *p++ = get();

    /* CRC check when receive buffer is filled */
    while (p != q) _crc = crc16_update(_crc, *q++);
    return _crc == 0;
  }

  /***************
   * JTAG Answer *
   ***************/

  void answer_transfer (void) {
    uint16_t _crc = ~0;
    int16_t _len = packet.size_word[0] + 8;
    uint8_t *_p = (uint8_t*) &packet.soh;
    uint8_t *_q = (uint8_t*) &packet.soh;
    while (_len--) _crc = crc16_update(_crc, *_q++);
    (*_q++) = _CAPS16(_crc)->bytes[0];
    (*_q++) = _CAPS16(_crc)->bytes[1];
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      while (_p != _q) put(*_p++);
    }
  }

  /********************
   * SIGN_ON Response *
   ********************/

  void sign_on_response (void) {
    NVM::before_address = -1;
    packet.size_word[0] = sizeof(sign_on_resp);
    for (uint8_t i = 0; i < sizeof(sign_on_resp); i++)
      packet.body[i] = pgm_read_byte( &sign_on_resp[i] );
    /* Generate version information from SERNUM */
    uint8_t* p = (uint8_t*) &SIGROW_SERNUM0;
    uint8_t* q = (uint8_t*) &SIGROW_SERNUM4;
    uint8_t* r = (uint8_t*) &packet.body[10];
      *r++ = *q++ ^ *p++ ^ *p++;
      *r++ = *q++ ^ *p++ ^ *p++;
      *r++ = *q++;
      *r++ = *q++;
      *r++ = *q++;
      *r++ = *q++;
    answer_transfer();
  }

  /**********************
   * CMND_SET_PARAMETER *
   **********************/

  bool set_parameter (void) {
    uint8_t param_val = packet.body[RSP_DATA + 1];
    switch ( packet.body[RSP_DATA] ) {
      /* JTAG interface speed */
      case PAR_BAUD_RATE : {
        /* Compatible range confirmation */
        if ((param_val >= BAUD_LOWER) && (param_val <= BAUD_UPPER)) {
          uint16_t baud = pgm_read_word( &BAUD_TABLE[param_val] );
          if (baud) {
            /* If normal, respond and then change speed */
            param_baud_rate_val = (jtag_baud_rate_e) param_val;
            answer_transfer();
            flush();
            /* If the division ratio is too small, change to double speed mode */
            if (baud < 64) {
              JTAG_USART.CTRLB = JTAG_USART_DBLON;
              baud <<= 1;
            }
            JTAG_USART.BAUD = baud;
            /* Speed to allow terminal mode */
            if (param_val == BAUD_38400 || param_val == BAUD_666666)
              bit_set(UPDI_CONTROL, UPDI::UPDI_TERM_bp);
            return false;
          }
        }
        /* Range violation results in failure response */
        set_response(RSP_ILLEGAL_VALUE);
        return true;
      }

      /* The emulation mode number is always fixed, so it does nothing */
      case PAR_EMU_MODE :
      /* This has been enhanced since AVRDUDE 7.2 but does nothing */
      case PAR_PDI_OFFSET_START :
      case PAR_PDI_OFFSET_END : {
        break;
      }
    }
    return true;
  }

  /**********************
   * CMND_GET_PARAMETER *
   **********************/

  void get_parameter (void) {
    switch ( packet.body[RSP_DATA] ) {
      case PAR_HW_VER : {
        packet.body[1] = sign_on_resp[5];
        packet.body[2] = sign_on_resp[9];
        packet.size_word[0] = 3;
        break;
      }
      case PAR_FW_VER : {
        packet.body[1] = sign_on_resp[3];
        packet.body[2] = sign_on_resp[4];
        packet.body[3] = sign_on_resp[7];
        packet.body[4] = sign_on_resp[8];
        packet.size_word[0] = 5;
        break;
      }
      case PAR_EMU_MODE : {
        /* This response code indicates an UPDI-only implementation */
        /* In normal JTAGICEmkII this is never queried */
        packet.body[1] = 0x55;
        packet.size_word[0] = 2;
        break;
      }
      case PAR_BAUD_RATE : {
        packet.body[1] = param_baud_rate_val;
        packet.size_word[0] = 2;
        break;
      }
      case PAR_VTARGET : {
        _CAPS16(packet.body[1])->word = SYS::get_vcc();
        packet.size_word[0] = 3;
        break;
      }
      /* When extended parameters are enabled */
      case PAR_TARGET_SIGNATURE : {
        /* SIB information can be returned as an extended signature. */
        /* Responds correctly only during programming mode.          */
        /* otherwise an error will be returned.                      */
        /* Silicon revision (REVID) can be read as normal IO memory. */
        if (bit_is_set(UPDI_CONTROL, UPDI::UPDI_INFO_bp)) {
          uint8_t *q = &packet.body[RSP_DATA];
          uint8_t *p = &updi_desc.sib[0];
          for (uint8_t i = 0; i < sizeof(updi_desc.sib); i++) *q++ = *p++;
          packet.size_word[0] = 1 + sizeof(updi_desc.sib);
          break;
        }
      }
      default : {
        set_response(RSP_ILLEGAL_PARAMETER);
        return;
      }
    }
    packet.body[MESSAGE_ID] = RSP_PARAMETER;
  }

  /************************
   * CMND_SET_DEVICE_DESC *
   ************************/

  void set_descripter (uint8_t type) {
    if (type == CMND_SET_DEVICE_DESC) {
      const struct jtag_device_descriptor *desc =
           (struct jtag_device_descriptor*)&packet.body[RSP_DATA];
      updi_desc.flash_page_size = *(uint16_t*)(&desc->uiFlashPageSize[0]);
      updi_desc.eeprom_page_size = desc->ucEepromPageSize;
      if ((desc->ucIDRAddress & '0') == '0') {
        /* Accepts special extension settings. */
        /* 0x31 must be passed for automatic HV control to be inhibited. */
        /* This parameter can be specified in the `idr` descriptor in the `part` section. */
        updi_desc.hvupdi_variant = desc->ucIDRAddress;  // Valid value 0x30,0x31 or 0x32
      }
      else {
        /* If the EEPROM page size is 32 or more, it is considered a HV=12V system. */
        /* This setting will be referenced when automatic HV control is requested.  */
        updi_desc.hvupdi_variant = updi_desc.eeprom_page_size >= 32 ? '0' : '2';
      }
    }
    else if (type == CMND_SET_UPDI_PARAMS) {
      /* This structure is used instead of CMND_SET_DEVICE_DESC after */
      /* answering 0x55 ('U') to PAR_EMU_MODE of CMND_GET_PARAMETER.  */
      /* Once you have confirmed the magic number and data length,    */
      /* you can simply clone it into your internal structure.        */
      const struct updi_device_descriptor *desc =
           (struct updi_device_descriptor*)&packet.body[RSP_DATA];
      if (desc->magicnumber == 'U' && desc->length <= sizeof(updi_desc) - 2) {
        uint8_t *q = 2 + (uint8_t*)&updi_desc;
        uint8_t *p = 2 + (uint8_t*)&desc;
        for (int8_t i = 0; i < desc->length; i++) *q++ = *p++;
      }
    }
  }

  /****************
   * JTAG Process *
   ****************/

  inline void process_command (void) {
    wdt_reset();
    #ifdef ENABLE_DEBUG_UPDI_SENDER
    UPDI::_send_buf_clear();
    #endif
    uint8_t message_id = packet.body[MESSAGE_ID];
    packet.size_word[0] = 1;
    packet.body[MESSAGE_ID] = RSP_OK;
    switch (message_id) {
      case CMND_GET_SIGN_ON : {
        SYS::WDT_ON();
        SYS::RTS_Disable();
        TIM::LED_Stop();
        UPDI::Target_Reset(true);
        openDrainWrite(TRST_PIN, LOW);
        transfer_enable();
        sign_on_response();
        return;
      }
      case CMND_SET_PARAMETER : {
        /* Leave as soon as the speed changes */
        if (!set_parameter()) return;
        break;
      }
      case CMND_GET_PARAMETER : {
        get_parameter();
        break;
      }
      case CMND_RESET : {
        /* Run only the first time */
        if (bit_is_clear(UPDI_CONTROL, UPDI::UPDI_INIT_bp)) {
          uint8_t hv_control = packet.body[RSP_DATA];
          bool hv_active = false;
          if ((hv_control & '0') == '0') {
            /* If the general reset setting is neither 0 nor 1. */
            /* Accepts special extension settings.              */
            /* This forces HV control.                          */
            updi_desc.hvupdi_variant = hv_control;
            /* '1' must be passed for automatic HV control to be inhibited */
            if (hv_control != '1') hv_active = true;
          }
          /* Here UPDI control is tried */
          UPDI::updi_activate(hv_active);
          if (bit_is_set(UPDI_CONTROL, UPDI::UPDI_TERM_bp)) {
            /* Disable WDT when interactive mode is enabled */
            TIM::LED_Blink();
            SYS::WDT_OFF();
          }
          else {
            /* If not set to interactive mode, the LED will flash rapidly */
            TIM::LED_Fast();
          }
          bit_set(UPDI_CONTROL, UPDI::UPDI_INIT_bp);
        }
        #ifdef ENABLE_DEBUG_UPDI_SENDER
        UPDI::_send_buf_copy();
        #endif
        break;
      }
      case CMND_READ_MEMORY : {
        if (!UPDI::runtime(UPDI::UPDI_CMD_READ_MEMORY)) {
          set_response(RSP_NO_TARGET_POWER);
        }
        break;
      }
      case CMND_WRITE_MEMORY : {
        /* Received packet error retransmission exception */
        if (before_seqnum == packet.number) break;
        if (UPDI::runtime(UPDI::UPDI_CMD_WRITE_MEMORY)) {
          /* Keep the sequence number if completed successfully */
          before_seqnum = packet.number;
        }
        else {
          set_response(RSP_ILLEGAL_MCU_STATE);
        }
        #ifdef ENABLE_DEBUG_UPDI_SENDER
        UPDI::_send_buf_copy();
        #endif
        break;
      }
      case CMND_XMEGA_ERASE : {
        /* Received packet error retransmission exception */
        if (before_seqnum == packet.number) break;
        if (UPDI::runtime(UPDI::UPDI_CMD_ERASE)) {
          /* Keep the sequence number if completed successfully */
          before_seqnum = packet.number;
        }
        else {
          set_response(RSP_ILLEGAL_POWER_STATE);
        }
        #ifdef ENABLE_DEBUG_UPDI_SENDER
        UPDI::_send_buf_copy();
        #endif
        break;
      }
      case CMND_SET_UPDI_PARAMS :
      case CMND_SET_DEVICE_DESC : {
        set_descripter(message_id);
      }
      /* Returns affirmative for all unsupported commands */
      case CMND_SET_XMEGA_PARAMS :
      case CMND_ENTER_PROGMODE :
      case CMND_LEAVE_PROGMODE :
      case CMND_GO :
      case CMND_GET_SYNC : {
        break;
      }
      case CMND_SIGN_OFF : {
        answer_transfer();
        flush();
        if (bit_is_set(UPDI_CONTROL, UPDI::UPDI_PROG_bp))
          UPDI::runtime(UPDI::UPDI_CMD_GO);
        /* After all processing is completed, reset itself */
        // SYS::WDT_REBOOT();
        SYS::System_Reset();
      }
      /* Returns negation for unknown commands */
      default : {
        set_response(RSP_FAILED);
      }
    }
    answer_transfer();
  }
}

/******************
 * Initialization *
 ******************/

void JTAG2::setup (void) {
  JTAG_USART.BAUD = pgm_read_word( &BAUD_TABLE[BAUD_19200] );
  JTAG_USART.CTRLA = JTAG_USART_CTRLA;
  JTAG_USART.CTRLC = JTAG_USART_CTRLC;
  JTAG_USART.CTRLB = JTAG_USART_OFF;
}

/*****************
 * JTAG Response *
 *****************/

void JTAG2::set_response (jtag_response_e response_code) {
  packet.size_word[0] = 4;
  /* response number */
  packet.body[MESSAGE_ID] = response_code;
  /* Internal status flag */
  packet.body[RSP_DATA    ] = UPDI_CONTROL;
  packet.body[RSP_DATA + 1] = UPDI_NVMCTRL;
  packet.body[RSP_DATA + 2] = UPDI_LASTL;
}

/*************
 * Main loop *
 *************/

void JTAG2::wakeup_jtag (void) {
  for (;;) {
    if (packet_receive()) process_command();
  }
}

// end of code

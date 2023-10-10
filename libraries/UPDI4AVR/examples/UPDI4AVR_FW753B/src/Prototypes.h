/**
 * @file Prototypes.h
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <api/macro_api.h>
#include <peripheral.h>
#include <setjmp.h>
#include "../Configuration.h"

#if !defined(__AVR_ATtiny826__) \
 && !defined(__AVR_ATtiny1626__) \
 && !defined(__AVR_ATtiny3226__) \
 && !defined(__AVR_ATtiny827__) \
 && !defined(__AVR_ATtiny1627__) \
 && !defined(__AVR_ATtiny3227__)
  #error This MCU family is not supported tinyAVR-2 series 20 pins or more and SRAM 1KB or more only
  #include BUILD_STOP
#endif

/*
 * UPDI4AVR Firmware Version
 */

#define VER_M_MAJ 2
#define VER_M_MIN 1
#define VER_S_MAJ 7 /* Version 7 or more only */
#define VER_S_MIN 53

/*
 */

#define UPDI_CONTROL GPIO_GPIOR0
#define UPDI_NVMCTRL GPIO_GPIOR1
#define UPDI_LASTL   GPIO_GPIOR2
#define UPDI_LASTH   GPIO_GPIOR3

/*
 * Prototypes
 */

namespace SYS {
  void setup (void);
  uint16_t get_vcc (void);
  void System_Reset (void);
  void ready (void);
  void PG_Enable (void);
  void PG_Disable (void);
  void RTS_Enable (void);
  void RTS_Disable (void);
  void LED_Invert (void);
  void WDT_SET (uint8_t _wdt_period);
  void WDT_OFF (void);
  void WDT_ON (void);
  void WDT_REBOOT (void);
} // end of SYS

namespace TIM {
  extern jmp_buf CONTEXT;
  void setup (void);
  void Timeout_Start (uint16_t _count);
  void Timeout_Stop (void) __attribute__ ((noinline));
  void LED_HeartBeat (void);
  void LED_Flash (void);
  void LED_Blink (void);
  void LED_Stop (void);
  void LED_Fast (void);
  void HV_Pulse_ON (void);
  void HV_Pulse_OFF (void);
} // end of TIM

namespace UPDI {
  enum updi_control_e {       /* UPDI_CONTROL bitposition */
      UPDI_INFO_bp      = 0   /*   1 UPDI通信許可で真 */
    , UPDI_PROG_bp      = 1   /*   2 UPDI書込許可で真（偽==デバイス施錠）*/
    , UPDI_ERFM_bp      = 2   /*   4 消去操作実行で真 */
    , UPDI_ERHV_bp      = 3   /*   8 HV操作実行で真 */
    , UPDI_FCHV_bp      = 4   /*  16 HV強制許可 */
    , UPDI_URWR_bp      = 5   /*  32 USERROW特殊書込 */
    , UPDI_TERM_bp      = 6   /*  64 terminal mode */
    , UPDI_CLKU_bp      = 7   /* 128 UPDI拘束動作で真 */
  };
  enum updi_nvmctrl_p {
      UPDI_BROW_bp      = 4   /*  16 BOOTROWが存在すると真 (AVR_EB) */
    , UPDI_LOWF_bp      = 5   /*  32 Flash先頭が0x4000で真（==megaAVR）*/
    , UPDI_GEN2_bp      = 6   /*  64 AVR_Dx以降で真（偽==FlashはData空間内）HV==8V */
    , UPDI_GEN3_bp      = 7   /* 128 AVR_Exで真（偽==FlashはData空間内）*/
  };

  enum updi_command_e {
      UPDI_CMD_READ_MEMORY      = 1
    , UPDI_CMD_WRITE_MEMORY
    , UPDI_CMD_ERASE
    , UPDI_CMD_GO
  };

  enum updi_operate_e {
    /* UPDI command */
      UPDI_NOP                  = 0x00
    , UPDI_SYNCH                = 0x55
    , UPDI_ACK                  = 0x40
    , UPDI_LDS                  = 0x00
    , UPDI_STS                  = 0x40
    , UPDI_LD                   = 0x20
    , UPDI_ST                   = 0x60
    , UPDI_LDCS                 = 0x80
    , UPDI_STCS                 = 0xC0
    , UPDI_REPEAT               = 0xA0
    , UPDI_RSTREQ               = 0x59
    /* KEY length */
    , UPDI_KEY_64               = 0xE0
    , UPDI_KEY_128              = 0xE1
    , UPDI_SIB_64               = 0xE4
    , UPDI_SIB_128              = 0xE5
    /* ADDR size */
    , UPDI_ADDR1                = 0x00
    , UPDI_ADDR2                = 0x04
    , UPDI_ADDR3                = 0x08
    /* DATA size */
    , UPDI_DATA1                = 0x00
    , UPDI_DATA2                = 0x01
    , UPDI_DATA3                = 0x02
    /* PoinTeR type  */
    , UPDI_PTR_IMD              = 0x00
    , UPDI_PTR_INC              = 0x04
    , UPDI_PTR_REG              = 0x08
    /* control system register */
    , UPDI_CS_STATUSA           = 0x00
    , UPDI_CS_STATUSB           = 0x01
    , UPDI_CS_CTRLA             = 0x02
    , UPDI_CS_CTRLB             = 0x03
    , UPDI_CS_ASI_KEY_STATUS    = 0x07
    , UPDI_CS_ASI_RESET_REQ     = 0x08
    , UPDI_CS_ASI_CTRLA         = 0x09
    , UPDI_CS_ASI_SYS_CTRLA     = 0x0A
    , UPDI_CS_ASI_SYS_STATUS    = 0x0B
    , UPDI_CS_ASI_CRC_STATUS    = 0x0C
  };

  enum updi_status_e {
      UPDI_KEY_UROWWRITE        = 0x20  // ASI_KEY_STATUS
    , UPDI_KEY_NVMPROG          = 0x10
    , UPSI_KEY_CHIPERASE        = 0x08
    , UPDI_SYS_RSTSYS           = 0x20  // ASI_SYS_STATUS
    , UPDI_SYS_INSLEEP          = 0x10
    , UPDI_SYS_NVMPROG          = 0x08
    , UPDI_SYS_UROWPROG         = 0x04
    , UPDI_SYS_LOCKSTATUS       = 0x01
    , UPDI_CRC_STATUS_gm        = 0x07  // ASI_CRC_STATUS
  };

  enum updi_bitset_e {
      UPDI_ERR_PESIG_gm         = 0x07 // STATUSB
    , UPDI_SET_IBDLY            = 0x80 // CTRLA
    , UPDI_SET_PARD             = 0x20
    , UPDI_SET_DTD              = 0x10
    , UPDI_SET_RSD              = 0x08
    , UPDI_SET_GTVAL_gm         = 0x07
    , UPDI_SET_GTVAL_128        = 0x00
    , UPDI_SET_GTVAL_64         = 0x01
    , UPDI_SET_GTVAL_32         = 0x02
    , UPDI_SET_GTVAL_16         = 0x03
    , UPDI_SET_GTVAL_8          = 0x04
    , UPDI_SET_GTVAL_4          = 0x05
    , UPDI_SET_GTVAL_2          = 0x06
    , UPDI_SET_NACKDIS          = 0x10 // CTRLB
    , UPDI_SET_CCDETDIS         = 0x08
    , UPDI_SET_UPDIDIS          = 0x04
    , UPDI_SET_UPDICLKSEL_bm    = 0x03 // ASI_CTRLA
    , UPDI_SET_UPDICLKSEL_32M   = 0x00 // 1800kbps
    , UPDI_SET_UPDICLKSEL_16M   = 0x01 // 900kbps
    , UPDI_SET_UPDICLKSEL_8M    = 0x02 // 450kbps
    , UPDI_SET_UPDICLKSEL_4M    = 0x03 // 225kbps
    , UPDI_SET_UROWDONE         = 0x02 // ASI_SYS_CTRLA
    , UPDI_SET_CLKREQ           = 0x01
  };

  void guardtime (void);
  void setup (void);
  uint8_t RECV (void);
  bool SEND (uint8_t _data);
  void BREAK (void);
  bool Target_Reset (bool _enable);
  bool send_bytes (const uint8_t *_data, size_t _len);
  bool send_repeat_header (uint8_t cmd, uint32_t addr, size_t len);
  bool st8 (uint32_t addr, uint8_t data);
  bool sts8 (uint32_t addr, uint8_t *data, size_t len);
  uint8_t ld8 (uint32_t addr);
  uint8_t get_cs_stat (const uint8_t code);
  bool is_cs_stat (const uint8_t code, uint8_t check);
  bool is_sys_stat (const uint8_t check);
  bool is_key_stat (const uint8_t check);
  bool is_rst_stat (void);
  bool set_cs_stat (const uint8_t code, uint8_t data);
  bool set_cs_ctra (const uint8_t data);
  bool set_cs_asi_ctra (const uint8_t data);
  uint8_t get_cs_asi_ctra (void);
  bool updi_reset (bool logic);
  bool enter_updi (bool skip = false);
  bool enter_prog (void);
  bool check_sig (void);
  bool enter_userrow (void);
  bool chip_erase (void);
  bool updi_activate (void);
  bool runtime (uint8_t updi_cmd);
} // end of UPDI

namespace NVM {
  enum nvm_register_e {
    /* register */
      NVMCTRL_REG_CTRLA    = 0x1000
    , NVMCTRL_REG_CTRLB    = 0x1001
    , NVMCTRL_REG_STATUS   = 0x1002
    , NVMCTRL_REG_INTCTRL  = 0x1003
    , NVMCTRL_REG_INTFLAGS = 0x1004
    , NVMCTRL_REG_DATA     = 0x1006
    , NVMCTRL_REG_DATAL    = 0x1006
    , NVMCTRL_REG_DATAH    = 0x1007
    , NVMCTRL_REG_ADDR     = 0x1008
    , NVMCTRL_REG_ADDRL    = 0x1008
    , NVMCTRL_REG_ADDRH    = 0x1009
    , NVMCTRL_REG_ADDRZ    = 0x100A
  };
  /* NVMCTRL v0 */
  enum nvm_control_e {
      NVM_CMD_WP   = 1
    , NVM_CMD_ER   = 2
    , NVM_CMD_ERWP = 3
    , NVM_CMD_PBC  = 4
    , NVM_CMD_CHER = 5
    , NVM_CMD_EEER = 6
    , NVM_CMD_WFU  = 7
  };
  /* NVMCTRL v2 */
  enum nvm_control_v2_e {
      NVM_V2_CMD_NOCMD      = 0x00  /* NVM_V3_CMD_NOCMD */
    , NVM_V2_CMD_NOOP       = 0x01  /* NVM_V3_CMD_NOOP */
    , NVM_V2_CMD_FLWR       = 0x02  /* v2 only */
    , NVM_V2_CMD_FLPER      = 0x08  /* NVM_V3_CMD_FLPER */
    , NVM_V2_CMD_FLMPER2    = 0x09  /* NVM_V3_CMD_FLMPER2 */
    , NVM_V2_CMD_FLMPER4    = 0x0A  /* NVM_V3_CMD_FLMPER4 */
    , NVM_V2_CMD_FLMPER8    = 0x0B  /* NVM_V3_CMD_FLMPER8 */
    , NVM_V2_CMD_FLMPER16   = 0x0C  /* NVM_V3_CMD_FLMPER16 */
    , NVM_V2_CMD_FLMPER32   = 0x0D  /* NVM_V3_CMD_FLMPER32 */
    , NVM_V2_CMD_EEWR       = 0x12  /* v2 only */
    , NVM_V2_CMD_EEERWR     = 0x13  /* v2 only */
    , NVM_V2_CMD_EEBER      = 0x18  /* v2 only */
    , NVM_V2_CMD_EEMBER2    = 0x19  /* v2 only */
    , NVM_V2_CMD_EEMBER4    = 0x1A  /* v2 only */
    , NVM_V2_CMD_EEMBER8    = 0x1B  /* v2 only */
    , NVM_V2_CMD_EEMBER16   = 0x1C  /* v2 only */
    , NVM_V2_CMD_EEMBER32   = 0x1D  /* v2 only */
    , NVM_V2_CMD_CHER       = 0x20  /* NVM_V2_CMD_CHER */
    , NVM_V2_CMD_EECHER     = 0x30  /* NVM_V3_CMD_EECHER */
  };
  /* NVMCTRL v3 */
  enum nvm_register_v3_e {
    /* register */
      NVMCTRL_V3_REG_CTRLA    = 0x1000  /* v1/v2と共通 */
    , NVMCTRL_V3_REG_CTRLB    = 0x1001  /* v1/v2と共通 */
    , NVMCTRL_V3_REG_INTCTRL  = 0x1004
    , NVMCTRL_V3_REG_INTFLAGS = 0x1005
    , NVMCTRL_V3_REG_STATUS   = 0x1006
    , NVMCTRL_V3_REG_DATA     = 0x1008
    , NVMCTRL_V3_REG_DATAL    = 0x1008
    , NVMCTRL_V3_REG_DATAH    = 0x1009
    , NVMCTRL_V3_REG_ADDR     = 0x100C
    , NVMCTRL_V3_REG_ADDRL    = 0x100C
    , NVMCTRL_V3_REG_ADDRH    = 0x100D
    , NVMCTRL_V3_REG_ADDRZ    = 0x100E
  };
  enum nvm_control_v3_e {
    /* command */
      NVM_V3_CMD_NOCMD      = 0x00  /* NVM_V2_CMD_NOCMD */
    , NVM_V3_CMD_NOOP       = 0x01  /* NVM_V2_CMD_NOOP */
    , NVM_V3_CMD_FLPW       = 0x04  /* v3 only */
    , NVM_V3_CMD_FLPERW     = 0x05  /* v3 only */
    , NVM_V3_CMD_FLPER      = 0x08  /* NVM_V2_CMD_FLPER */
    , NVM_V3_CMD_FLMPER2    = 0x09  /* NVM_V2_CMD_FLMPER2 */
    , NVM_V3_CMD_FLMPER4    = 0x0A  /* NVM_V2_CMD_FLMPER4 */
    , NVM_V3_CMD_FLMPER8    = 0x0B  /* NVM_V2_CMD_FLMPER8 */
    , NVM_V3_CMD_FLMPER16   = 0x0C  /* NVM_V2_CMD_FLMPER16 */
    , NVM_V3_CMD_FLMPER32   = 0x0D  /* NVM_V2_CMD_FLMPER32 */
    , NVM_V3_CMD_FLPBCLR    = 0x0F  /* v3 only */
    , NVM_V3_CMD_EEPW       = 0x14  /* v3 only */
    , NVM_V3_CMD_EEPERW     = 0x15  /* v3 only */
    , NVM_V3_CMD_EEPER      = 0x17  /* v3 only */
    , NVM_V3_CMD_EEPBCLR    = 0x1F  /* v3 only */
    , NVM_V3_CMD_CHER       = 0x20  /* NVM_V2_CMD_CHER */
    , NVM_V3_CMD_EECHER     = 0x30  /* NVM_V2_CMD_EECHER */
  };
  enum avr_base_addr_e {
      BASE_NVMCTRL = 0x1000
    , BASE_FUSE    = 0x1050
    , BASE_SIGROW  = 0x1100
  };
  enum avr_base_eb_addr_e {
      EB_SIGROW    = 0x1080
    , EB_USERROW   = 0x1200
    , EB_BOOTROW   = 0x1300
  };

  extern volatile uint32_t nvm_flash_offset;
  extern volatile uint32_t nvm_eeprom_offset;
  extern volatile uint32_t nvm_fuse_offset;
  extern volatile uint32_t nvm_user_sig_offset;
  extern volatile uint32_t nvm_data_offset;
  extern volatile uint16_t flash_page_size;

  bool read_memory (void);
  bool write_memory (void);

  bool read_data (uint32_t start_addr, size_t byte_count);
  bool read_flash (uint32_t start_addr, size_t byte_count);
  bool read_userrow_dummy (size_t byte_count);

  uint8_t nvm_wait (void);
  bool nvm_ctrl (uint8_t nvmcmd);

  bool write_fuse (uint16_t addr, uint8_t data);
  bool write_eeprom (uint32_t start_addr, size_t byte_count);
  bool write_flash (uint32_t start_addr, size_t byte_count);

  bool nvm_ctrl_v2 (uint8_t nvmcmd);
  bool write_fuse_v2 (uint16_t addr, uint8_t data);
  bool write_eeprom_v2 (uint32_t start_addr, size_t byte_count);
  bool write_flash_v2 (uint32_t start_addr, size_t byte_count);

  uint8_t nvm_wait_v3 (void);
  bool nvm_ctrl_v3 (uint8_t nvmcmd);
  bool write_fuse_v3 (uint16_t addr, uint8_t data);
  bool write_eeprom_v3 (uint32_t start_addr, size_t byte_count);
  bool write_flash_v3 (uint32_t start_addr, size_t byte_count);

  bool write_userrow (size_t byte_count);

} // end of NVM

namespace JTAG2 {
  /* See documentation AVR067: JTAGICE mkII Communication Protocol Manual */

  /* Parameters IDs */
  enum jtag_parameter_e {
      PARAM_HW_VER    = 0x01
    , PARAM_FW_VER    = 0x02
    , PARAM_EMU_MODE  = 0x03
    , PARAM_BAUD_RATE = 0x05
    , PARAM_VTARGET   = 0x06
  };

  /* valid values for PARAM_BAUD_RATE_VAL */
  /* Changing CMT_SET_PARAMETER command */
  enum jtag_baud_rate_e {
      BAUD_NOTUSED
    , BAUD_2400         // 1: F_CPU 9.83Mhz lower limit
    , BAUD_4800         // 2: F_CPU 19.67MHz lower limit
    , BAUD_9600         // 3: supported min
    , BAUD_19200        // 4: JTAG2 startup default speed
    , BAUD_38400        // 5: terminal mode active
    , BAUD_57600
    , BAUD_115200       // 7: normal : avrdude 6.x max speed
    , BAUD_14400        // 8: using avrdude 6.x lower
    , BAUD_153600       // 9: using avrdude 7.x upper
    , BAUD_230400       // 10: UPDI4AVR standard speed
    , BAUD_460800
    , BAUD_921600
    , BAUD_128000
    , BAUD_256000
    , BAUD_512000       // 16: CH340E not supported
    , BAUD_1024000      // 17: CH340E not supported
    , BAUD_150000
    , BAUD_200000
    , BAUD_250000
    , BAUD_300000
    , BAUD_400000
    , BAUD_500000
    , BAUD_600000
    , BAUD_666666       // 24: terminal mode active
    , BAUD_1000000      // 25: F_CPU 10MHz or more
    , BAUD_1500000      // 26: F_CPU 12MHz or more
    , BAUD_2000000      // 27: F_CPU 16MHz or more
    , BAUD_3000000      // 28: F_CPU 24MHz or more

#if (F_CPU / 2400 < 4096)
    , BAUD_LOWER = BAUD_2400
#elif (F_CPU / 4800 < 4096)
    , BAUD_LOWER = BAUD_4800
#elif (F_CPU / 9600 < 4096)
    , BAUD_LOWER = BAUD_9600
#else
    , BAUD_LOWER = BAUD_19200
#endif

#if (F_CPU >= 24000000UL)
    , BAUD_UPPER = BAUD_3000000
#elif (F_CPU >= 16000000UL)
    , BAUD_UPPER = BAUD_2000000
#elif (F_CPU >= 12000000UL)
    , BAUD_UPPER = BAUD_1500000
#elif (F_CPU >= 8000000UL)
    , BAUD_UPPER = BAUD_1000000
#else
    , BAUD_UPPER = BAUD_115200
#endif

  };

  /* JTAG2::CONTROL flags */
  enum jtag_control_e {
      HOST_SIGN_ON              = 0x01
    , USART_TX_EN               = 0x02
    , CHANGE_BAUD               = 0x04
    , ANS_FAILED                = 0x80
  };

  /* Master Commands IDs */
  enum jtag_cmnd_e {
      CMND_SIGN_OFF             = 0x00
    , CMND_GET_SIGN_ON          = 0x01
    , CMND_SET_PARAMETER        = 0x02
    , CMND_GET_PARAMETER        = 0x03
    , CMND_WRITE_MEMORY         = 0x04
    , CMND_READ_MEMORY          = 0x05
    , CMND_GO                   = 0x08
    , CMND_RESET                = 0x0B
    , CMND_SET_DEVICE_DESCRIPTOR= 0x0C
    , CMND_GET_SYNC             = 0x0F
    , CMND_ENTER_PROGMODE       = 0x14
    , CMND_LEAVE_PROGMODE       = 0x15
    , CMND_XMEGA_ERASE          = 0x34
    , CMND_SET_XMEGA_PARAMS     = 0x36 /* Undocumented, upper FW 7.xx */
  };

  /* Single byte response IDs */
  enum jtag_response_e {
    // Success
      RSP_OK                    = 0x80
    , RSP_PARAMETER             = 0x81
    , RSP_MEMORY                = 0x82
    , RSP_SIGN_ON               = 0x86
    // Error
    , RSP_FAILED                = 0xA0
    , RSP_ILLEGAL_PARAMETER     = 0xA1
    , RSP_ILLEGAL_MEMORY_TYPE   = 0xA2
    , RSP_ILLEGAL_MEMORY_RANGE  = 0xA3
    , RSP_ILLEGAL_MCU_STATE     = 0xA5
    , RSP_ILLEGAL_VALUE         = 0xA6
    , RSP_ILLEGAL_BREAKPOINT    = 0xA8
    , RSP_ILLEGAL_JTAG_ID       = 0xA9
    , RSP_ILLEGAL_COMMAND       = 0xAA
    , RSP_NO_TARGET_POWER       = 0xAB
    , RSP_DEBUGWIRE_SYNC_FAILED = 0xAC
    , RSP_ILLEGAL_POWER_STATE   = 0xAD
  };

  /* Memory type message sub-command */
  enum jtag_mem_type_e {
      MTYPE_IO_SHADOW           = 0x30  // cached IO registers?
    , MTYPE_SRAM                = 0x20  // target's SRAM or [ext.] IO registers
    , MTYPE_EEPROM              = 0x22  // EEPROM, what way?
    , MTYPE_EVENT               = 0x60  // ICE event memory
    , MTYPE_SPM                 = 0xA0  // flash through LPM/SPM
    , MTYPE_FLASH_PAGE          = 0xB0  // flash in programming mode
    , MTYPE_EEPROM_PAGE         = 0xB1  // EEPROM in programming mode
    , MTYPE_FUSE_BITS           = 0xB2  // fuse bits in programming mode
    , MTYPE_LOCK_BITS           = 0xB3  // lock bits in programming mode
    , MTYPE_SIGN_JTAG           = 0xB4  // signature in programming mode
    , MTYPE_OSCCAL_BYTE         = 0xB5  // osccal cells in programming mode
    , MTYPE_CAN                 = 0xB6  // CAN mailbox
    , MTYPE_XMEGA_FLASH         = 0xC0  // xmega (app.) flash
    , MTYPE_BOOT_FLASH          = 0xC1  // xmega boot flash
    , MTYPE_EEPROM_XMEGA        = 0xC4  // xmega EEPROM in debug mode
    , MTYPE_USERSIG             = 0xC5  // xmega user signature
    , MTYPE_PRODSIG             = 0xC6  // xmega production signature
  };

  /* CMND_XMEGA_ERASE sub-command */
  enum jtag_erase_mode_e {
      XMEGA_ERASE_CHIP
    , XMEGA_ERASE_APP
    , XMEGA_ERASE_BOOT
    , XMEGA_ERASE_EEPROM
    , XMEGA_ERASE_APP_PAGE
    , XMEGA_ERASE_BOOT_PAGE
    , XMEGA_ERASE_EEPROM_PAGE
    , XMEGA_ERASE_USERSIG
  };

  /* JTAG2 packet */
  enum jtag_packet_e {
      MESSAGE_START = 0x1B          /* SOH */
    , TOKEN         = 0x0E          /* STX */
    , MAX_BODY_SIZE = 512 + 4 + 4 + 1
  };
  union jtag_packet_t {
    uint8_t _pad;                   // alignment padding
    uint8_t raw[MAX_BODY_SIZE + 1 + 4 + 4 + 1 + 2];
    struct {
      uint8_t soh;                  // $00:1 $1B
      union {                       // $01:2
        uint16_t number;
        uint8_t number_byte[2];
      };
      union {                       // $03:4=N
        uint32_t size;
        int16_t size_word[2];
        uint8_t size_byte[4];
      };
      uint8_t stx;                  // $07:1 $0E
      uint8_t body[MAX_BODY_SIZE];  // $08:N
      uint8_t _crc[2];              // $08+N:2
    };
  } extern packet;

  /* pblic methods */
  void setup (void);
  void transfer_enable (void);
  void transfer_disable (void);
  uint8_t get (void);
  uint8_t put (uint8_t data);
  void flush (void);
  uint16_t crc16_update(uint16_t crc, uint8_t data);
  bool packet_receive (void);
  void answer_transfer (void);
  void set_response (jtag_response_e response_code);
  void sign_on_response (void);
  bool set_parameter (void);
  void get_parameter (void);
  void process_command (void);
  void wakeup_jtag (void);
} // end of JTAG2

// end of header

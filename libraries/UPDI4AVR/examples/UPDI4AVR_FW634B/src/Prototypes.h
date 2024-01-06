/**
 * @file Prototypes.h
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.3
 * @date 2023-12-24
 *
 * @copyright Copyright (c) 2023 askn37 at github.com
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
  #include "BUILD_STOP"
#endif

/*****************************
 * UPDI4AVR Firmware Version *
 *****************************/

/* Main units emulated */
#define VER_M_MAJ 7
#define VER_M_MIN 53

/* Sub units emulated */
#define VER_S_MAJ 6
#define VER_S_MIN 34

/***************************************
 * Bit-flag / general purpose register *
 ***************************************/

#define UPDI_CONTROL GPIO_GPIOR0
#define UPDI_NVMCTRL GPIO_GPIOR1
#define UPDI_LASTL   GPIO_GPIOR2
#define UPDI_LASTH   GPIO_GPIOR3

#define bit_set(sfr,bit)    (sfr |= _BV(bit))
#define bit_clear(sfr,bit)  (sfr &= ~_BV(bit))

/**************
 * Prototypes *
 **************/

namespace SYS {
  void init (void);
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
  void WDT_Short (void);
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
  void delay_50us (void);
  void delay_800us (void);
  void delay_200ms (void);
} // end of TIM

namespace UPDI {
  enum updi_control_e {   /* UPDI_CONTROL bitposition (true)    */
      UPDI_INFO_bp  = 0   /*    1 UPDI communication permission */
    , UPDI_PROG_bp  = 1   /*    2 UPDI write permission (false == device locked) */
    , UPDI_ERFM_bp  = 2   /*    4 performing an erase operation */
    , UPDI_ERHV_bp  = 3   /*    8 executing HV operation        */
    , UPDI_FCHV_bp  = 4   /* 0x10 HV compulsory permit          */
    , UPDI_TERM_bp  = 5   /* 0x20 Terminal mode                 */
    , UPDI_CLKU_bp  = 6   /* 0x40 UPDI double speed operation   */
    , UPDI_INIT_bp  = 7   /* 0x80 Initialization completion     */
  };
  enum updi_nvmctrl_p {   /* UPDI_NVMCTRL bitposition (true)    */
      UPDI_GEN5_bp  = 3   /*    8 AVR_EB BOOTROW exists         */
    , UPDI_GEN4_bp  = 4   /* 0x10 AVR_DU BOOTROW exists         */
    , UPDI_GEN3_bp  = 5   /* 0x20 AVR_Ex or later               */
    , UPDI_GEN2_bp  = 6   /* 0x40 AVR_Dx or later HV=8.2V       */
    , UPDI_LOWF_bp  = 7   /* 0x80 High-Flash starts with 0x4000（=megaAVR）*/
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
    , UPDI_SIB_256              = 0xE6  // undocumented tricks
    /* ADDR size */
    , UPDI_ADDR1                = 0x00  // 8bit (not used)
    , UPDI_ADDR2                = 0x04  // 16bit
    , UPDI_ADDR3                = 0x08  // 24bit
    /* DATA size */
    , UPDI_DATA1                = 0x00  // 8bit
    , UPDI_DATA2                = 0x01  // 16bit
    , UPDI_DATA3                = 0x02  // 24bit (PTR settings only)
    /* PoinTeR type  */
    , UPDI_PTR_IMD              = 0x00  // Immediate
    , UPDI_PTR_INC              = 0x04  // Incremental After
    , UPDI_PTR_REG              = 0x08  // Registered
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
    /* ASI_KEY_STATUS */
      UPDI_KEY_UROWWRITE        = 0x20
    , UPDI_KEY_NVMPROG          = 0x10
    , UPDI_KEY_CHIPERASE        = 0x08
    /* ASI_SYS_STATUS */
    , UPDI_SYS_LOCKSTATUS       = 0x01  // (all)
    , UPDI_SYS_BOOTDONE         = 0x02  // (v3~)
    , UPDI_SYS_UROWPROG         = 0x04  // (all)
    , UPDI_SYS_NVMPROG          = 0x08  // (all)
    , UPDI_SYS_INSLEEP          = 0x10  // (all)
    , UPDI_SYS_RSTSYS           = 0x20  // (all)
    , UPDI_SYS_ERASEFAIL        = 0x40  // (v2~)
    , UPDI_SYS_BDEF             = 0x80  // (v3~)
    , UPDI_SYS_LOCKSTATUS_bp    = 0
    , UPDI_SYS_BOOTDONE_bp      = 1
    , UPDI_SYS_UROWPROG_bp      = 2
    , UPDI_SYS_NVMPROG_bp       = 3
    , UPDI_SYS_INSLEEP_bp       = 4
    , UPDI_SYS_RSTSYS_bp        = 5
    , UPDI_SYS_ERASEFAIL_bp     = 6
    , UPDI_SYS_BDEF_bp          = 7
    /* ASI_CRC_STATUS */
    , UPDI_CRC_STATUS_gm        = 0x07
  };

  enum updi_bitset_e {
    /* STATUSB */
      UPDI_ERR_PESIG_gm         = 0x07
    /* CTRLA */
    , UPDI_SET_IBDLY            = 0x80
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
    /* CTRLB */
    , UPDI_SET_NACKDIS          = 0x10
    , UPDI_SET_CCDETDIS         = 0x08
    , UPDI_SET_UPDIDIS          = 0x04
    /* ASI_CTRLA */
    , UPDI_SET_UPDICLKSEL_bm    = 0x03
    , UPDI_SET_UPDICLKSEL_32M   = 0x00  // 1800kbps (OSD only : After AVR_DA)
    , UPDI_SET_UPDICLKSEL_16M   = 0x01  // 900kbps (OSD only)
    , UPDI_SET_UPDICLKSEL_8M    = 0x02  // 450kbps (AVR_DA/DB can be written with this)
    , UPDI_SET_UPDICLKSEL_4M    = 0x03  // 225kbps (Normal)
    /* ASI_SYS_CTRLA */
    , UPDI_SET_UROWDONE         = 0x02
    , UPDI_SET_CLKREQ           = 0x01  // (should always be set)
  };

  enum updi_command_e {
      UPDI_CMD_READ_MEMORY      = 1
    , UPDI_CMD_WRITE_MEMORY     = 2
    , UPDI_CMD_ERASE            = 3
    , UPDI_CMD_GO               = 4
  };

  #ifdef ENABLE_DEBUG_UPDI_SENDER
  extern uint16_t _send_ptr;
  void _send_buf_clear (void);
  size_t _send_buf_copy (void);
  void _send_buf_push (uint8_t data);
  #endif

  void setup (void);
  bool Target_Reset (bool _enable);
  bool updi_reset (bool logic);

  uint8_t RECV (void);
  bool SEND (uint8_t _data);
  void BREAK (void);
  void STOP (void);
  bool send_bytes (uint8_t *_data, uint8_t _len);
  bool send_repeat_header (uint32_t addr, uint8_t cmd, uint8_t len);
  bool st8 (uint32_t addr, uint8_t data);
  bool sts8 (uint32_t addr, uint8_t *data, uint8_t len);
  bool sts8rsd (uint32_t addr, uint8_t *data, uint8_t len);
  bool sts16 (uint32_t addr, uint8_t *data, size_t len);
  bool sts16rsd (uint32_t addr, uint8_t *data, size_t len);
  uint8_t ld8 (uint32_t addr);
  bool lds8 (uint32_t addr, uint8_t *data, uint8_t len);
  bool lds16 (uint32_t addr, uint8_t *data, size_t len);

  uint8_t get_cs_stat (uint8_t code);
  bool is_cs_stat (uint8_t code, uint8_t check);
  bool is_sys_stat (uint8_t check);
  bool is_key_stat (uint8_t check);
  bool set_cs_stat (uint8_t code, uint8_t data);
  bool set_cs_ctra (uint8_t data);
  bool set_cs_ctrb (uint8_t data);
  bool set_cs_asi_ctra (uint8_t data);
  uint8_t get_cs_asi_ctra (void);
  bool loop_until_sys_stat_is_clear (uint8_t bitmap, uint16_t limit = 0);
  bool loop_until_sys_stat_is_set (uint8_t bitmap, uint16_t limit = 0);
  bool loop_until_key_stat_is_clear (uint8_t bitmap, uint16_t limit = 0);
  bool loop_until_key_stat_is_set (uint8_t bitmap, uint16_t limit = 0);

  bool read_sib (uint8_t *s_ptr);
  void HV_Pulse (void);
  bool set_nvmprog_key (void);
  bool set_erase_key (void);
  bool set_urowwrite_key (void);
  bool write_userrow (uint32_t start_addr, uint8_t *data, size_t byte_count);
  bool chip_erase (void);
  bool enter_updi (bool skip = false);
  bool enter_prog (void);
  bool updi_activate (bool hv_active);
  bool runtime (uint8_t updi_cmd);
} // end of UPDI

namespace NVM {
  /* NVMCTRL version 0,2 */
  enum nvm_register_v02_e {
    /* register */
      NVMCTRL_REG_CTRLA     = 0x1000
    , NVMCTRL_REG_CTRLB     = 0x1001
    , NVMCTRL_REG_STATUS    = 0x1002
    , NVMCTRL_REG_INTCTRL   = 0x1003
    , NVMCTRL_REG_INTFLAGS  = 0x1004
    , NVMCTRL_REG_DATA      = 0x1006
    , NVMCTRL_REG_DATAL     = 0x1006
    , NVMCTRL_REG_DATAH     = 0x1007
    , NVMCTRL_REG_ADDR      = 0x1008
    , NVMCTRL_REG_ADDRL     = 0x1008
    , NVMCTRL_REG_ADDRH     = 0x1009
    , NVMCTRL_REG_ADDRZ     = 0x100A  /* Not in v0 */
  };
  /* NVMCTRL version 3,5 */
  enum nvm_register_v35_e {
    /* register */
      NVMCTRL_V3_REG_CTRLA    = 0x1000
    , NVMCTRL_V3_REG_CTRLB    = 0x1001
    , NVMCTRL_V5_REG_CTRLC    = 0x1002  /* v5 only */
    , NVMCTRL_V3_REG_INTCTRL  = 0x1004
    , NVMCTRL_V3_REG_INTFLAGS = 0x1005
    , NVMCTRL_V3_REG_STATUS   = 0x1006
    , NVMCTRL_V3_REG_DATA     = 0x1008
    , NVMCTRL_V3_REG_DATAL    = 0x1008
    , NVMCTRL_V3_REG_DATAH    = 0x1009
    , NVMCTRL_V3_REG_ADDR     = 0x100C
    , NVMCTRL_V3_REG_ADDR0    = 0x100C
    , NVMCTRL_V3_REG_ADDR1    = 0x100D
    , NVMCTRL_V3_REG_ADDR2    = 0x100E
  };
  /* NVMCTRL version 4 */
  enum nvm_register_v4_e {
    /* register */
      NVMCTRL_V4_REG_CTRLA    = 0x1000
    , NVMCTRL_V4_REG_CTRLB    = 0x1001
    , NVMCTRL_V4_REG_CTRLC    = 0x1002
    , NVMCTRL_V4_REG_INTCTRL  = 0x1004
    , NVMCTRL_V4_REG_INTFLAGS = 0x1005
    , NVMCTRL_V4_REG_STATUS   = 0x1006
    , NVMCTRL_V4_REG_DATA     = 0x1008
    , NVMCTRL_V4_REG_DATA0    = 0x1008
    , NVMCTRL_V4_REG_DATA1    = 0x1009
    , NVMCTRL_V4_REG_DATA2    = 0x100A
    , NVMCTRL_V4_REG_DATA3    = 0x100B
    , NVMCTRL_V4_REG_ADDR     = 0x100C
    , NVMCTRL_V4_REG_ADDR0    = 0x100C
    , NVMCTRL_V4_REG_ADDR1    = 0x100D
    , NVMCTRL_V4_REG_ADDR2    = 0x100E
  };
  /* NVMCTRL version 0 */
  enum nvm_control_v0_e {
      NVM_CMD_NOCMD = 0   /* No command */
    , NVM_CMD_WP    = 1   /* Write Page */
    , NVM_CMD_ER    = 2   /* Erase */
    , NVM_CMD_ERWP  = 3   /* Erase and Write Page */
    , NVM_CMD_PBC   = 4   /* Page BUffer Clear */
    , NVM_CMD_CHER  = 5   /* Chip Erase */
    , NVM_CMD_EEER  = 6   /* EEPROM Chip Erase */
    , NVM_CMD_WFU   = 7   /* Write FUSE */
  };
  /* NVMCTRL version 2,4 */
  enum nvm_control_v24_e {
      NVM_V2_CMD_NOCMD        = 0x00  /* No command */
    , NVM_V2_CMD_NOOP         = 0x01  /* No oparation */
    , NVM_V2_CMD_FLWR         = 0x02  /* Flash Write */
    , NVM_V2_CMD_FLPER        = 0x08  /* Flash Page Erase */
    , NVM_V2_CMD_FLMPER2      = 0x09  /* Flash 2 Page Erase */
    , NVM_V2_CMD_FLMPER4      = 0x0A  /* Flash 4 Page Erase */
    , NVM_V2_CMD_FLMPER8      = 0x0B  /* Flash 8 Page Erase */
    , NVM_V2_CMD_FLMPER16     = 0x0C  /* Flash 16 Page Erase */
    , NVM_V2_CMD_FLMPER32     = 0x0D  /* Flash 32 Page Erase */
    , NVM_V2_CMD_EEWR         = 0x12  /* EEPROM Write */
    , NVM_V2_CMD_EEERWR       = 0x13  /* EEPROM Erase and Write */
    , NVM_V2_CMD_EEBER        = 0x18  /* EEPROM Byte Erase */
    , NVM_V2_CMD_EEMBER2      = 0x19  /* EEPROM 2 Byte Erase */
    , NVM_V2_CMD_EEMBER4      = 0x1A  /* EEPROM 4 Byte Erase */
    , NVM_V2_CMD_EEMBER8      = 0x1B  /* EEPROM 8 Byte Erase */
    , NVM_V2_CMD_EEMBER16     = 0x1C  /* EEPROM 16 Byte Erase */
    , NVM_V2_CMD_EEMBER32     = 0x1D  /* EEPROM 32 Byte Erase */
    , NVM_V2_CMD_CHER         = 0x20  /* Chip Erase */
    , NVM_V2_CMD_EECHER       = 0x30  /* EEPROM Chip Erase */
  };
  /* NVMCTRL version 3,5 */
  enum nvm_control_v35_e {
    /* command */
      NVM_V3_CMD_NOCMD        = 0x00  /* No command */
    , NVM_V3_CMD_NOOP         = 0x01  /* No oparation */
    , NVM_V3_CMD_FLPW         = 0x04  /* Flash Page Write */
    , NVM_V3_CMD_FLPERW       = 0x05  /* Flash Page Erase and Write */
    , NVM_V3_CMD_FLPER        = 0x08  /* Flash Page Erase */
    , NVM_V3_CMD_FLMPER2      = 0x09  /* Flash 2 Page Erase */
    , NVM_V3_CMD_FLMPER4      = 0x0A  /* Flash 4 Page Erase */
    , NVM_V3_CMD_FLMPER8      = 0x0B  /* Flash 8 Page Erase */
    , NVM_V3_CMD_FLMPER16     = 0x0C  /* Flash 16 Page Erase */
    , NVM_V3_CMD_FLMPER32     = 0x0D  /* Flash 32 Page Erase */
    , NVM_V3_CMD_FLPBCLR      = 0x0F  /* Flash Page Buffer Clear */
    , NVM_V3_CMD_EEPW         = 0x14  /* EEPROM Page Write */
    , NVM_V3_CMD_EEPERW       = 0x15  /* EEPROM Page Erase and Write */
    , NVM_V3_CMD_EEPER        = 0x17  /* EEPROM Page Erase */
    , NVM_V3_CMD_EEPBCLR      = 0x1F  /* EEPROM Page Buffer Clear */
    , NVM_V3_CMD_CHER         = 0x20  /* Chip Erase */
    , NVM_V3_CMD_EECHER       = 0x30  /* EEPROM Chip Erase */
  };
  enum avr_base_addr_e {
      BASE_NVMCTRL = 0x1000
    , BASE_FUSE    = 0x1050
    , BASE_USERROW = 0x1080
    , BASE_SIGROW  = 0x1100
    , BASE_EEPROM  = 0x1400

    , BASE23_FUSE    = 0x1280
    , BASE23_USERROW = 0x1300

    , BASE45_SIGROW  = 0x1080
    , BASE45_BOOTROW = 0x1100
    , BASE45_USERROW = 0x1200
  };
  extern uint32_t before_address;
  bool read_memory (uint32_t start_addr, size_t byte_count);
  bool write_memory (void);
} // end of NVM

namespace JTAG2 {
  /* JTAG2::CONTROL flags */
  enum jtag_control_e {
      HOST_SIGN_ON              = 0x01
    , USART_TX_EN               = 0x02
    , CHANGE_BAUD               = 0x04
    , ANS_FAILED                = 0x80
  };

  /* See documentation AVR067: JTAGICE mkII Communication Protocol Manual */

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
    , CMND_SET_DEVICE_DESC      = 0x0C
    , CMND_GET_SYNC             = 0x0F
    , CMND_ENTER_PROGMODE       = 0x14
    , CMND_LEAVE_PROGMODE       = 0x15
    , CMND_XMEGA_ERASE          = 0x34
    , CMND_SET_XMEGA_PARAMS     = 0x36 /* Undocumented, upper FW 7.xx */
    /*** ATMEL defines more than ***/
    , CMND_SET_UPDI_PARAMS      = 0x55
  };

  /* Slave Response IDs */
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
    /*** ATMEL defines more than ***/
  };

  /*** CMND_{READ,WRITE}_MEMORY sub-command ***/
  enum jtag_mem_type_e {
    /************* Absolute Address Type = A for all *****/
    /************* Relative Address Type = R for FWV=7 ***/
      MTYPE_SRAM                = 0x20  // A   SRAM and Extend IO registers
    , MTYPE_EEPROM              = 0x22  // A/R EEPROM in programming mode (mapping)
    , MTYPE_IO_SHADOW           = 0x30  // A   IO registers file (old mapping)
    , MTYPE_EVENT               = 0x60  // --- ICE event memory
    , MTYPE_SPM                 = 0xA0  // --- FLASH through LPM/SPM
    , MTYPE_FLASH_PAGE          = 0xB0  // A   FLASH in programming mode  (avsolute address)
    , MTYPE_EEPROM_PAGE         = 0xB1  // A   EEPROM in programming mode (avsolute address)
    , MTYPE_FUSE_BITS           = 0xB2  // A/R FUSE bits in programming mode
    , MTYPE_LOCK_BITS           = 0xB3  // A/R LOCK bits in programming mode
    , MTYPE_SIGN_JTAG           = 0xB4  // A   SIGNATURE in programming mode
    , MTYPE_OSCCAL_BYTE         = 0xB5  // --- For OSC calibration (Refer to AVR057)
    , MTYPE_CAN                 = 0xB6  // --- CAN mailbox storage
    /*** No clear documentation from here ***/
    , MTYPE_XMEGA_APP_FLASH     = 0xC0  // A/R XMEGA_APPLICATION_FLASH
    , MTYPE_XMEGA_BOOT_FLASH    = 0xC1  // A/R XMEGA_BOOT_FLASH
    , MTYPE_XMEGA_EEPROM        = 0xC4  // A/R XMEGA_EEPROM
    , MTYPE_XMEGA_USERSIG       = 0xC5  // A/R XMEGA_USER_SIGNATURE
    , MTYPE_XMEGA_PRODSIG       = 0xC6  // A/R XMEGA_CALIBRATION_SIGNATURE
    /*** ATMEL defines more than ***/
  };
  /* MTYPE_FLASH_PAGE is originally used in combination with MTYPE_SPM, */
  /* but when used alone, always absolute address is used. */

  /* CMND_XMEGA_ERASE sub-command */
  enum jtag_erase_mode_e {
      XMEGA_ERASE_CHIP          = 0x00
    /* The following is not implemented */
    /* In interactive mode, normal memory writing is substituted */
    , XMEGA_ERASE_APP           = 0x01
    , XMEGA_ERASE_BOOT          = 0x02
    , XMEGA_ERASE_EEPROM        = 0x03
    , XMEGA_ERASE_APP_PAGE      = 0x04
    , XMEGA_ERASE_BOOT_PAGE     = 0x05
    , XMEGA_ERASE_EEPROM_PAGE   = 0x06
    , XMEGA_ERASE_USERSIG       = 0x07
    , XMEGA_ERASE_USERROW       = 0x07
    /*** ATMEL defines more than ***/
  };

  /* Parameters IDs */
  enum jtag_parameter_e {
      PAR_HW_VER    = 0x01          // read[2]
    , PAR_FW_VER    = 0x02          // read[4]
    , PAR_EMU_MODE  = 0x03          // read[1]
    , PAR_BAUD_RATE = 0x05          // r/w[1]
    , PAR_VTARGET   = 0x06          // read[2]
    , PAR_TARGET_SIGNATURE = 0x1D   // read[32]
    , PAR_PDI_OFFSET_START = 0x32   // write[4]
    , PAR_PDI_OFFSET_END   = 0x33   // write[4]
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
    /*** ATMEL defines more than ***/
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

  /*** AVR_EB BOOTROW Support (FWV=6 and 7) ****************/
  /* MTYPE : MTYPE_FLASH_PAGE (always absolute address)    */
  /* ERASE : XMEGA_ERASE_APP_PAGE or XMEGA_ERASE_BOOT_PAGE */

  /*** Using set_device_descriptor (FWV=6 for Parallel-Interface, SPI, DGI) ***/
  struct jtag_device_descriptor {
    unsigned char ucReadIO[8];            /* LSB = IOloc 0, MSB = IOloc63 */
    unsigned char ucReadIOShadow[8];      /* LSB = IOloc 0, MSB = IOloc63 */
    unsigned char ucWriteIO[8];           /* LSB = IOloc 0, MSB = IOloc63 */
    unsigned char ucWriteIOShadow[8];     /* LSB = IOloc 0, MSB = IOloc63 */
    unsigned char ucReadExtIO[52];        /* LSB = IOloc 96, MSB = IOloc511 */
    unsigned char ucReadIOExtShadow[52];  /* LSB = IOloc 96, MSB = IOloc511 */
    unsigned char ucWriteExtIO[52];       /* LSB = IOloc 96, MSB = IOloc511 */
    unsigned char ucWriteIOExtShadow[52]; /* LSB = IOloc 96, MSB = IOloc511 */
    unsigned char ucIDRAddress;           /* IDR address */
    unsigned char ucSPMCRAddress;     /* SPMCR Register address and dW BasePC */
    unsigned char ucRAMPZAddress;     /* RAMPZ Register address in SRAM I/O */
    unsigned char uiFlashPageSize[2]; /* Device Flash Page Size */
    unsigned char ucEepromPageSize;   /* Device Eeprom Page Size in bytes */
    unsigned char ulBootAddress[4];   /* Device Boot Loader Start Address */
    unsigned char uiUpperExtIOLoc[2]; /* Topmost (last) extended I/O */
    unsigned char ulFlashSize[4];     /* Device Flash Size */
    unsigned char ucEepromInst[20];   /* Instructions for W/R EEPROM */
    unsigned char ucFlashInst[3];     /* Instructions for W/R FLASH */
    unsigned char ucSPHaddr;          /* stack pointer high */
    unsigned char ucSPLaddr;          /* stack pointer low */
    unsigned char uiFlashpages[2];    /* number of pages in flash */
    unsigned char ucDWDRAddress;      /* DWDR register address */
    unsigned char ucDWBasePC;         /* base/mask value of the PC */
    unsigned char ucAllowFullPageBitstream; /* FALSE on ALL new */
    unsigned char uiStartSmallestBootLoaderSection[2];
    unsigned char EnablePageProgramming;    /* For JTAG parts only, */
    unsigned char ucCacheType;        /* CacheType_Normal 0x00, */
    unsigned char uiSramStartAddr[2]; /* Start of SRAM */
    unsigned char ucResetType;        /* Selects reset type */
    unsigned char ucPCMaskExtended;   /* For parts with extended PC */
    unsigned char ucPCMaskHigh;       /* PC high mask */
    unsigned char ucEindAddress;      /* EIND address */
    unsigned char EECRAddress[2];     /* EECR memory-mapped IO address */
  };
  /* The only meaningful values for the UPDI generation are:
   *    uiFlashPageSize[2]  := 32,64,128,512
   *    ucEepromPageSize    := 1,8,32,64
   */

  /* Conversion device_descriptor -> updi_device_descriptor
   *
   *    uiFlashPageSize  -> flash_page_size  : Valid value 32,64,128 or 512
   *    ucIDRAddress     -> hvupdi_variant   : If valid, the upper nibble is 3 ('0')
   */

  struct updi_device_descriptor {
    uint8_t magicnumber;              // cannot guess; must be 0x55
    uint8_t length;                   // length of the following data = 8
    uint8_t hvupdi_variant;           // Valid value '0','1' or '2'
    uint8_t nvmctrl_version;          // Valid value '0','2','3' or '5'
    uint16_t flash_page_size;         // Valid value 32,64,128 or 512
    uint8_t eeprom_page_size;         // Vaild value 1,8,32,64
    uint8_t signature[3];             // Valid value $1E:$9x:$NN
    /*** Not yet finalized or used after this point ***/
    // uint16_t nvm_signature_offset;    // Valid value 0x1100 or 0x1080
    // uint16_t nvm_eeprom_offset;       // 0x1400 fixed
    // uint16_t nvm_userrow_offset;
    // uint16_t nvm_bootrow_offset;
    // uint16_t nvm_data_offset;         // SRAM start address
    // uint32_t nvm_code_offset;         // 0x4000,0x8000 or 0x800000
    /*** Below this is the local work area ***/
    uint8_t sib[32];
  } extern updi_desc;

  /* JTAG2 packet */
  enum jtag_packet_e {
      MESSAGE_START = 0x1B          /* SOH */
    , TOKEN         = 0x0E          /* STX */
    , MAX_BODY_SIZE = 10 + 512 + 10
    , BUFFER_CACHE  = 10 + 256 + 10
    , MESSAGE_ID    = 0
    , RSP_DATA      = 1
    , MEM_TYPE      = 1
    , DATA_LENGTH   = 2
    , DATA_ADDRESS  = 6
    , DATA_START    = 10
  };
  union jtag_packet_t {
    uint8_t _pad;                   // alignment padding
    uint8_t raw[MAX_BODY_SIZE + (1 + 2 + 4 + 1 + 2)];
    struct {
      uint8_t soh;                  // $00:1 $1B PREFIX
      union {                       // $01:2
        uint16_t number;
        uint8_t number_byte[2];
      };
      union {                       // $03:4=N
        uint32_t size;
        int16_t size_word[2];
        uint8_t size_byte[4];
      };
      uint8_t stx;                  // $07:1 $0E TOKEN
      uint8_t body[MAX_BODY_SIZE];  // $08:N
      uint8_t _crc[2];              // $08+N:2
    };
  } extern packet;

  /* pblic methods */
  void setup (void);
  void set_response (jtag_response_e response_code);
  void wakeup_jtag (void);
} // end of JTAG2

// end of header

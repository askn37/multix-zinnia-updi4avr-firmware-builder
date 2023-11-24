/**
 * @file Configuration.h
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.2
 * @date 2023-11-11
 *
 * @copyright Copyright (c) 2023 askn37 at github.com
 *
 */
#pragma once

/*********************
 * Build mode switch *
 *********************/

/* Locked devices return a tentative signature */
#define ENABLE_ADDFEATS_LOCK_SIG

/* Debug output of further determined internal structure */
// #define ENABLE_ADDFEATS_LOCK_SIG_DEBUGOUT

/* Enable SIB to be output as IO dump */
// #define ENABLE_ADDFEATS_DUMP_SIB

/* Includes UPDI communication log when writing NVM in response. */
// #define ENABLE_DEBUG_UPDI_SENDER

/********************
 * Speed definition *
 ********************/

/* CPU reference clock minimum 10MHz */
#if !defined(F_CPU) || (F_CPU < 10000000L)
  #warning F_CPU cannot be less than 10000000L
  #include BUILD_STOP
#endif

/* UPDI normal baudrate */
#define UPDI_BAUD (225000)

/* CLK_USART calculated（10MHz=178） */
#define UPDI_BAUD_CALC ((F_CPU / UPDI_BAUD * 8 + 1) / 2)

/* Single wire guard time value */
#define UPDI_GTVAL     UPDI::UPDI_SET_GTVAL_2
#define UPDI_GTVAL_RSD UPDI::UPDI_SET_GTVAL_2

/* LED Timer config */
#define HBEAT_HZ   (0.5)
#define TCA0_STEP  ((uint8_t)(sqrt((F_CPU / 1024.0) * (1.0 / HBEAT_HZ)) - 0.5))
#define TCB1_HBEAT (((TCA0_STEP / 2) * 256) + TCA0_STEP - 1)
#define TCB1_STEP  (170)
#define TCB1_BLINK (((TCB1_STEP / 2) * 256) + TCB1_STEP - 1)
#define TCB1_FLASH ((8 * 256) + TCB1_STEP - 1)
#define TCB1_FAST  ((4 * 256) + TCB1_STEP / 8)

/*******************
 * GPIO allocation *
 *******************/

/**********************
 * MZU2306B (ATtiny*26/*27) GPIO Layout
 *
 * PA0 -- UPDI
 * PA1 IO U0TDAT LUTnIN0_U0TXD (USART0_ALT)
 * PA2 I  RTS
 * PA3 O  HVP1 WO3
 * PA4 O  HVP2 WO4
 * PA5 O  LEDY LUT3OUT
 * PA6 O  HV12
 * PA7 O  LEDG EVOUTA
 *
 * PB0 I  SW1
 * PB1 IO TRST
 * PB2 -  U0TXD         (unused USART0_DEF)
 * PB3 -  U0RXD LUT2OUT (unused USART0_DEF)
 * PB4 -  U0RST         (force FUSE)
 * PB5 O  HV8V
 *
 * PC0 O  PGEN
 * PC1 I  HTCR U1JTRX LUT3IN1 (USART1_ALT)
 * PC2 O  HRCT U1JTTX LUT3IN2 (USART1_ALT)
 * PC3 I  HVJP
 */

/* Host UPDI : PIN_PA0 definition not required */

/* Host UART : half-duplex two-wire (no external pull-up) */
#define JTAG_TXD_PIN      PIN_PC2
#define JTAG_RXD_PIN      PIN_PC1
#define JTAG_PORT         PORTC
#define JTAG_TXD_CONFIG   ( PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc )
#define JTAG_RXD_CONFIG   ( PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc )
#define JTAG_PMUX_ALT     PORTMUX_USART1_ALT1_gc
#define JTAG_USART        USART1
#define JTAG_USART_CTRLA  ( 0 )
#define JTAG_USART_ON     ( USART_RXEN_bm | USART_ODME_bm | USART_TXEN_bm)
#define JTAG_USART_DBLON  ( USART_RXEN_bm | USART_ODME_bm | USART_TXEN_bm | USART_RXMODE_CLK2X_gc )
#define JTAG_USART_OFF    ( USART_RXEN_bm | USART_ODME_bm)
#define JTAG_USART_CTRLC  ( USART_CHSIZE_8BIT_gc \
                          | USART_PMODE_DISABLED_gc \
                          | USART_CMODE_ASYNCHRONOUS_gc \
                          | USART_SBMODE_1BIT_gc )

/* Target UPDI : Half-duplex single wire (external pull-up 100kohm) */
#define UPDI_TDAT_PIN     PIN_PA1
#define UPDI_PORT         PORTA
#define UPDI_TDAT_CONFIG  ( PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc )
#define UPDI_PMUX_ALT     PORTMUX_USART0_ALT1_gc
#define UPDI_USART        USART0
#define UPDI_USART_CTRLA  ( USART_LBME_bm | _BV(1) ) /* Bp1 : undocumented : loopback drive mode auto insert grace bit */
#define UPDI_USART_ON     ( USART_RXEN_bm \
                          | USART_TXEN_bm \
                          | USART_ODME_bm )
#define UPDI_USART_OFF    ( USART_ODME_bm )
#define UPDI_USART_CTRLC  ( USART_CHSIZE_8BIT_gc \
                          | USART_PMODE_EVEN_gc \
                          | USART_CMODE_ASYNCHRONOUS_gc \
                          | USART_SBMODE_2BIT_gc )

/* RTS monitoring INPUT (no external pull-up) */
#define RTS_SENSE_PIN     PIN_PA2
#define RTS_SENSE_PORT    PORTA
#define RTS_SENSE_CONFIG  ( PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc )
#define RTS_SENSE_DISABLE ( PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc )

/* HV generator (external pull-up 100k when unused input) */
/* PA[3,4] == TCA0_WOA[3,4] */
#define HVP1_PIN          PIN_PA3
#define HVP2_PIN          PIN_PA4
#define HVEN_PORT         PORTA
#define HVP1_PIN_CONFIG   ( PORT_ISC_INPUT_DISABLE_gc )
#define HVP2_PIN_CONFIG   ( PORT_ISC_INPUT_DISABLE_gc | PORT_INVEN_bm )
#define HVEN_CONFIG       ( TCA_SPLIT_HCMP1EN_bm | TCA_SPLIT_HCMP2EN_bm )

#define HV12_PIN          PIN_PA6
#define HV12_PORT         PORTA
#define HV12_PIN_CONFIG   ( PORT_ISC_INPUT_DISABLE_gc )
#define HV12_PIN_ON       HIGH
#define HV12_PIN_OFF      LOW

#define HV8_PIN           PIN_PB5
#define HV8_PORT          PORTB
#define HV8_PIN_CONFIG    ( PORT_ISC_INPUT_DISABLE_gc )
#define HV8_PIN_ON        HIGH
#define HV8_PIN_OFF       LOW

/* LED indicator OUTPUT */
/* PA7 == EVSYS_EVOUTA_ALT1 */
#define LEDG_PIN          PIN_PA7
#define LEDG_PORT         PORTA
#define LEDG_PIN_CONFIG   ( PORT_ISC_INPUT_DISABLE_gc )
#define LEDG_PIN_ON       HIGH
#define LEDG_PIN_OFF      LOW
#define LEDG_EVOUT        EVSYS_USEREVSYSEVOUTA
#define LEDG_EVOUT_MUX    PORTMUX_EVSYSROUTEA
#define LEDG_EVOUT_ALT    PORTMUX_EVOUTA_ALT1_gc

/* PA5 == CCL_LUT3_OUT */
#define LEDY_PIN          PIN_PA5
#define LEDY_PORT         PORTA
#define LEDY_PIN_CONFIG   ( PORT_ISC_INTDISABLE_gc )
#define LEDY_PIN_ON       HIGH
#define LEDY_PIN_OFF      LOW

/* Button monitoring INPUT (no external pull-up) */
#define SW_SENSE_PIN      PIN_PB0
#define SW_SENSE_PORT     PORTB
#define SW_SENSE_CONFIG   ( PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc )
#define SW_SENSE_PIN_ON   LOW
#define SW_SENSE_PIN_OFF  HIGH

/* JP1 monitoring INPUT (no external pull-up) */
#define JP_SENSE_PIN      PIN_PC3
#define JP_SENSE_PORT     PORTC
#define JP_SENSE_CONFIG   ( PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc )
#define JP_SENSE_PIN_ON   LOW
#define JP_SENSE_PIN_OFF  HIGH

/* UART mode（T-Gate）OUTPUT */
#define PGEN_PIN          PIN_PC0
#define PGEN_PORT         PORTC
#define PGEN_PIN_CONFIG   ( PORT_ISC_INTDISABLE_gc )
#define PGEN_PIN_ON       HIGH
#define PGEN_PIN_OFF      LOW

/* Target reset OUTPUT (external pull-up 100kohm) (open-drain) */
#define TRST_PIN          PIN_PB1
#define TRST_PORT         PORTB
#define TRST_PIN_CONFIG   ( PORT_PULLUPEN_bm | PORT_ISC_INPUT_DISABLE_gc )
#define TRST_ON           LOW
#define TRST_OFF          HIGH

// enf of header

/**
 * @file peripheral.h
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once
#include <api/HarfUART.h>
#include <api/TWIM.h>

#ifdef __cplusplus
extern "C" {
#endif

/* configuration to "api/Portmux.cpp" */

/* USARTn in HarfUART.h */
#define Serial0 Serial0A
extern HarfUART_Class Serial0A;   /* upload.port=UART0_B2 */
extern HarfUART_Class Serial0B;   /* upload.port=UART0_A1 */

#define Serial1 Serial1A
extern HarfUART_Class Serial1A;   /* upload.port=UART1_A1 */
extern HarfUART_Class Serial1B;   /* upload.port=UART1_C2 */

/* Serial2x not implemented */
/* Serial3x not implemented */

/* struct UART_portmux_t in Portmux.h */
#define _PORTMUX_USART0A {&PORTMUX_USARTROUTEA, PORTMUX_USART0_gm, PORTMUX_USART0_DEFAULT_gc, &PORTB, PIN2_bm, PIN3_bm, &PORTB_PIN3CTRL}
#define _PORTMUX_USART0B {&PORTMUX_USARTROUTEA, PORTMUX_USART0_gm, PORTMUX_USART0_ALT1_gc   , &PORTA, PIN1_bm, PIN2_bm, &PORTA_PIN2CTRL}
#define _PORTMUX_USART1A {&PORTMUX_USARTROUTEA, PORTMUX_USART1_gm, PORTMUX_USART1_DEFAULT_gc, &PORTA, PIN1_bm, PIN2_bm, &PORTA_PIN2CTRL}
#define _PORTMUX_USART1B {&PORTMUX_USARTROUTEA, PORTMUX_USART1_gm, PORTMUX_USART1_ALT1_gc   , &PORTC, PIN2_bm, PIN1_bm, &PORTC_PIN1CTRL}

/* TWIn in TWIM.h */
#define Wire0 Wire0A
extern TWIM_Class Wire0A;

/* struct TWI_portmux_t in Portmux.h */
#define _PORTMUX_TWI0A {nullptr, 0, 0, &PORTA, PIN2_bm, PIN3_bm, &PORTA_PIN2CTRL, &PORTA_PIN3CTRL}

#ifdef __cplusplus
} // extern "C"
#endif

// end of code

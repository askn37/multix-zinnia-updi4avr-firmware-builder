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

/* Serial1x not implemented */
/* Serial2x not implemented */
/* Serial3x not implemented */

/* struct UART_portmux_t in Portmux.h */
#define _PORTMUX_USART0A {&PORTMUX_CTRLB, 1, 0, &PORTB, PIN2_bm, PIN3_bm, &PORTB_PIN3CTRL}
#define _PORTMUX_USART0B {&PORTMUX_CTRLB, 1, 1, &PORTA, PIN1_bm, PIN2_bm, &PORTA_PIN2CTRL}

/* TWIn in TWIM.h */
#define Wire0 Wire0A
extern TWIM_Class Wire0A;
extern TWIM_Class Wire0B;

/* struct TWI_portmux_t in Portmux.h */
#define _PORTMUX_TWI0A {&PORTMUX_CTRLB, 16, 0 , &PORTB, PIN0_bm, PIN1_bm, &PORTB_PIN0CTRL, &PORTB_PIN1CTRL}
#define _PORTMUX_TWI0B {&PORTMUX_CTRLB, 16, 16, &PORTA, PIN2_bm, PIN1_bm, &PORTA_PIN2CTRL, &PORTA_PIN1CTRL}

#ifdef __cplusplus
} // extern "C"
#endif

// end of code

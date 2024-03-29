/**
 * @file TIM.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.2
 * @date 2023-11-12
 *
 * @copyright Copyright (c) 2023 askn37 at github.com
 *
 */
#include "Prototypes.h"
#include <math.h>
#include <setjmp.h>

/********************
 * [TIMer assignment]
 *   TCA0 -- Split operation          CLK_TCA0 := CLK_PER (Direct)
 *           WOA0: Divide by 255 ---> CLK_TCB1
 *           WOA3: PIN_PA3 for HV generation
 *           WOA4: PIN_PA4 for HV generation
 *   TCB0 -- For Timeout generation   CLK_TCB0 := PIT/128 (EVSYS_CH1)
 *           For Baudrate calibratoer CLK_TCB0 := F_CPU
 *   TCB1 -- LED Control              CLK_TCB1 := CLK_PER
 *
 * [LED control]
 *   Division ratio is based on F_CPU := 10MHz/20MHz
 *
 * [HeartBeat]
 *   CLK_TCA0 -> WOA0 -> CCL1_INSEL0
 *                             ExOR -> EVSYS_CH2 -> EVOUTA
 *   CLK_TCB0 -> WOB1 -> CCL1_INSEL1
 *
 * [Flash/Blink]
 *   PIT/128 -> EVSYS_CH3 -> TCB1COUNT -> CCL0 -> EVSYS_CH0 -> EVOUTA
 */

namespace TIM {
  jmp_buf CONTEXT;
  uint8_t mode = 0;
}

void TIM::setup (void) {

  /* EVSYS signal distribution */

  EVSYS_CHANNEL0 = EVSYS_CHANNEL0_CCL_LUT0_gc;
  EVSYS_CHANNEL1 = EVSYS_CHANNEL1_RTC_PIT_DIV128_gc;
  EVSYS_CHANNEL2 = EVSYS_CHANNEL2_CCL_LUT1_gc;
  EVSYS_CHANNEL3 = EVSYS_CHANNEL3_PORTA_PIN5_gc;  /* <- PA5:LEDY */
  EVSYS_USERTCB0COUNT = EVSYS_USER_CHANNEL1_gc;
  EVSYS_USERTCB1COUNT = EVSYS_USER_CHANNEL1_gc;
  EVSYS_USERCCLLUT0A  = EVSYS_USER_CHANNEL3_gc;   /* <- PA5:LEDY */
  EVSYS_USERCCLLUT1A  = EVSYS_USER_CHANNEL3_gc;   /* <- PA5:LEDY */

  /* When PA5:LEDY of CCL3 is used, the signal output of CCL0 and CCL1 is stopped. */

  /* CCL/LUT construction */

  /* TRUTH0: 010 is ON */
  CCL_TRUTH0    = CCL_TRUTH_2_bm;
  CCL_LUT0CTRLC = CCL_INSEL2_EVENTA_gc;           /* <- IN2:PA5 */
  CCL_LUT0CTRLB = CCL_INSEL1_TCB1_gc;             /* <- IN1:POS */
  CCL_LUT0CTRLA = CCL_ENABLE_bm;

  /* TRUTH1: 001 010 is ON */
  CCL_TRUTH1    = CCL_TRUTH_1_bm | CCL_TRUTH_2_bm;
  CCL_LUT1CTRLC = CCL_INSEL2_EVENTA_gc;           /* <- IN2:PA5 */
  CCL_LUT1CTRLB = CCL_INSEL0_TCA0_gc | CCL_INSEL1_TCB1_gc;  /* <- IN0:POS IN1:POS */
  CCL_LUT1CTRLA = CCL_ENABLE_bm;

  /* TRUTH3: 001 010 is ON */
  /* CCL_INSEL0_USART0_gc : USART0TX normal HIGH */
  /* CCL_INSEL1_IO_gc     : USART1RX normal HIGH */
  /* CCL_INSEL2_IO_gc     : USART1TX normal HIGH */
  CCL_TRUTH3    = (uint8_t) ~CCL_TRUTH_7_bm;
  CCL_LUT3CTRLC = CCL_INSEL2_IO_gc;
  CCL_LUT3CTRLB = CCL_INSEL0_USART0_gc | CCL_INSEL1_IO_gc;
  CCL_LUT3CTRLA = CCL_ENABLE_bm | CCL_OUTEN_bm;

  /* CCL enable */
  CCL_CTRLA = CCL_RUNSTDBY_bm | CCL_ENABLE_bm;

  /* RTC_PIT enable */
  RTC_PITCTRLA = RTC_PITEN_bm;

  /* Timer */

  /* TCA0 */
  TCA0_SPLIT_CTRLD = TCA_SPLIT_SPLITM_bm;
  TCA0_SPLIT_LPER  = TCA0_STEP - 2;
  TCA0_SPLIT_LCMP0 = TCA0_STEP / 2;
  TCA0_SPLIT_HPER  = 1;
  TCA0_SPLIT_HCMP0 = 1;     /* WOA3=PA3 */
  TCA0_SPLIT_HCMP1 = 1;     /* WOA4=PA4 */
  TCA0_SPLIT_CTRLA = TCA_SPLIT_RUNSTDBY_bm | TCA_SPLIT_ENABLE_bm | TCA_SPLIT_CLKSEL_DIV1024_gc;

  /* TCB1 */
  TCB1_CTRLB = TCB_CNTMODE_PWM8_gc;
}

/*
 * Timeout control
 */

void TIM::Timeout_Start (uint16_t _ms) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    TCB0_CNT = 0;
    TCB0_CCMP = _ms >> 2;
    TCB0_INTCTRL = TCB_CAPT_bm;
    TCB0_INTFLAGS = TCB_CAPT_bm;
    TCB0_CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_EVENT_gc;
  }
}

void TIM::Timeout_Stop (void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    TCB0_CTRLA = 0;
    TCB0_INTFLAGS = TCB_CAPT_bm;
  }
  reti();
}

/*
 * LED operation switching
 */

/* Heartbeat (waiting) */
void TIM::LED_HeartBeat (void) {
  if (TIM::mode != 1) {
    TIM::mode = 1;
    TCB1_CCMP = TCB1_HBEAT;
    TCB1_CNT = 0;
    TCB1_CTRLA = TCB_RUNSTDBY_bm | TCB_ENABLE_bm | TCB_CLKSEL_TCA0_gc;
    LEDG_EVOUT = EVSYS_USER_CHANNEL2_gc;
  }
}

void LED_TCB1 (uint8_t mode, uint16_t ccmp) {
  if (TIM::mode != mode) {
    TIM::mode = mode;
    TCB1_CCMP = ccmp;
    TCB1_CNT = 0;
    TCB1_CTRLA = TCB_RUNSTDBY_bm | TCB_ENABLE_bm | TCB_CLKSEL_EVENT_gc;
    LEDG_EVOUT = EVSYS_USER_CHANNEL0_gc;
  }
}

/* Flash (after RTS assert/UPDI authorization) */
void TIM::LED_Flash (void) {
  LED_TCB1(2, TCB1_FLASH);
}

/* Flashing (SW1 assert) */
void TIM::LED_Blink (void) {
  LED_TCB1(3, TCB1_BLINK);
}

/* Fast blinking (UPDI memory access in progress) */
void TIM::LED_Fast (void) {
  LED_TCB1(4, TCB1_FAST);
}

/* Suspension (before UPDI approval) */
/* TCA0 changed to charge pump drive speed */
void TIM::LED_Stop (void) {
  if (TIM::mode != 0) {
    TIM::mode = 0;
    TCA0_SPLIT_CTRLA = TCA_SPLIT_RUNSTDBY_bm | TCA_SPLIT_ENABLE_bm | TCA_SPLIT_CLKSEL_DIV1_gc;
    LEDG_EVOUT = EVSYS_USER_OFF_gc;
    digitalWrite(LEDG_PIN, LOW);
  }
}

/*
 * HV charge pump drive control
 */

void TIM::HV_Pulse_ON (void) {
  TCA0_SPLIT_CTRLB = TCA_SPLIT_HCMP0EN_bm | TCA_SPLIT_HCMP1EN_bm;
}

void TIM::HV_Pulse_OFF (void) {
  TCA0_SPLIT_CTRLB = 0;
}

/*
 * Constant delay
 */

void TIM::delay_50us (void) {
  delay_micros(50);
}

void TIM::delay_800us (void) {
  delay_micros(800);
}

void TIM::delay_200ms (void) {
  delay_millis(200);
}

/*
 * SW1 monitoring LEVEL interrupt
 */

ISR(portIntrruptVector(SW_SENSE_PIN), ISR_NAKED) {
  /***
    This interrupt keeps the target device in reset as long as
    the low level signal continues. Since the interrupt exits
    with a system reset, it does not return to main operation.
  ***/

  /***
    The LED flashes rapidly while the UPDI data line is LOW.
    If there is no response for 1 second,
    the process will be aborted and the system will restart.
  ***/
  SYS::WDT_SET(WDT_PERIOD_1KCLK_gc);
  TIM::LED_Fast();
  while (!digitalRead(UPDI_TDAT_PIN));

  /***
    If UPDI can be communicated, attempt to reset the target.
  ***/
  SYS::WDT_Short();
  UPDI::Target_Reset(true);

  /* Attempt to reset the target hardware. */
  UPDI_USART.CTRLB = UPDI_USART_OFF;
  delay_micros(800);
  pinMode(UPDI_TDAT_PIN, OUTPUT);
  digitalWrite(UPDI_TDAT_PIN, LOW);
  openDrainWrite(TRST_PIN, LOW);

  /* The LED will blink and wait while the button is pressed. */
  TIM::LED_Blink();
  SYS::WDT_OFF();
  while (!digitalRead(SW_SENSE_PIN));

  /* The system will be restarted. */
  SYS::WDT_REBOOT();
}

/*
 * RTS monitoring upper and lower end interrupts
 */

ISR(portIntrruptVector(RTS_SENSE_PIN)) {
  /***
    When this interrupt occurs, the WDT is reconfigured and the target
    device is held in reset. If JTAG communication is not started within
    the timeout limit, the system will be reset. Typically this is the
    signal that starts the Arduino bootloader. Therefore, the time limit
    is set to approximately 250ms.
  ***/

  SYS::WDT_Short();
  SYS::RTS_Disable();
  portRegister(RTS_SENSE_PIN).INTFLAGS =
  portRegister(RTS_SENSE_PIN).INTFLAGS;
  SYS::PG_Enable();
  TIM::LED_Flash();
  UPDI::Target_Reset(true);
  openDrainWrite(TRST_PIN, LOW);
}

/*
 * Timeout handler
 */

ISR(TCB0_INT_vect, ISR_NAKED) {
  /***
    This interrupt is a global escape due to timeout.
    There is no return to the source of the interrupt.
  ***/
  __asm__ __volatile__ ("EOR R1,R1");
  TCB0_CTRLA = 0;
  TCB0_INTFLAGS = TCB_CAPT_bm;
  longjmp(TIM::CONTEXT, 2);
}

// end of code

/***
RTS pin change summary

* Arduino IDE console

    HIGH --                      -----
           |                    |
    LOW     --------------------
          OPEN                CLOSE

* Arduino IDE console for macos 1st opening

    HIGH ----------------------------

    LOW
          OPEN                CLOSE

* AVRDUDE arduino bootloader

    HIGH --   ------       ----------------
           | |      |     |     ^
    LOW     -        -----      start
          OPEN 250ms 100us 100ms      CLOSE

* AVRDUDE -xrtsdtr=high

    HIGH --   -----------------   -----
           | | ^               | |
    LOW     -  start            -
          OPEN                CLOSE

* AVRDUDE -xrtsdtr=low

    HIGH --                      ------
           |   start            |
    LOW     --------------------
          OPEN                CLOSE

- Do nothing if RTS=HIGH when the power is turned on.
- RTS=LOW edge deactivates the device and puts the UART into JTAG mode.
- If JTAG starts within 250ms, keep it there.
- If JTAG is not recognized within 250ms, put the UART on the device side and activate the device.

*/

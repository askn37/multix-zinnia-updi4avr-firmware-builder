/**
 * @file main.cpp
 * @author askn (K.Sato) multix.jp
 * @brief SUM_FW2401A
 * @version 0.1
 * @date 2024-04-14
 *
 * @copyright Copyright (c) 2024 askn37 at github.com
 *
 */

/**********
 * This firmware is written for ATtiny1616.
 * The generated binary code will be less than 1KiB.
 * You can use it as is with t416/t816.
 * But other than that it doesn't work out of the box.
 */

#include <avr/io.h>
#include <api/macro_api.h>
#include <api/power.h>
#include <peripheral.h>

/* Operating frequency must be 10MHz or less for 3.3V operation. */
#ifdef F_CPU
#undef F_CPU
#endif
#define F_CPU 10000000L

#define LED_DELAY 400       /* MAX=65535 */
#define UPDI_BAUD (225000)  /* MAX=225000 */
#define UPDI_BREAK (4500)   /* this must be slow enough */
#define UPDI_USART_ON     ( USART_ODME_bm | USART_TXEN_bm )
#define LED_ON ( PORT_ISC_INPUT_DISABLE_gc | PORT_INVEN_bm )
#define PORTMUX_CTRLA_DEF ( PORTMUX_LUT0_ALTERNATE_gc \
                          | PORTMUX_LUT1_DEFAULT_gc \
                          | PORTMUX_EVOUT1_bm )
#define PORTMUX_CTRLA_ALT ( PORTMUX_LUT0_ALTERNATE_gc \
                          | PORTMUX_LUT1_ALTERNATE_gc \
                          | PORTMUX_EVOUT1_bm )

/**********
 * setup main clock and port configuration
 */
void setup_system (void) {
  /* CLK_PER = 10MHz (3.3V upper limit speed) */
  _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm);

  /* port pin power reduction */
  uint8_t pin_pup = PORT_ISC_INTDISABLE_gc | PORT_PULLUPEN_bm;
  uint8_t pin_int = PORT_ISC_BOTHEDGES_gc  | PORT_PULLUPEN_bm;
  uint8_t pin_fal = PORT_ISC_FALLING_gc    | PORT_PULLUPEN_bm;
  uint8_t pin_lvl = PORT_ISC_LEVEL_gc      | PORT_PULLUPEN_bm;
  uint8_t pin_dig = PORT_ISC_INPUT_DISABLE_gc;
  // uint8_t pin_res = PORT_ISC_INTDISABLE_gc; // reset default

  /* Cannot proceed if external reset is required. */
  /* This keeps all pins Hi-Z except reset.        */
  pinControlRegister(PIN_PB4) = pin_fal;  // I:DBG_RESET
  // while (!digitalRead(PIN_PB4));

  /* Please look at the circuit diagram carefully.      */
  /* Some signals are connected to multiple (DUP) pins. */
  /* TxD and RxD are signal names on the CH340X side,   */
  /* so they may seem reversed.                         */

  // pinControlRegister(PIN_PA0) = pin_dig; // -:notused
  pinControlRegister(PIN_PA1) = pin_pup;    // IO:TDAT:USART0_TXD (DUP)
  pinControlRegister(PIN_PA2) = pin_pup;    // -:notused(DBG_HTCR)
  pinControlRegister(PIN_PA3) = LED_ON;     // O:LED:TCB1_WO
  pinControlRegister(PIN_PA4) = pin_lvl;    // I:SW1:PORTA_INT
  pinControlRegister(PIN_PA5) = pin_pup;    // IO:TRST:opendrain
  pinControlRegister(PIN_PA6) = pin_pup;    // I:HRCT:sense
  pinControlRegister(PIN_PA7) = pin_dig;    // O:HTCR:LUT1_OUT_0
  pinControlRegister(PIN_PB0) = pin_int;    // I:DTR:PORTB_INT
  pinControlRegister(PIN_PB1) = pin_pup;    // I:SW2-2:sense
  pinControlRegister(PIN_PB2) = pin_dig;    // O:RxD:EVOUT1
  pinControlRegister(PIN_PB3) = pin_dig;    // -:TxD:notused (DUP)
  // pinControlRegister(PIN_PB4) = pin_fal; // I:DBG_RESET
  pinControlRegister(PIN_PB5) = pin_pup;    // I:SW2-3:sense
  pinControlRegister(PIN_PC0) = pin_fal;    // I:RTS:PORTC_INT
  pinControlRegister(PIN_PC1) = pin_pup;    // O:TDAT:LUT1_OUT_0 (DUP)
  // pinControlRegister(PIN_PC2) = pin_res; // I:TDAT:async_in (DUP)
  pinControlRegister(PIN_PC3) = pin_pup;    // I:TxD:LUT1_IN0:sense (DUP)

  /* output only pin */
  openDrainWriteMacro(PIN_PA3, LOW);
  openDrainWriteMacro(PIN_PA7, LOW);
  openDrainWriteMacro(PIN_PB2, LOW);
}

/**********
 * setup peripheral functions
 */
void setup_peripheral (void) {
  /**********
   * PORTMUX
   */
  PORTMUX_CTRLA = PORTMUX_CTRLA_DEF;            // LUT1_OUT -> PA7
  PORTMUX_CTRLB = PORTMUX_USART0_ALTERNATE_gc;  // USART0_TxD -> PA1

  /**********
   * EVSYS
   * 
   * Definition names are very difficult to understand in tinyAVR-1.
   *
   * EVSYS_ASYNCCH0 <- PA6:HRCT --- RxD pass through
   * EVSYS_ASYNCCH1 <- CCL_LUT0 --- activity output
   * EVSYS_ASYNCCH2 <- PC2:TDAT --- RxD pass through
   * EVSYS_ASYNCCH3 <- CCL_LUT1 --- activity input from TxD
   *
   * EVSYS_ASYNCUSER2:CCL_LUT0_EV0 <- EVSYS_ASYNCCH3(TxD)
   * EVSYS_ASYNCUSER3:CCL_LUT0_EV1 <- EVSYS_ASYNCCH0(HRCT)
   * EVSYS_ASYNCUSER9:EVOUT1(RxD) <- EVSYS_ASYNCCH0 or EVSYS_ASYNCCH2
   * EVSYS_ASYNCUSER11:TCB1_COUNT <- EVSYS_ASYNCCH1(CCL_LUT0)
   */
  EVSYS_ASYNCCH0 = EVSYS_ASYNCCH0_PORTA_PIN6_gc;
  EVSYS_ASYNCCH1 = EVSYS_ASYNCCH1_CCL_LUT0_gc;
  EVSYS_ASYNCCH2 = EVSYS_ASYNCCH2_PORTC_PIN2_gc;
  EVSYS_ASYNCCH3 = EVSYS_ASYNCCH3_CCL_LUT1_gc;

  EVSYS_ASYNCUSER2  = EVSYS_ASYNCUSER2_ASYNCCH3_gc;
  EVSYS_ASYNCUSER3  = EVSYS_ASYNCUSER3_ASYNCCH0_gc;
  EVSYS_ASYNCUSER11 = EVSYS_ASYNCUSER11_ASYNCCH1_gc;

  /**********
   * USART0 - updi reset sender for UART mode
   */
  USART0_BAUD  = ((F_CPU / UPDI_BAUD * 8 + 1) / 2);
  USART0_CTRLA = ( USART_LBME_bm | USART_RS485_INT_gc );
  USART0_CTRLC = ( USART_CHSIZE_8BIT_gc \
                 | USART_PMODE_EVEN_gc \
                 | USART_CMODE_ASYNCHRONOUS_gc \
                 | USART_SBMODE_2BIT_gc );

  /**********
   * TCA - generate TCB clock
   *
   * CLK_TCA = 9765.625Hz(10MHz/1024) -> CLK_TCB1
   * 
   * Here we will generate the operating clock for TCB1.
   */
  TCA0_SINGLE_CTRLA = TCA_SINGLE_ENABLE_bm
                    | TCA_SINGLE_CLKSEL_DIV1024_gc;

  /**********
   * TCB - LED off time delay
   *
   * TCB1_MODE <- SINGLE
   * TCB1_ASYNC <- Enable
   * TCB1_CCMPEN <- Enable
   * TCB1_EDGE <- Falling
   * TCB1_CAPTEI <- Enable
   * TCB1_COUNT <- Event from CCL0
   * TCB1_CCMP <- Number
   * 
   * Controls the LED off time. The LED is on by default,
   * but requires a delay before it turns off.
   * This is achieved by using TCB1's single-shot mode
   * and thinning out successive event occurrences.
   * 
   * The obtained results must be inverted using the port peripheral.
   */
  TCB1_CCMP   = LED_DELAY;
  TCB1_EVCTRL = TCB_CAPTEI_bm | TCB_EDGE_bm;
  TCB1_CTRLB  = TCB_CNTMODE_SINGLE_gc | TCB_CCMPEN_bm | TCB_ASYNC_bm;
  TCB1_CTRLA  = TCB_ENABLE_bm | TCB_CLKSEL_TCA0_gc | TCB_RUNSTDBY_bm;

  /**********
   * CCL0 - Activity logic synthesis
   *
   * CCL0_IN0 <- EV0
   * CCL0_IN1 <- EV1
   * CCL0_TRUTH <- EV0:HIGH OR EV1:HIGH
   * CCL0_OUT -> None
   * 
   * TRUTH
   * EV0 | EV1 | OUT
   *   L |   L |   L
   *   H |   X |   H
   *   X |   H |   H
   * 
   * Logic synthesis results are sent to TCB1
   * via EVSYS without using the output port.
   */
  CCL_TRUTH0    = 0b11101110;
  CCL_LUT0CTRLB = CCL_INSEL0_EVENT0_gc | CCL_INSEL1_EVENT1_gc;
  CCL_LUT0CTRLA = CCL_ENABLE_bm;

  /**********
   * CCL1 - TxD pass through
   *
   * CCL1_IN0 <- PC3
   * CCL1_TRUTH <- IN0:HIGH
   * CCL0_OUT -> PA7:HTCR or PC1:TDAT(ALT)
   * 
   * Logic synthesis results are output to a dedicated external port.
   * Two ports can be selected with PORTMUX.
   */
  CCL_TRUTH1    = 0b00000010;
  CCL_LUT1CTRLB = CCL_INSEL0_IO_gc;
  CCL_LUT1CTRLA = CCL_OUTEN_bm | CCL_ENABLE_bm;

  /* CCL enable */
  CCL_CTRLA = CCL_ENABLE_bm | CCL_RUNSTDBY_bm;
}

/**********
 * The operating mode is determined by the DTR and DIP switch combination.
 */
bool is_pass_through_mode_of_uart (void) {
  /* SW2-2=HIGH is fixed mode */
  if (digitalReadMacro(PIN_PB1)) {
    /* SW2-3=HIGH is UART mode */
    return digitalReadMacro(PIN_PB5);
  }
  /* auto change mode */
  else {
    /* DTR=HIGH is UPDI mode */
    return !digitalReadMacro(PIN_PB0);
  }
}

/*********
 * Configure the multiplexer/demultiplexer depending on the operating mode
 */
void switch_pass_through_mode (void) {
  if (is_pass_through_mode_of_uart()) {
    /* PC3:TxD -> PA7:HTCR */
    /* PA6:HRCT -> PB2:RxD */
    PORTMUX_CTRLA = PORTMUX_CTRLA_DEF;
    openDrainWriteMacro(PIN_PC1, HIGH); // TDAT opendrain input
    EVSYS_ASYNCUSER9 = EVSYS_ASYNCUSER9_ASYNCCH0_gc;
  }
  else {
    /* PC3:TxD -> PC1:TDAT */
    /* PC2:TDAT -> PB2:RxD */
    PORTMUX_CTRLA = PORTMUX_CTRLA_ALT;
    openDrainWriteMacro(PIN_PC1, LOW);  // TDAT opendrain output
    EVSYS_ASYNCUSER9 = EVSYS_ASYNCUSER9_ASYNCCH2_gc;
  }
}

/* UPDI reception is not used */
// uint8_t updi_recv (void) {
//   loop_until_bit_is_set(USART0_STATUS, USART_RXCIF_bp);
//   GPIO_GPIOR1 = USART0_RXDATAH;
//   return GPIO_GPIOR0 = USART0_RXDATAL;
// }

/**********
 * UPDI communication is implemented as a single wire UART
 */
void updi_send (uint8_t _data) {
  loop_until_bit_is_set(USART0_STATUS, USART_DREIF_bp);
  USART0_STATUS  = USART_TXCIF_bm;
  USART0_TXDATAL = _data;
}

/**********
 * Reset the target AVR via UPDI communication
 */
void send_reset_updi (void) {
  openDrainWriteMacro(PIN_PA5, LOW);  // TRST negate (LOW)

  EVSYS_ASYNCUSER9 = EVSYS_ASYNCUSER9_OFF_gc;
  PORTMUX_CTRLA = PORTMUX_CTRLA_DEF;  // LUT1_OUT -> PA7

  /* Three GPIOs are connected to the TDAT pin. */
  /* You must ensure that all items are unused. */
  openDrainWriteMacro(PIN_PC1, HIGH); // TDAT opendrain input
  openDrainWriteMacro(PIN_PC2, HIGH); // TDAT opendrain input
  openDrainWriteMacro(PIN_PA1, HIGH); // TDAT opendrain input
  /* These macros expand into a single IO operation instruction */

  /* CCL disable */
  CCL_CTRLA = 0;

  /* enter UPDI setup */
  USART0_CTRLB = UPDI_USART_ON;       // USART0 enable
  USART0_BAUD = ((F_CPU / UPDI_BREAK * 8 + 1) / 2); // slow rate
  updi_send(0x00);      // UPDI_BREAK
  loop_until_bit_is_set(USART0_STATUS, USART_TXCIF_bp); // send wait
  USART0_BAUD = ((F_CPU / UPDI_BAUD * 8 + 1) / 2);  // normal rate
  while (!digitalReadMacro(PIN_PC2)); // target setup wait

  /* UPDI reset command */
  updi_send(0x55);      // UPDI_SYNCH
  updi_send(0xC8);      // UPDI_STCS | UPDI_CS_ASI_RESET_REQ
  updi_send(0x59);      // UPDI_RSTREQ

  /* While SW1 is pressed, blink the LED and wait */
  uint16_t cnt = 0;
  while (!digitalRead(PIN_PA4)) {
    if (cnt++ == 0) pinControlRegister(PIN_PA3) ^= PORT_INVEN_bm;
  }
  pinControlRegister(PIN_PA3) = LED_ON;

  /* UPDI not-reset command */
  updi_send(0x55);      // UPDI_SYNCH
  updi_send(0xC8);      // UPDI_STCS | UPDI_CS_ASI_RESET_REQ
  updi_send(0x00);      // UPDI_NOP

  /* UPDI exit command */
  updi_send(0x55);      // UPDI_SYNCH
  updi_send(0xC3);      // UPDI_STCS | UPDI_CS_CTRLB
  updi_send(0x04);      // UPDI_SET_UPDIDIS

  /* USART disable */
  loop_until_bit_is_set(USART0_STATUS, USART_TXCIF_bp); // send wait
  USART0_CTRLB = 0;
  CCL_CTRLA = CCL_ENABLE_bm | CCL_RUNSTDBY_bm;  // CCL restart
  openDrainWriteMacro(PIN_PA5, HIGH); // TRST assert (pull-up)

  /* Restore operating mode */
  switch_pass_through_mode();
}

/* PORTA_INT : sense SW1 / LEVEL */
/* SW1 controls the reset of the target MCU. */
/* It will reset itself after the operation. */
ISR(portIntrruptVector(PIN_PA4), ISR_NAKED) {
  send_reset_updi();
  _PROTECTED_WRITE(RSTCTRL_SWRR, RSTCTRL_SWRE_bm);  // system reset
}

/* The order in which the following events occur together is unknown. */

/* PORTB_INT : sense DTR / BOTHEDGE */
/* The DTR signal controls switching between operating modes. */
ISR(portIntrruptVector(PIN_PB0)) {
  PORTB_INTFLAGS = PIN0_bm;
  // /* Reboot if external reset is requested */
  // if (!digitalRead(PIN_PB4)) _PROTECTED_WRITE(RSTCTRL_SWRR, RSTCTRL_SWRE_bm);
  /* Otherwise switch the operating mode */
  switch_pass_through_mode();
}

/* PORTC_INT : sense RTS / FALLING */
/* RTS signals are handled differently depending on the operating mode. */
ISR(portIntrruptVector(PIN_PC0)) {
  PORTC_INTFLAGS = PIN0_bm;
  bool sw2_2 = digitalReadMacro(PIN_PB1); // ON is false
  bool sw2_3 = digitalReadMacro(PIN_PB5); // ON is false
  /* UART fixed mode only */
  if (sw2_2 && sw2_3) {         // DIP 2-OFF and 3-OFF
    send_reset_updi();          // This process takes time.
    PORTB_INTFLAGS = PIN0_bm;   // Therefore, DTR events are ignored.
  }
  /* In other than UPDI fixed mode, operate the TRST line. */
  else if (!sw2_2 || sw2_3) {   // DIP 2-ON or 3-OFF 
    openDrainWriteMacro(PIN_PA5, LOW);  // TRST negate (LOW)
    nop();                              // GPIO change wait
    openDrainWriteMacro(PIN_PA5, HIGH); // TRST assert (pull-up)
  }
}

/* main function */
int main (void) {
  setup_system();
  setup_peripheral();
  switch_pass_through_mode();
  sei();
  for (;;) power_standby();
  /* There is nothing for the MCU to do while waiting for operation */
}

// end of code

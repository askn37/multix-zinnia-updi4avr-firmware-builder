/**
 * @file main.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "Prototypes.h"

int main (void) {
  cli();
  initVariant();    /* Macro_API     */
  SYS::setup();     /* SYStem module */
  TIM::setup();     /* TIMer module  */
  JTAG2::setup();   /* JTAG2 module  */
  UPDI::setup();    /* UPDI module   */
  SYS::ready();
  JTAG2::wakeup_jtag();
}

// end of code

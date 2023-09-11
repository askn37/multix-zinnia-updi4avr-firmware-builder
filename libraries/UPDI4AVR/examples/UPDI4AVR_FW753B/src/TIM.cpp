/**
 * @file TIM.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "Prototypes.h"
#include <math.h>
#include <setjmp.h>

/*
 * [TIMer割付]
 *  TCA0 -- 分割動作 CLK_TCA0 == CLK_PER (Direct)
 *    WOA0 : 255分周 -> CLK_TCB1
 *    WOA3 : HV発生用 PIN_PA3
 *    WOA4 : HV発生用 PIN_PA4
 *  TCB0 -- タイムアウト発生用 CLK_TCB0 == PIT/128 (EVSYS_CH1)
 *  TCB1 -- LED制御 CLK_TCB1 == CLK_PER
 *
 * [LED制御]
 * 分周比は F_CPU=20MHz 基準
 *
 * [HeartBeat]
 *   CLK_TCA0 -> WOA0 -> CCL1_INSEL0
 *                           ExOR    -> EVSYS_CH2 -> EVOUTA
 *   CLK_TCAB -> WOB1 -> CCL1_INSEL1
 *
 * [Flash/Blink]
 *   PIT/128 -> EVSYS_CH3 -> TCB1COUNT -> CCL0 -> EVSYS_CH0 -> EVOUTA
 */

namespace TIM {
  jmp_buf CONTEXT;
  uint8_t mode = 0;
}

void TIM::setup (void) {

  /*
   * EVSYS信号分配
   */

  EVSYS_CHANNEL0 = EVSYS_CHANNEL0_CCL_LUT0_gc;
  EVSYS_CHANNEL1 = EVSYS_CHANNEL1_RTC_PIT_DIV128_gc;
  EVSYS_CHANNEL2 = EVSYS_CHANNEL2_CCL_LUT1_gc;
  EVSYS_CHANNEL3 = EVSYS_CHANNEL3_PORTA_PIN5_gc;
  EVSYS_USERTCB0COUNT = EVSYS_USER_CHANNEL1_gc;
  EVSYS_USERTCB1COUNT = EVSYS_USER_CHANNEL1_gc;
  EVSYS_USERCCLLUT0A  = EVSYS_USER_CHANNEL3_gc;
  EVSYS_USERCCLLUT1A  = EVSYS_USER_CHANNEL3_gc;

  /*
   * CCL/LUT構築
   */

  /* TRUTH0: 010 is ON */
  CCL_TRUTH0    = CCL_TRUTH_2_bm;
  CCL_LUT0CTRLC = CCL_INSEL0_EVENTA_gc;
  CCL_LUT0CTRLB = CCL_INSEL1_TCB1_gc;                       /* IN1:POS */
  CCL_LUT0CTRLA = CCL_ENABLE_bm;

  /* TRUTH1: 001 010 is ON */
  CCL_TRUTH1    = CCL_TRUTH_1_bm | CCL_TRUTH_2_bm;
  CCL_LUT1CTRLC = CCL_INSEL0_EVENTA_gc;
  CCL_LUT1CTRLB = CCL_INSEL0_TCA0_gc | CCL_INSEL1_TCB1_gc;  /* IN0:POS IN1:POS */
  CCL_LUT1CTRLA = CCL_ENABLE_bm;

  /* TRUTH3: 001 010 is ON */
  /* CCL_INSEL0_USART0_gc : USART0TX normal HIGH */
  /* CCL_INSEL1_IO_gc     : USART1RX normal HIGH */
  /* CCL_INSEL2_IO_gc     : USART1TX normal HIGH */
  CCL_TRUTH3    = (uint8_t) ~CCL_TRUTH_7_bm;
  CCL_LUT3CTRLC = CCL_INSEL2_IO_gc;
  CCL_LUT3CTRLB = CCL_INSEL0_USART0_gc | CCL_INSEL1_IO_gc;
  CCL_LUT3CTRLA = CCL_ENABLE_bm | CCL_OUTEN_bm;

  /* CCL 有効化 */
  CCL_CTRLA = CCL_RUNSTDBY_bm | CCL_ENABLE_bm;

  /* RTC_PIT 有効化 */
  RTC_PITCTRLA = RTC_PITEN_bm;

  /*
   * タイマー
   */

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

  /*
   * 起動時LEDモード決定
   *
   * RTS Deactive なら ハートビート
   * RTS Active なら フラッシュ
   */

  if ( digitalRead(RTS_SENSE_PIN) ) {
    LED_HeartBeat();
  }
  else {
    LED_Flash();
  }

  /* 休止モード（未使用）*/
  // set_sleep_mode(SLEEP_MODE_STANDBY);
  // sleep_enable();
}

/*
 * タイムアウト制御
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
 * LED動作切替
 */

/* ハートビート（待機中）*/
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

/* フラッシュ（RTSアサート/UPDI許認可後）*/
void TIM::LED_Flash (void) {
  LED_TCB1(2, TCB1_FLASH);
}

/* 点滅（SW1アサート）*/
void TIM::LED_Blink (void) {
  LED_TCB1(3, TCB1_BLINK);
}

/* 高速点滅（UPDIメモリアクセス中）*/
void TIM::LED_Fast (void) {
  LED_TCB1(4, TCB1_FAST);
}

/* 停止（UPDI許認可前）*/
/* TCA0はチャージポンプ駆動速度に変更 */
void TIM::LED_Stop (void) {
  if (TIM::mode != 0) {
    TIM::mode = 0;
    TCA0_SPLIT_CTRLA = TCA_SPLIT_RUNSTDBY_bm | TCA_SPLIT_ENABLE_bm | TCA_SPLIT_CLKSEL_DIV1_gc;
    LEDG_EVOUT = EVSYS_USER_OFF_gc;
    digitalWrite(LEDG_PIN, LOW);
  }
}

/*
 * HVチャージポンプ駆動制御
 */

void TIM::HV_Pulse_ON (void) {
  TCA0_SPLIT_CTRLB = TCA_SPLIT_HCMP0EN_bm | TCA_SPLIT_HCMP1EN_bm;
}

void TIM::HV_Pulse_OFF (void) {
  TCA0_SPLIT_CTRLB = 0;
}

/*
 * SW1監視 LEVEL割込
 *
 * LOW Active で UPDIターゲットをリセットON
 * Deactive で 本体リセット
 * このハンドラから主処理に戻ることはない
 */

ISR(portIntrruptVector(SW_SENSE_PIN), ISR_NAKED) {
  /* LED は点滅 */
  TIM::LED_Flash();

  /* ターゲットリセットON */
  UPDI::Target_Reset(true);
  UPDI_USART.CTRLB = UPDI_USART_OFF;

  /* チャタリング抑制 */
  delay_micros(800);

  /* ターゲットリセット維持 */
  pinMode(UPDI_TDAT_PIN, OUTPUT);
  digitalWrite(UPDI_TDAT_PIN, LOW);
  openDrainWrite(TRST_PIN, LOW);

  /* LEDは 交互点滅 */
  TIM::LED_Blink();

  /* 押している間は待機 */
  while (!digitalRead(SW_SENSE_PIN));

  /* 本体リセット */
  SYS::System_Reset();
}

/*
 * RTS監視 上下端割込
 *
 * LOW Active で UPDIターゲットをリセットON/OFF
 * Deactive で 本体リセット
 */

ISR(portIntrruptVector(RTS_SENSE_PIN)) {
  portRegister(RTS_SENSE_PIN).INTFLAGS =
  portRegister(RTS_SENSE_PIN).INTFLAGS;

  if ( digitalRead(RTS_SENSE_PIN) ) {
    /* RTS 開放で本体リセット */
    SYS::System_Reset();
  }
  else {
    /* LED は点滅 */
    TIM::LED_Flash();

    /* ターゲット再起動 */
    UPDI::Target_Reset(true);

    /* ターゲットリセットパルス */
    openDrainWrite(TRST_PIN, LOW);
    nop();
    openDrainWrite(TRST_PIN, HIGH);

    /* ターゲット再起動解除 */
    UPDI::Target_Reset(false);
  }
}

/*
 * タイムアウトハンドラ
 *
 * ここから割込元に戻ることはない
 */

ISR(TCB0_INT_vect, ISR_NAKED) {
  __asm__ __volatile__ ("EOR R1,R1");
  TCB0_CTRLA = 0;
  TCB0_INTFLAGS = TCB_CAPT_bm;
  longjmp(TIM::CONTEXT, 2);
}

// end of code

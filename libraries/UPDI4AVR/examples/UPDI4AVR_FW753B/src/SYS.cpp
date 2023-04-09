/**
 * @file SYS.cpp
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "Prototypes.h"
#include <avr/io.h>

void SYS::setup (void) {

  /* ターゲットリセット開放 */
  pinControlRegister(TRST_PIN) = TRST_PIN_CONFIG;

  /* 出力方向ポート */
  PORTA_DIRSET = _BV(pinPosition(HVP1_PIN))
               | _BV(pinPosition(HVP2_PIN))
               | _BV(pinPosition(HV12_PIN))
               | _BV(pinPosition(LEDG_PIN))
               | _BV(pinPosition(LEDY_PIN));
  PORTB_DIRSET = _BV(pinPosition(HV8_PIN));

  /* USART 切替 LOW=ターゲット開通（PG_Disable） */
  PORTC_DIRSET = _BV(pinPosition(PGEN_PIN));

  /* LED 出力 */
  LEDG_EVOUT_MUX = LEDG_EVOUT_ALT;          /* OUT:PA7 */
  PORTMUX_CCLROUTEA = PORTMUX_LUT3_ALT1_gc; /* OUT:PA5 IN:PC0,PC1,PC2 */
  PORTA_OUTSET = _BV(pinPosition(LEDG_PIN))
               | _BV(pinPosition(LEDY_PIN));

  /* USART代替選択 */
  PORTMUX_USARTROUTEA = JTAG_PMUX_ALT | UPDI_PMUX_ALT;

  /* JTAG ポート */
  pinControlRegister(JTAG_TXD_PIN) = JTAG_TXD_CONFIG;
  pinControlRegister(JTAG_RXD_PIN) = JTAG_RXD_CONFIG;

  /* UPDI ポート */
  pinControlRegister(UPDI_TDAT_PIN) = UPDI_TDAT_CONFIG;

  /* HV 発生器 */
  pinControlRegister(HVP1_PIN) = HVP1_PIN_CONFIG;
  pinControlRegister(HVP2_PIN) = HVP2_PIN_CONFIG;

  /* SW1 割込許可 */
  pinControlRegister(SW_SENSE_PIN) = SW_SENSE_CONFIG;

  /* RTS 認識 */
  pinControlRegister(RTS_SENSE_PIN) = RTS_SENSE_CONFIG;

  /* JP1 認識 */
  pinControlRegister(JP_SENSE_PIN) = JP_SENSE_CONFIG;

  /* 状態変数初期化 */
  UPDI_CONTROL = 0;
  UPDI_NVMCTRL = 0;
}

/*
 * 自己VCC測定
 */

uint16_t SYS::get_vcc (void) {
  ADC0_CTRLA = ADC_ENABLE_bm;
  ADC0_CTRLB = ADC_PRESC_DIV2_gc;
  ADC0_CTRLC = ADC_REFSEL_1024MV_gc | ((F_CPU / 1000000UL) << ADC_TIMEBASE_gp);
  ADC0_CTRLE = 17; /* (SAMPDUR + 0.5) * fCLK_ADC = 10.5 µs sample duration */
  ADC0_MUXPOS = ADC_MUXPOS_VDDDIV10_gc; /* ADC channel VDD/10 */
  ADC0_COMMAND = ADC_MODE_SINGLE_12BIT_gc | ADC_START_IMMEDIATE_gc;
  loop_until_bit_is_set(ADC0_INTFLAGS, ADC_SAMPRDY_bp);
  uint16_t adc_reading = ADC0_SAMPLE;
  adc_reading += adc_reading + (adc_reading >> 1);  /* x2.5 */
  ADC0_CTRLA = 0;
  return adc_reading;
}

/*
 * 本体リセット
 */

void SYS::System_Reset (void) {
  _PROTECTED_WRITE(RSTCTRL_SWRR, RSTCTRL_SWRE_bm);
}

/*
 * 各種ON/OFF
 */

void SYS::PG_Enable (void) {
  digitalWrite(PGEN_PIN, HIGH);
}

void SYS::PG_Disable (void) {
  digitalWrite(PGEN_PIN, LOW);
}

void SYS::RTS_Enable (void) {
  pinControlRegister(RTS_SENSE_PIN) = RTS_SENSE_CONFIG;
}

void SYS::RTS_Disable (void) {
  pinControlRegister(RTS_SENSE_PIN) = RTS_SENSE_DISABLE;
}

void SYS::LED_Invert (void) {
  pinControlRegister(LEDG_PIN) = PORT_INVEN_bm | PORT_ISC_INPUT_DISABLE_gc;
}

/*
 * 起動シーケンスの最後に実行
 */

void SYS::ready (void) {
  RSTCTRL_RSTFR = 0xFF;

  /* 初期化中に検出された非同期割込をクリアする */
  portRegister(RTS_SENSE_PIN).INTFLAGS = 0xFF;

  /* 割込許可 */
  sei();
}

/*
 * WDT操作
 */

void SYS::WDT_SET (uint8_t _wdt_period) {
  loop_until_bit_is_clear(WDT_STATUS, WDT_SYNCBUSY_bp);
  _PROTECTED_WRITE(WDT_CTRLA, _wdt_period);
}

void SYS::WDT_OFF (void) { WDT_SET(WDT_PERIOD_OFF_gc); }

void SYS::WDT_ON (void) { WDT_SET(WDT_PERIOD_8KCLK_gc); }

void SYS::WDT_REBOOT (void) {
  WDT_SET(WDT_PERIOD_8CLK_gc);
  for (;;);
}

// end of code

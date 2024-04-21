/**
 * @file variant_io.h
 * @author askn (K.Sato) multix.jp
 * @brief
 * @version 0.1
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

/* C Language Only */
#if !defined (__ASSEMBLER__)

/* Clock Select */
typedef enum TCB_CLKSEL_ALT_enum
{
    TCB_CLKSEL_DIV1_gc = (0x00<<1),  /* CLK_PER */
    TCB_CLKSEL_DIV2_gc = (0x01<<1),  /* CLK_PER/2 */
    TCB_CLKSEL_TCA0_gc = (0x02<<1),  /* Use CLK_TCA from TCA0 */
} TCB_CLKSEL_ALT_t;

typedef enum RTC_CLKSEL_ALT_enum
{
    RTC_CLKSEL_OSC32K_gc = (0x00<<0),   /* 32.768 kHz from OSC32K */
    RTC_CLKSEL_OSC1K_gc = (0x01<<0),    /* 1.024 kHz from OSC32K */
    RTC_CLKSEL_XOSC32K_gc = (0x02<<0)   /* 32.768 kHz from XOSC32K */
} RTC_CLKSEL_ALT_t;

#endif /* !defined (__ASSEMBLER__) */

/* CCL.TRUTH0  bit masks and bit positions */
#define CCL_TRUTH_gm  0xFF  /* Truth Table group mask. */
#define CCL_TRUTH_gp  0  /* Truth Table group position. */
#define CCL_TRUTH_0_bm  (1<<0)  /* Truth Table bit 0 mask. */
#define CCL_TRUTH_0_bp  0  /* Truth Table bit 0 position. */
#define CCL_TRUTH_1_bm  (1<<1)  /* Truth Table bit 1 mask. */
#define CCL_TRUTH_1_bp  1  /* Truth Table bit 1 position. */
#define CCL_TRUTH_2_bm  (1<<2)  /* Truth Table bit 2 mask. */
#define CCL_TRUTH_2_bp  2  /* Truth Table bit 2 position. */
#define CCL_TRUTH_3_bm  (1<<3)  /* Truth Table bit 3 mask. */
#define CCL_TRUTH_3_bp  3  /* Truth Table bit 3 position. */
#define CCL_TRUTH_4_bm  (1<<4)  /* Truth Table bit 4 mask. */
#define CCL_TRUTH_4_bp  4  /* Truth Table bit 4 position. */
#define CCL_TRUTH_5_bm  (1<<5)  /* Truth Table bit 5 mask. */
#define CCL_TRUTH_5_bp  5  /* Truth Table bit 5 position. */
#define CCL_TRUTH_6_bm  (1<<6)  /* Truth Table bit 6 mask. */
#define CCL_TRUTH_6_bp  6  /* Truth Table bit 6 position. */
#define CCL_TRUTH_7_bm  (1<<7)  /* Truth Table bit 7 mask. */
#define CCL_TRUTH_7_bp  7  /* Truth Table bit 7 position. */


/* FUSE - Fuses */
#define FUSE_CODESIZE  _SFR_MEM8(0x1287) /* FUSE_APPEND */
#define FUSE_BOOTSIZE  _SFR_MEM8(0x1288) /* FUSE_BOOTEND */

/* GPR - General Purpose Registers */
#define GPR_GPR0  _SFR_MEM8(0x001C)
#define GPR_GPR1  _SFR_MEM8(0x001D)
#define GPR_GPR2  _SFR_MEM8(0x001E)
#define GPR_GPR3  _SFR_MEM8(0x001F)


#define CLKSEL_OSCHF_gc  CLKCTRL_CLKSEL_OSC20M_gc
#define CLKSEL_OSC32K_gc CLKCTRL_CLKSEL_OSCULP32K_gc
#define SLEEP_MODE_ADC   SLEEP_MODE_IDLE

// end of code

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

#endif /* !defined (__ASSEMBLER__) */

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
#define SLEEP_MODE_ADC   SLEEP_MODE_STANDBY

// end of code

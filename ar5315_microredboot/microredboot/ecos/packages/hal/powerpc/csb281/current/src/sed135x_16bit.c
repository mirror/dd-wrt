//==========================================================================
//
//      sed135x_16bit.c
//
// Author(s):   Michael Kelly - Cogent Computer Systems, Inc.
// Date:        05-24-2002
// Description: Init Code for SED135x Display Controller
//
//==========================================================================
// Copyright (C) 2003 Gary Thomas

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/lcd_support.h>
#include <cyg/hal/hal_io.h>

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef volatile unsigned short vushort;
typedef unsigned long  ulong;
typedef volatile unsigned long  vulong;

// board specific defines needed by sed135x
#define SED_REG_BASE             0x78000000      // *RCE3
#define SED_MEM_BASE             0x78400000
#define SED_STEP                 2               // 16-bit port on 32-bit boundary

// Control/Status Registers, 16-bit mode
#define SED_REG16(_x_)           *(vushort *)((ulong)SED_REG_BASE + (((ulong)_x_ * SED_STEP) ^ 2))   
#define H2SED(_x_)               _le16(_x_)
#define SED_GET_PHYS_ADD(_reg_)  (SED_MEM_BASE + ((_reg_ * SED_STEP) ^ 2))
#define RD_FB16(_reg_,_val_)     ((_val_) = *(vushort *)SED_GET_PHYS_ADD(_reg_))
#define WR_FB16(_reg_,_val_)     (*(vushort *)SED_GET_PHYS_ADD(_reg_) = (_val_))

#define SED_DISP_MODE_CRT        1
#define SED_DISP_MODE_LCD        0

#include "sed1356_16bit.h"

//------------------------------------------------------------------------
// Color Lookup Table Values
//

static ushort sed_lut_16bit[256][3] = {
//    RED,  GREEN, BLUE    // Entry
  { 0x0000,  0x0000, 0x0000, },  // 00  
  { 0x0000,  0x0000, 0x00A0, },  // 01 
  { 0x0000,  0x00A0, 0x0000, },  // 02 
  { 0x0000,  0x00A0, 0x00A0, },  // 03 
  { 0x00A0,  0x0000, 0x0000, },  // 04 
  { 0x00A0,  0x0000, 0x00A0, },  // 05 
  { 0x00A0,  0x00A0, 0x0000, },  // 06 
  { 0x00A0,  0x00A0, 0x00A0, },  // 07 
  { 0x0050,  0x0050, 0x0050, },  // 08 
  { 0x0050,  0x0050, 0x00F0, },  // 09 
  { 0x0050,  0x00F0, 0x0050, },  // 0A 
  { 0x0050,  0x00F0, 0x00F0, },  // 0B 
  { 0x00F0,  0x0050, 0x0050, },  // 0C 
  { 0x00F0,  0x0050, 0x00F0, },  // 0D 
  { 0x00F0,  0x00F0, 0x0050, },  // 0E 
  { 0x00F0,  0x00F0, 0x00F0, },  // 0F 
  { 0x0000,  0x0000, 0x0000, },  // 10 
  { 0x0010,  0x0010, 0x0010, },  // 11 
  { 0x0020,  0x0020, 0x0020, },  // 12 
  { 0x0020,  0x0020, 0x0020, },  // 13 
  { 0x0030,  0x0030, 0x0030, },  // 14 
  { 0x0040,  0x0040, 0x0040, },  // 15 
  { 0x0050,  0x0050, 0x0050, },  // 16 
  { 0x0060,  0x0060, 0x0060, },  // 17 
  { 0x0070,  0x0070, 0x0070, },  // 18 
  { 0x0080,  0x0080, 0x0080, },  // 19 
  { 0x0090,  0x0090, 0x0090, },  // 1A 
  { 0x00A0,  0x00A0, 0x00A0, },  // 1B 
  { 0x00B0,  0x00B0, 0x00B0, },  // 1C 
  { 0x00C0,  0x00C0, 0x00C0, },  // 1D 
  { 0x00E0,  0x00E0, 0x00E0, },  // 1E 
  { 0x00F0,  0x00F0, 0x00F0, },  // 1F 
  { 0x0000,  0x0000, 0x00F0, },  // 20 
  { 0x0040,  0x0000, 0x00F0, },  // 21 
  { 0x0070,  0x0000, 0x00F0, },  // 22 
  { 0x00B0,  0x0000, 0x00F0, },  // 23 
  { 0x00F0,  0x0000, 0x00F0, },  // 24 
  { 0x00F0,  0x0000, 0x00B0, },  // 25 
  { 0x00F0,  0x0000, 0x0070, },  // 26 
  { 0x00F0,  0x0000, 0x0040, },  // 27 
  { 0x00F0,  0x0000, 0x0000, },  // 28 
  { 0x00F0,  0x0040, 0x0000, },  // 29 
  { 0x00F0,  0x0070, 0x0000, },  // 2A 
  { 0x00F0,  0x00B0, 0x0000, },  // 2B 
  { 0x00F0,  0x00F0, 0x0000, },  // 2C 
  { 0x00B0,  0x00F0, 0x0000, },  // 2D 
  { 0x0070,  0x00F0, 0x0000, },  // 2E 
  { 0x0040,  0x00F0, 0x0000, },  // 2F 
  { 0x0000,  0x00F0, 0x0000, },  // 30 
  { 0x0000,  0x00F0, 0x0040, },  // 31 
  { 0x0000,  0x00F0, 0x0070, },  // 32 
  { 0x0000,  0x00F0, 0x00B0, },  // 33 
  { 0x0000,  0x00F0, 0x00F0, },  // 34 
  { 0x0000,  0x00B0, 0x00F0, },  // 35 
  { 0x0000,  0x0070, 0x00F0, },  // 36 
  { 0x0000,  0x0040, 0x00F0, },  // 37 
  { 0x0070,  0x0070, 0x00F0, },  // 38 
  { 0x0090,  0x0070, 0x00F0, },  // 39 
  { 0x00B0,  0x0070, 0x00F0, },  // 3A 
  { 0x00D0,  0x0070, 0x00F0, },  // 3B 
  { 0x00F0,  0x0070, 0x00F0, },  // 3C 
  { 0x00F0,  0x0070, 0x00D0, },  // 3D 
  { 0x00F0,  0x0070, 0x00B0, },  // 3E 
  { 0x00F0,  0x0070, 0x0090, },  // 3F 
  { 0x00F0,  0x0070, 0x0070, },  // 40 
  { 0x00F0,  0x0090, 0x0070, },  // 41 
  { 0x00F0,  0x00B0, 0x0070, },  // 42 
  { 0x00F0,  0x00D0, 0x0070, },  // 43 
  { 0x00F0,  0x00F0, 0x0070, },  // 44 
  { 0x00D0,  0x00F0, 0x0070, },  // 45 
  { 0x00B0,  0x00F0, 0x0070, },  // 46 
  { 0x0090,  0x00F0, 0x0070, },  // 47 
  { 0x0070,  0x00F0, 0x0070, },  // 48 
  { 0x0070,  0x00F0, 0x0090, },  // 49 
  { 0x0070,  0x00F0, 0x00B0, },  // 4A 
  { 0x0070,  0x00F0, 0x00D0, },  // 4B 
  { 0x0070,  0x00F0, 0x00F0, },  // 4C 
  { 0x0070,  0x00D0, 0x00F0, },  // 4D 
  { 0x0070,  0x00B0, 0x00F0, },  // 4E 
  { 0x0070,  0x0090, 0x00F0, },  // 4F 
  { 0x00B0,  0x00B0, 0x00F0, },  // 50 
  { 0x00C0,  0x00B0, 0x00F0, },  // 51 
  { 0x00D0,  0x00B0, 0x00F0, },  // 52 
  { 0x00E0,  0x00B0, 0x00F0, },  // 53 
  { 0x00F0,  0x00B0, 0x00F0, },  // 54 
  { 0x00F0,  0x00B0, 0x00E0, },  // 55 
  { 0x00F0,  0x00B0, 0x00D0, },  // 56 
  { 0x00F0,  0x00B0, 0x00C0, },  // 57 
  { 0x00F0,  0x00B0, 0x00B0, },  // 58 
  { 0x00F0,  0x00C0, 0x00B0, },  // 59 
  { 0x00F0,  0x00D0, 0x00B0, },  // 5A 
  { 0x00F0,  0x00E0, 0x00B0, },  // 5B 
  { 0x00F0,  0x00F0, 0x00B0, },  // 5C 
  { 0x00E0,  0x00F0, 0x00B0, },  // 5D 
  { 0x00D0,  0x00F0, 0x00B0, },  // 5E 
  { 0x00C0,  0x00F0, 0x00B0, },  // 5F 
  { 0x00B0,  0x00F0, 0x00B0, },  // 60 
  { 0x00B0,  0x00F0, 0x00C0, },  // 61 
  { 0x00B0,  0x00F0, 0x00D0, },  // 62 
  { 0x00B0,  0x00F0, 0x00E0, },  // 63 
  { 0x00B0,  0x00F0, 0x00F0, },  // 64 
  { 0x00B0,  0x00E0, 0x00F0, },  // 65 
  { 0x00B0,  0x00D0, 0x00F0, },  // 66 
  { 0x00B0,  0x00C0, 0x00F0, },  // 67 
  { 0x0000,  0x0000, 0x0070, },  // 68 
  { 0x0010,  0x0000, 0x0070, },  // 69 
  { 0x0030,  0x0000, 0x0070, },  // 6A 
  { 0x0050,  0x0000, 0x0070, },  // 6B 
  { 0x0070,  0x0000, 0x0070, },  // 6C 
  { 0x0070,  0x0000, 0x0050, },  // 6D 
  { 0x0070,  0x0000, 0x0030, },  // 6E 
  { 0x0070,  0x0000, 0x0010, },  // 6F 
  { 0x0070,  0x0000, 0x0000, },  // 70 
  { 0x0070,  0x0010, 0x0000, },  // 71 
  { 0x0070,  0x0030, 0x0000, },  // 72 
  { 0x0070,  0x0050, 0x0000, },  // 73 
  { 0x0070,  0x0070, 0x0000, },  // 74 
  { 0x0050,  0x0070, 0x0000, },  // 75 
  { 0x0030,  0x0070, 0x0000, },  // 76 
  { 0x0010,  0x0070, 0x0000, },  // 77 
  { 0x0000,  0x0070, 0x0000, },  // 78 
  { 0x0000,  0x0070, 0x0010, },  // 79 
  { 0x0000,  0x0070, 0x0030, },  // 7A 
  { 0x0000,  0x0070, 0x0050, },  // 7B 
  { 0x0000,  0x0070, 0x0070, },  // 7C 
  { 0x0000,  0x0050, 0x0070, },  // 7D 
  { 0x0000,  0x0030, 0x0070, },  // 7E 
  { 0x0000,  0x0010, 0x0070, },  // 7F 
  { 0x0030,  0x0030, 0x0070, },  // 80 
  { 0x0040,  0x0030, 0x0070, },  // 81 
  { 0x0050,  0x0030, 0x0070, },  // 82 
  { 0x0060,  0x0030, 0x0070, },  // 83 
  { 0x0070,  0x0030, 0x0070, },  // 84 
  { 0x0070,  0x0030, 0x0060, },  // 85 
  { 0x0070,  0x0030, 0x0050, },  // 86 
  { 0x0070,  0x0030, 0x0040, },  // 87 
  { 0x0070,  0x0030, 0x0030, },  // 88 
  { 0x0070,  0x0040, 0x0030, },  // 89 
  { 0x0070,  0x0050, 0x0030, },  // 8A 
  { 0x0070,  0x0060, 0x0030, },  // 8B 
  { 0x0070,  0x0070, 0x0030, },  // 8C 
  { 0x0060,  0x0070, 0x0030, },  // 8D 
  { 0x0050,  0x0070, 0x0030, },  // 8E 
  { 0x0040,  0x0070, 0x0030, },  // 8F 
  { 0x0030,  0x0070, 0x0030, },  // 90 
  { 0x0030,  0x0070, 0x0040, },  // 91 
  { 0x0030,  0x0070, 0x0050, },  // 92 
  { 0x0030,  0x0070, 0x0060, },  // 93 
  { 0x0030,  0x0070, 0x0070, },  // 94 
  { 0x0030,  0x0060, 0x0070, },  // 95 
  { 0x0030,  0x0050, 0x0070, },  // 96 
  { 0x0030,  0x0040, 0x0070, },  // 97 
  { 0x0050,  0x0050, 0x0070, },  // 98 
  { 0x0050,  0x0050, 0x0070, },  // 99 
  { 0x0060,  0x0050, 0x0070, },  // 9A 
  { 0x0060,  0x0050, 0x0070, },  // 9B 
  { 0x0070,  0x0050, 0x0070, },  // 9C 
  { 0x0070,  0x0050, 0x0060, },  // 9D 
  { 0x0070,  0x0050, 0x0060, },  // 9E 
  { 0x0070,  0x0050, 0x0050, },  // 9F 
  { 0x0070,  0x0050, 0x0050, },  // A0 
  { 0x0070,  0x0050, 0x0050, },  // A1 
  { 0x0070,  0x0060, 0x0050, },  // A2 
  { 0x0070,  0x0060, 0x0050, },  // A3 
  { 0x0070,  0x0070, 0x0050, },  // A4 
  { 0x0060,  0x0070, 0x0050, },  // A5 
  { 0x0060,  0x0070, 0x0050, },  // A6 
  { 0x0050,  0x0070, 0x0050, },  // A7 
  { 0x0050,  0x0070, 0x0050, },  // A8 
  { 0x0050,  0x0070, 0x0050, },  // A9 
  { 0x0050,  0x0070, 0x0060, },  // AA 
  { 0x0050,  0x0070, 0x0060, },  // AB 
  { 0x0050,  0x0070, 0x0070, },  // AC 
  { 0x0050,  0x0060, 0x0070, },  // AD 
  { 0x0050,  0x0060, 0x0070, },  // AE 
  { 0x0050,  0x0050, 0x0070, },  // AF 
  { 0x0000,  0x0000, 0x0040, },  // B0 
  { 0x0010,  0x0000, 0x0040, },  // B1 
  { 0x0020,  0x0000, 0x0040, },  // B2 
  { 0x0030,  0x0000, 0x0040, },  // B3 
  { 0x0040,  0x0000, 0x0040, },  // B4 
  { 0x0040,  0x0000, 0x0030, },  // B5 
  { 0x0040,  0x0000, 0x0020, },  // B6 
  { 0x0040,  0x0000, 0x0010, },  // B7 
  { 0x0040,  0x0000, 0x0000, },  // B8 
  { 0x0040,  0x0010, 0x0000, },  // B9 
  { 0x0040,  0x0020, 0x0000, },  // BA 
  { 0x0040,  0x0030, 0x0000, },  // BB 
  { 0x0040,  0x0040, 0x0000, },  // BC 
  { 0x0030,  0x0040, 0x0000, },  // BD 
  { 0x0020,  0x0040, 0x0000, },  // BE 
  { 0x0010,  0x0040, 0x0000, },  // BF 
  { 0x0000,  0x0040, 0x0000, },  // C0 
  { 0x0000,  0x0040, 0x0010, },  // C1 
  { 0x0000,  0x0040, 0x0020, },  // C2 
  { 0x0000,  0x0040, 0x0030, },  // C3 
  { 0x0000,  0x0040, 0x0040, },  // C4 
  { 0x0000,  0x0030, 0x0040, },  // C5 
  { 0x0000,  0x0020, 0x0040, },  // C6 
  { 0x0000,  0x0010, 0x0040, },  // C7 
  { 0x0020,  0x0020, 0x0040, },  // C8 
  { 0x0020,  0x0020, 0x0040, },  // C9 
  { 0x0030,  0x0020, 0x0040, },  // CA 
  { 0x0030,  0x0020, 0x0040, },  // CB 
  { 0x0040,  0x0020, 0x0040, },  // CC 
  { 0x0040,  0x0020, 0x0030, },  // CD 
  { 0x0040,  0x0020, 0x0030, },  // CE 
  { 0x0040,  0x0020, 0x0020, },  // CF 
  { 0x0040,  0x0020, 0x0020, },  // D0 
  { 0x0040,  0x0020, 0x0020, },  // D1 
  { 0x0040,  0x0030, 0x0020, },  // D2 
  { 0x0040,  0x0030, 0x0020, },  // D3 
  { 0x0040,  0x0040, 0x0020, },  // D4 
  { 0x0030,  0x0040, 0x0020, },  // D5 
  { 0x0030,  0x0040, 0x0020, },  // D6 
  { 0x0020,  0x0040, 0x0020, },  // D7 
  { 0x0020,  0x0040, 0x0020, },  // D8 
  { 0x0020,  0x0040, 0x0020, },  // D9 
  { 0x0020,  0x0040, 0x0030, },  // DA 
  { 0x0020,  0x0040, 0x0030, },  // DB 
  { 0x0020,  0x0040, 0x0040, },  // DC 
  { 0x0020,  0x0030, 0x0040, },  // DD 
  { 0x0020,  0x0030, 0x0040, },  // DE 
  { 0x0020,  0x0020, 0x0040, },  // DF 
  { 0x0020,  0x0020, 0x0040, },  // E0 
  { 0x0030,  0x0020, 0x0040, },  // E1 
  { 0x0030,  0x0020, 0x0040, },  // E2 
  { 0x0030,  0x0020, 0x0040, },  // E3 
  { 0x0040,  0x0020, 0x0040, },  // E4 
  { 0x0040,  0x0020, 0x0030, },  // E5 
  { 0x0040,  0x0020, 0x0030, },  // E6 
  { 0x0040,  0x0020, 0x0030, },  // E7 
  { 0x0040,  0x0020, 0x0020, },  // E8 
  { 0x0040,  0x0030, 0x0020, },  // E9 
  { 0x0040,  0x0030, 0x0020, },  // EA 
  { 0x0040,  0x0030, 0x0020, },  // EB 
  { 0x0040,  0x0040, 0x0020, },  // EC 
  { 0x0030,  0x0040, 0x0020, },  // ED 
  { 0x0030,  0x0040, 0x0020, },  // EE 
  { 0x0030,  0x0040, 0x0020, },  // EF 
  { 0x0020,  0x0040, 0x0020, },  // F0 
  { 0x0020,  0x0040, 0x0030, },  // F1 
  { 0x0020,  0x0040, 0x0030, },  // F2 
  { 0x0020,  0x0040, 0x0030, },  // F3 
  { 0x0020,  0x0040, 0x0040, },  // F4 
  { 0x0020,  0x0030, 0x0040, },  // F5 
  { 0x0020,  0x0030, 0x0040, },  // F6 
  { 0x0020,  0x0030, 0x0040, },  // F7 
  { 0x0000,  0x0000, 0x0000, },  // F8 
  { 0x0000,  0x0000, 0x0000, },  // F9 
  { 0x0000,  0x0000, 0x0000, },  // FA 
  { 0x0000,  0x0000, 0x0000, },  // FB 
  { 0x0000,  0x0000, 0x0000, },  // FC 
  { 0x0000,  0x0000, 0x0000, },  // FD 
  { 0x0000,  0x0000, 0x0000, },  // FE 
  { 0x0000,  0x0000, 0x0000, },  // FF 
};

//--------------------------------------------------------------------------
// function prototypes
//
int         sed135x_init(int depth, struct lcd_info *lcd);
static void sed135x_on(void);
static void sed135x_off(void);
static void sed_lcd_bkl(uchar bright);

extern void fs6377_init(int);
// Why doesn't the real mode work?
#define fs6377_init(mode) _csb281_fs6377_init(0)

// global flags to determine what, if anything, was found
static int sed135x_ok;
static int sed_disp_mode_crt;

// GPIO1 is used to control the LCD backlight on many CSB's
#define SED1356_BKL_ON          SED1356_REG_GPIO_CTL |= H2SED(SED1356_GPIO_GPIO1)   // GPIO1 = 1 
#define SED1356_BKL_OFF         SED1356_REG_GPIO_CTL &= H2SED(~SED1356_GPIO_GPIO1)  // GPIO1 = 0

// GPIO2 is used to sense the presence of a monitor.  0 = monitor connected, 1 = no monitor
// we invert the sense to make it easier to test and more logical.
#define SED1356_CRT             SED1356_REG_GPIO_CTL & H2SED(SED1356_GPIO_GPIO2)

#define SED_ROW_SIZE(_depth_)   ((PIXELS_PER_ROW * _depth_) / 8)

//--------------------------------------------------------------------------
// sed135x_on
//
// This function turns on the SED1355 or SED1356 LCD and/or CRT
//
static void 
sed135x_on(void)
{
    uchar temp8;
    int i;

    sed135x_off();

    // Turn on the LCD and/or CRT
    // The SED1356 supports seperate LCD and CRT timing registers
    // that have already been setup.  We just blank the side we 
    // aren't using and enable the other.
    if (sed_disp_mode_crt) {  // 1 = CRT Mode
        // Blank the LCD and CRT
        SED1356_REG_LCD_DISP_MODE_and_MISC |= H2SED(SED1356_LCD_DISP_BLANK);
        SED1356_REG_CRT_DISP_MODE |= H2SED(SED1356_CRT_DISP_BLANK);

        // turn the LCD backlight off
        sed_lcd_bkl(0);

        // Set the SED1356 to CRT Mode
        SED1356_REG_DISP_MODE = H2SED(SED1356_DISP_SWIV_NORM | 
                                      SED1356_DISP_MODE_CRT);

        // Turn on the CRT
        SED1356_REG_CRT_DISP_MODE &= ~H2SED(SED1356_CRT_DISP_BLANK);
    } // if CRT mode
    else {  // 0 = LCD Mode
        // Blank the LCD and CRT
        SED1356_REG_LCD_DISP_MODE_and_MISC |= H2SED(SED1356_LCD_DISP_BLANK);
        SED1356_REG_CRT_DISP_MODE |= H2SED(SED1356_CRT_DISP_BLANK);

        // Set the SED1356 to LCD Mode
        SED1356_REG_DISP_MODE = H2SED(SED1356_DISP_SWIV_NORM | 
                                      SED1356_DISP_MODE_LCD);

        // Turn on the LCD
        SED1356_REG_LCD_DISP_MODE_and_MISC &= ~H2SED(SED1356_LCD_DISP_BLANK);
        sed_lcd_bkl(0xff);    // turn the LCD backlight on/full brightness
    } // else LCD Mode
}

//--------------------------------------------------------------------------
// sed_lcd_bkl()
//
// This function turns on the LCD backlight connected to GPIO1.  This is
// not used if the board has a different method of controlling the
// backlight.  Since the Sed has only a single GPIO bit and no way
// to modulate it, we use any non-zero value of bright to turn it on.
//
static void 
sed_lcd_bkl(uchar bright)
{
    // Any non-zero value for bright means on
    if (bright) 
        SED1356_BKL_ON;
    else 
        SED1356_BKL_OFF;

}

//--------------------------------------------------------------------------
// sed135x_off
//
// This function turns off the SED1356 LCD and/or CRT and the display
// fifo.  It can also turn off the clocks if mode is true, thus allowing
// the programmable clock generator to be changed.
//
static void 
sed135x_off(void)
{
    SED1356_REG_DISP_MODE = H2SED(SED1356_DISP_SWIV_NORM | 
                                  SED1356_DISP_MODE_OFF);
    sed_lcd_bkl(0);  // turn the LCD backlight off
}

//--------------------------------------------------------------------------
// sed135x_init
//
// This function sets up the sed1355 or sed1356 whichever is found
//
int 
sed135x_init(int depth, struct lcd_info *lcd)
{
    vushort temp16;
    int i;
  
    sed135x_ok = 0;
    sed_disp_mode_crt = 0;  // assume LCD

    if ((depth != 4) && (depth != 8) && (depth != 16)) {
        diag_printf("Invalid depth: %d\n", depth);
        return -1;
    }

    // enable host access
    SED1356_REG_REV_and_MISC = 0x0000;

    // Check the ID to make sure we even have a SED1356 installed
    temp16 = SED1356_REG_REV_and_MISC & H2SED(SED1356_REV_ID_MASK);

    if (temp16 != H2SED(SED1356_REV_ID_1356)){
        diag_printf("SED1356 Not Found! SED_REG_REV = %04x.\n", temp16);
        return -1;
    }

    // Disable the display
    SED1356_REG_DISP_MODE = H2SED(SED1356_DISP_SWIV_NORM | 
                                  SED1356_DISP_MODE_OFF);

    // Test for the presence of a CRT
    SED1356_REG_GPIO_CTL = 0x0000;                      // Disable Backlight 
    SED1356_REG_GPIO_CFG = H2SED(SED1356_GPIO_GPIO1);  // GPIO1 Out, GPIO2 In
    if (SED1356_CRT) sed_disp_mode_crt = 0; 
    else sed_disp_mode_crt = 1;

    // Enable Power Save Mode before we mess with the clocks
    SED1356_REG_MEM_CFG_and_REF_RATE = H2SED(SED1356_REF_TYPE_SELF); // set dram to self refresh first
    // shut off MCLK
    SED1356_REG_PWR_CFG_and_STAT = H2SED(SED1356_PWR_MCLK);

    // Wait until power is down - when MCLK bit goes true
    while ((SED1356_REG_PWR_CFG_and_STAT & H2SED(SED1356_PWR_MCLK)) == 0){}

    // Change the programmable clock generator to the desired timing
    if (sed_disp_mode_crt) fs6377_init(SED_DISP_MODE_CRT);
    else fs6377_init(SED_DISP_MODE_LCD);

    // Re-enable Power
    SED1356_REG_PWR_CFG_and_STAT = 0x0000;

    // Common Control Registers
    SED1356_REG_MCLK_CFG = H2SED(SED1356_MCLK_SRC_BCLK);
    SED1356_REG_LCD_PCLK_CFG = H2SED(SED1356_PCLK_SRC_CLKI);
    SED1356_REG_CRT_PCLK_CFG = H2SED(SED1356_PCLK_SRC_CLKI);
    SED1356_REG_MEDIA_PCLK_CFG = 0x0000;
    SED1356_REG_WAIT_STATE = H2SED(0x0001);
    SED1356_REG_MEM_CFG_and_REF_RATE = H2SED(SED1356_MEM_CFG_2CAS_EDO | 
                                             SED1356_REF_RATE_2048);
    SED1356_REG_MEM_TMG0_and_1 = H2SED(SED1356_MEM_TMG0_EDO50_MCLK33 | 
                                       SED1356_MEM_TMG1_EDO50_MCLK33);
    SED1356_REG_PANEL_TYPE_and_MOD_RATE = H2SED(SED1356_PANEL_TYPE_16 | 
                                                SED1356_PANEL_TYPE_CLR | 
                                                SED1356_PANEL_TYPE_TFT);

    // LCD Specific Registers
    SED1356_REG_LCD_HOR_DISP = H2SED((PIXELS_PER_ROW/8) - 1);
    SED1356_REG_LCD_HOR_NONDISP_and_START = H2SED(SED_HOR_NONDISP_LCD | 
                                                  (SED_HOR_PULSE_START_LCD << 8));
    SED1356_REG_LCD_HOR_PULSE = H2SED(SED1356_PULSE_WID(SED_HOR_PULSE_WIDTH_LCD) | 
                                      SED1356_PULSE_POL_LOW);
    SED1356_REG_LCD_VER_DISP_HT_LO_and_HI = H2SED((PIXELS_PER_COL - 1) & 0x3ff);
    SED1356_REG_LCD_VER_NONDISP_and_START = H2SED(SED_VER_NONDISP_LCD | 
                                                  (SED_VER_PULSE_START_LCD << 8));
    SED1356_REG_LCD_VER_PULSE = H2SED(SED1356_PULSE_WID(SED_VER_PULSE_WIDTH_LCD) | 
                                      SED1356_PULSE_POL_LOW);
    switch (depth) {
    case 4:  SED1356_REG_LCD_DISP_MODE_and_MISC = H2SED(SED1356_LCD_DISP_BLANK | 
                                                        SED1356_LCD_DISP_SWIV_NORM | 
                                                        SED1356_LCD_DISP_4BPP);  
        break;
    case 8:  SED1356_REG_LCD_DISP_MODE_and_MISC = H2SED(SED1356_LCD_DISP_BLANK | 
                                                        SED1356_LCD_DISP_SWIV_NORM | 
                                                        SED1356_LCD_DISP_8BPP);  
        break;
    default: SED1356_REG_LCD_DISP_MODE_and_MISC = H2SED(SED1356_LCD_DISP_BLANK | 
                                                        SED1356_LCD_DISP_SWIV_NORM | 
                                                        SED1356_LCD_DISP_16BPP); break;
    }

    SED1356_REG_LCD_DISP_START_LO_and_MID = 0x0000;
    SED1356_REG_LCD_DISP_START_HI = 0x0000;
    SED1356_REG_LCD_ADD_OFFSET_LO_and_HI = H2SED((SED_ROW_SIZE(depth) / 2) & 0x7ff);
    SED1356_REG_LCD_PIXEL_PAN = 0x0000;
    SED1356_REG_LCD_FIFO_THRESH_LO_and_HI = 0x0000;  // auto mode

    // LCD Specific Registers
    SED1356_REG_CRT_HOR_DISP = H2SED((PIXELS_PER_ROW/8) - 1);
    SED1356_REG_CRT_HOR_NONDISP_and_START = H2SED(SED_HOR_NONDISP_CRT | 
                                                  (SED_HOR_PULSE_START_CRT << 8));
    SED1356_REG_CRT_HOR_PULSE = H2SED(SED1356_PULSE_WID(SED_HOR_PULSE_WIDTH_CRT) | 
                                      SED1356_PULSE_POL_LOW);
    SED1356_REG_CRT_VER_DISP_HT_LO_and_HI = H2SED((PIXELS_PER_COL - 1) & 0x3ff);
    SED1356_REG_CRT_VER_NONDISP_and_START = H2SED(SED_VER_NONDISP_CRT | 
                                                  (SED_VER_PULSE_START_CRT << 8));
    SED1356_REG_CRT_VER_PULSE_and_OUT_CTL = H2SED(SED1356_PULSE_WID(SED_VER_PULSE_WIDTH_CRT) | 
                                                  SED1356_PULSE_POL_LOW | SED1356_CRT_OUT_DAC_LVL);
    switch (depth) {
    case 4:  SED1356_REG_CRT_DISP_MODE = H2SED(SED1356_CRT_DISP_BLANK | 
                                               SED1356_CRT_DISP_4BPP);  
        break;
    case 8:  SED1356_REG_CRT_DISP_MODE = H2SED(SED1356_CRT_DISP_BLANK | 
                                               SED1356_CRT_DISP_8BPP);  
        break;
    default: SED1356_REG_CRT_DISP_MODE = H2SED(SED1356_CRT_DISP_BLANK | 
                                               SED1356_CRT_DISP_16BPP); 
        break;
    }


    SED1356_REG_CRT_DISP_START_LO_and_MID = 0x0000;
    SED1356_REG_CRT_DISP_START_HI = 0x0000;
    SED1356_REG_CRT_ADD_OFFSET_LO_and_HI = H2SED((SED_ROW_SIZE(depth) / 2) & 0x7ff);
    SED1356_REG_CRT_PIXEL_PAN = 0x0000;
    SED1356_REG_CRT_FIFO_THRESH_LO_and_HI = 0x0000;  // auto mode

    // Disable Cursor
    SED1356_REG_LCD_CURSOR_CTL_and_START_ADD = 0x0000;
    SED1356_REG_CRT_CURSOR_CTL_and_START_ADD = 0x0000;

    // Disable BitBlt
    SED1356_REG_BLT_CTL_0_and_1 = 0x0000;
    SED1356_REG_BLT_ROP_CODE_and_BLT_OP = 0x0000;
    SED1356_REG_BLT_SRC_START_LO_and_MID = 0x0000;
    SED1356_REG_BLT_SRC_START_HI = 0x0000;
    SED1356_REG_BLT_DEST_START_LO_and_MID = 0x0000;
    SED1356_REG_BLT_DEST_START_HI = 0x0000;
    SED1356_REG_BLT_ADD_OFFSET_LO_and_HI = 0x0000;
    SED1356_REG_BLT_WID_LO_and_HI = 0x0000;
    SED1356_REG_BLT_HGT_LO_and_HI = 0x0000;
    SED1356_REG_BLT_BG_CLR_LO_and_HI = 0x0000;
    SED1356_REG_BLT_FG_CLR_LO_and_HI = 0x0000;

    // Fill the LUT, write to both LCD and CRT luts simultaneously
    SED1356_REG_LUT_MODE = 0x0000;
    for (i = 0; i < 256; i++){

        SED1356_REG_LUT_ADD = H2SED(i);
        SED1356_REG_LUT_DATA = H2SED(sed_lut_16bit[i][0]);  // red
        SED1356_REG_LUT_DATA = H2SED(sed_lut_16bit[i][1]);  // green
        SED1356_REG_LUT_DATA = H2SED(sed_lut_16bit[i][2]);  // blue
    }

    // Disable Power Save Mode
    SED1356_REG_PWR_CFG_and_STAT = 0x0000;

    // Set Watchdog               
//  SED1356_REG_WATCHDOG_CTL = 0x0000;

    // Device found & initialized
    sed135x_ok = 1;

    // turn on the display
    sed135x_on();

    // Fill in the info structure
    lcd->height = 480;            // FIXME
    lcd->width = 640;             // FIXME
    lcd->bpp = depth;
    lcd->type = FB_TRUE_RGB565;
    lcd->rlen = (640*2*2);        // FIXME
    lcd->access_size = 2;         // Framebuffer fixed at 16 bit access
    lcd->stride = 4;              // Only on "odd" 16 byte chunks
    lcd->fb = SED_GET_PHYS_ADD(0);
    lcd->on = sed135x_on;
    lcd->off = sed135x_off;

    return 0;
}

//--------------------------------------------------------------------------
// sed_pwr_dn()
static void 
sed_pwr_dn(void)
{

    // Enable Host Access
    SED1356_REG_REV_and_MISC = 0x0000;

    // Disable the display
    SED1356_REG_DISP_MODE = H2SED(SED1356_DISP_MODE_OFF);

    // Enable Power Save Mode
    // set dram to self refresh first
    SED1356_REG_MEM_CFG_and_REF_RATE = H2SED(SED1356_REF_TYPE_SELF);

    // shut off MCLK
    SED1356_REG_PWR_CFG_and_STAT = H2SED(SED1356_PWR_MCLK);

    // Wait until power is down - when MCLK bit goes true
    while ((SED1356_REG_PWR_CFG_and_STAT & H2SED(SED1356_PWR_MCLK)) == 0){}
}



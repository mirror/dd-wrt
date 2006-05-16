/*
 * BRIEF MODULE DESCRIPTION
 *	Au1200 LCD Driver.
 *
 * Copyright 2004 AMD
 * Author: AMD
 *
 * Based on:
 * linux/drivers/video/skeletonfb.c -- Skeleton for a frame buffer device
 *  Created 28 Dec 1997 by Geert Uytterhoeven
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED	  ``AS	IS'' AND   ANY	EXPRESS OR IMPLIED
 *  WARRANTIES,	  INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO	EVENT  SHALL   THE AUTHOR  BE	 LIABLE FOR ANY	  DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED	  TO, PROCUREMENT OF  SUBSTITUTE GOODS	OR SERVICES; LOSS OF
 *  USE, DATA,	OR PROFITS; OR	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN	 CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <asm/uaccess.h>

#include <asm/au1000.h>
#include <asm/au1xxx_gpio.h>
#include "au1200fb.h"

#include <video/fbcon.h>
#include <video/fbcon-cfb16.h>
#include <video/fbcon-cfb32.h>
#define CMAPSIZE 16

#define AU1200_LCD_GET_WINENABLE	1
#define AU1200_LCD_SET_WINENABLE	2
#define AU1200_LCD_GET_WINLOCATION	3
#define AU1200_LCD_SET_WINLOCATION	4
#define AU1200_LCD_GET_WINSIZE		5
#define AU1200_LCD_SET_WINSIZE		6
#define AU1200_LCD_GET_BACKCOLOR	7
#define AU1200_LCD_SET_BACKCOLOR	8
#define AU1200_LCD_GET_COLORKEY	9
#define AU1200_LCD_SET_COLORKEY	10
#define AU1200_LCD_GET_PANEL		11
#define AU1200_LCD_SET_PANEL		12

typedef struct au1200_lcd_getset_t
{
	unsigned int subcmd;
	union {
		struct {
			int enable;
		} winenable;
		struct {
			int x, y;
		} winlocation;
		struct {
			int hsz, vsz;
		} winsize;
		struct {
			unsigned int color;
		} backcolor;
		struct {
			unsigned int key;
			unsigned int mask;
		} colorkey;
		struct {
			int panel;
			char desc[80];
		} panel;
	};
} au1200_lcd_getset_t;

AU1200_LCD *lcd = (AU1200_LCD *)AU1200_LCD_ADDR;
static int window_index = 0; /* default is zero */
static int panel_index = -1; /* default is call board_au1200fb_panel */

struct window_settings
{
	unsigned char name[64];
	uint32 mode_backcolor;
	uint32 mode_colorkey;
	uint32 mode_colorkeymsk;
	struct
	{
		int xres;
		int yres;
		int xpos;
		int ypos;
		uint32 mode_winctrl1; /* winctrl1[FRM,CCO,PO,PIPE] */
		uint32 mode_winenable;
	} w[4];
};

struct panel_settings
{
	unsigned char name[64];
	/* panel physical dimensions */
	uint32 Xres;
	uint32 Yres;
	/* panel timings */
	uint32 mode_screen;
	uint32 mode_horztiming;
	uint32 mode_verttiming;
	uint32 mode_clkcontrol;
	uint32 mode_pwmdiv;
	uint32 mode_pwmhi;
	uint32 mode_outmask;
	uint32 mode_fifoctrl;
	uint32 mode_toyclksrc;
	uint32 mode_backlight;
	uint32 mode_auxpll;
	int (*device_init)(void);
	int (*device_shutdown)(void);
};

#if defined(__BIG_ENDIAN)
#define LCD_WINCTRL1_PO_16BPP LCD_WINCTRL1_PO_00
#else
#define LCD_WINCTRL1_PO_16BPP LCD_WINCTRL1_PO_01
#endif

extern int board_au1200fb_panel (void);
extern int board_au1200fb_panel_init (void);
extern int board_au1200fb_panel_shutdown (void);

#if defined(CONFIG_FOCUS_ENHANCEMENTS)
extern int board_au1200fb_focus_init_hdtv(void);
extern int board_au1200fb_focus_init_component(void);
extern int board_au1200fb_focus_init_cvsv(void);
extern int board_au1200fb_focus_shutdown(void);
#endif

/*
 * Default window configurations
 */
static struct window_settings windows[] =
{
	{ /* Index 0 */
		"0-FS gfx, 1-video, 2-ovly gfx, 3-ovly gfx",
		/* mode_backcolor	*/ 0x006600ff,
		/* mode_colorkey,msk*/ 0, 0,
		{
			{
			/* xres, yres, xpos, ypos */ 0, 0, 0, 0,
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_16BPP565|LCD_WINCTRL1_PO_16BPP,
			/* mode_winenable*/ LCD_WINENABLE_WEN0,
			},
			{
			/* xres, yres, xpos, ypos */ 0, 0, 0, 0,
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_16BPP565|LCD_WINCTRL1_PO_16BPP,
			/* mode_winenable*/ 0,
			},
			{
			/* xres, yres, xpos, ypos */ 0, 0, 0, 0,
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_16BPP565|LCD_WINCTRL1_PO_16BPP|LCD_WINCTRL1_PIPE,
			/* mode_winenable*/ 0,
			},
			{
			/* xres, yres, xpos, ypos */ 0, 0, 0, 0,
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_16BPP565|LCD_WINCTRL1_PO_16BPP|LCD_WINCTRL1_PIPE,
			/* mode_winenable*/ 0,
			},
		},
	},

	{ /* Index 1 */
		"0-FS gfx, 1-video, 2-ovly gfx, 3-ovly gfx",
		/* mode_backcolor	*/ 0x006600ff,
		/* mode_colorkey,msk*/ 0, 0,
		{
			{
			/* xres, yres, xpos, ypos */ 320, 240, 5, 5,
#if 0
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_16BPP565|LCD_WINCTRL1_PO_16BPP,
#endif
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_24BPP|LCD_WINCTRL1_PO_00,
			/* mode_winenable*/ LCD_WINENABLE_WEN0,
			},
			{
			/* xres, yres, xpos, ypos */ 0, 0, 0, 0,
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_16BPP565|LCD_WINCTRL1_PO_16BPP,
			/* mode_winenable*/ 0,
			},
			{
			/* xres, yres, xpos, ypos */ 100, 100, 0, 0,
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_16BPP565|LCD_WINCTRL1_PO_16BPP|LCD_WINCTRL1_PIPE,
			/* mode_winenable*/ 0/*LCD_WINENABLE_WEN2*/,
			},
			{
			/* xres, yres, xpos, ypos */ 200, 25, 0, 0,
			/* mode_winctrl1 */ LCD_WINCTRL1_FRM_16BPP565|LCD_WINCTRL1_PO_16BPP|LCD_WINCTRL1_PIPE,
			/* mode_winenable*/ 0,
			},
		},
	},
	/* Need VGA 640 @ 24bpp, @ 32bpp */
	/* Need VGA 800 @ 24bpp, @ 32bpp */
	/* Need VGA 1024 @ 24bpp, @ 32bpp */
} ;

/*
 * Controller configurations for various panels.
 */
static struct panel_settings panels[] =
{
	{ /* Index 0: QVGA 320x240 H:33.3kHz V:110Hz */
		"VGA_320x240",
		320, 240,
		/* mode_screen 		*/ LCD_SCREEN_SX_N(320) | LCD_SCREEN_SY_N(240),
		/* mode_horztiming	*/ 0x00c4623b,
		/* mode_verttiming	*/ 0x00502814,
		/* mode_clkcontrol	*/ 0x00020002, /* /4=24Mhz */
		/* mode_pwmdiv		*/ 0x00000000,
		/* mode_pwmhi		*/ 0x00000000,
		/* mode_outmask		*/ 0x00FFFFFF,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* 96MHz AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ NULL,
		/* device_shutdown	*/ NULL,
	},

	{ /* Index 1: VGA 640x480 H:30.3kHz V:58Hz */
		"VGA_640x480",
		640, 480,
		/* mode_screen 		*/ 0x13f9df80,
		/* mode_horztiming	*/ 0x003c5859,
		/* mode_verttiming	*/ 0x00741201,
		/* mode_clkcontrol	*/ 0x00020001, /* /4=24Mhz */
		/* mode_pwmdiv		*/ 0x00000000,
		/* mode_pwmhi		*/ 0x00000000,
		/* mode_outmask		*/ 0x00FFFFFF,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ NULL,
		/* device_shutdown	*/ NULL,
	},

	{ /* Index 2: SVGA 800x600 H:46.1kHz V:69Hz */
		"SVGA_800x600",
		800, 600,
		/* mode_screen 		*/ 0x18fa5780,
		/* mode_horztiming	*/ 0x00dc7e77,
		/* mode_verttiming	*/ 0x00584805,
		/* mode_clkcontrol	*/ 0x00020000, /* /2=48Mhz */
		/* mode_pwmdiv		*/ 0x00000000,
		/* mode_pwmhi		*/ 0x00000000,
		/* mode_outmask		*/ 0x00FFFFFF,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ NULL,
		/* device_shutdown	*/ NULL,
	},

	{ /* Index 3: XVGA 1024x768 H:56.2kHz V:70Hz */
		"XVGA_1024x768",
		1024, 768,
		/* mode_screen 		*/ 0x1ffaff80,
		/* mode_horztiming	*/ 0x007d0e57,
		/* mode_verttiming	*/ 0x00740a01,
		/* mode_clkcontrol	*/ 0x000A0000, /* /1 */
		/* mode_pwmdiv		*/ 0x00000000,
		/* mode_pwmhi		*/ 0x00000000,
		/* mode_outmask		*/ 0x00FFFFFF,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 6, /* 72MHz AUXPLL */
		/* device_init		*/ NULL,
		/* device_shutdown	*/ NULL,
	},

	{ /* Index 4: XVGA 1280x1024 H:68.5kHz V:65Hz */
		"XVGA_1280x1024",
		1280, 1024,
		/* mode_screen 		*/ 0x27fbff80,
		/* mode_horztiming	*/ 0x00cdb2c7,
		/* mode_verttiming	*/ 0x00600002,
		/* mode_clkcontrol	*/ 0x000A0000, /* /1 */
		/* mode_pwmdiv		*/ 0x00000000,
		/* mode_pwmhi		*/ 0x00000000,
		/* mode_outmask		*/ 0x00FFFFFF,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 10, /* 120MHz AUXPLL */
		/* device_init		*/ NULL,
		/* device_shutdown	*/ NULL,
	},

	{ /* Index 5: Samsung 1024x768 TFT */
		"Samsung_1024x768_TFT",
		1024, 768,
		/* mode_screen 		*/ 0x1ffaff80,
		/* mode_horztiming	*/ 0x018cc677,
		/* mode_verttiming	*/ 0x00241217,
		/* mode_clkcontrol	*/ 0x00000000, /* SCB 0x1 /4=24Mhz */
		/* mode_pwmdiv		*/ 0x8000063f, /* SCB 0x0 */
		/* mode_pwmhi		*/ 0x03400000, /* SCB 0x0 */
		/* mode_outmask		*/ 0x00fcfcfc,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* 96MHz AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ board_au1200fb_panel_init,
		/* device_shutdown	*/ board_au1200fb_panel_shutdown,
	},

	{ /* Index 6: Toshiba 640x480 TFT */
		"Toshiba_640x480_TFT",
		640, 480,
		/* mode_screen 		*/ LCD_SCREEN_SX_N(640) | LCD_SCREEN_SY_N(480),
		/* mode_horztiming	*/ LCD_HORZTIMING_HPW_N(96) | LCD_HORZTIMING_HND1_N(13) | LCD_HORZTIMING_HND2_N(51),
		/* mode_verttiming	*/ LCD_VERTTIMING_VPW_N(2) | LCD_VERTTIMING_VND1_N(11) | LCD_VERTTIMING_VND2_N(32) ,
		/* mode_clkcontrol	*/ 0x00000000, /* /4=24Mhz */
		/* mode_pwmdiv		*/ 0x8000063f,
		/* mode_pwmhi		*/ 0x03400000,
		/* mode_outmask		*/ 0x00fcfcfc,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* 96MHz AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ board_au1200fb_panel_init,
		/* device_shutdown	*/ board_au1200fb_panel_shutdown,
	},

	{ /* Index 7: Sharp 320x240 TFT */
		"Sharp_320x240_TFT",
		320, 240,
		/* mode_screen 		*/ LCD_SCREEN_SX_N(320) | LCD_SCREEN_SY_N(240),
		/* mode_horztiming	*/ LCD_HORZTIMING_HPW_N(60) | LCD_HORZTIMING_HND1_N(13) | LCD_HORZTIMING_HND2_N(2),
		/* mode_verttiming	*/ LCD_VERTTIMING_VPW_N(2) | LCD_VERTTIMING_VND1_N(2) | LCD_VERTTIMING_VND2_N(5) ,
		/* mode_clkcontrol	*/ LCD_CLKCONTROL_PCD_N(7), /* /16=6Mhz */
		/* mode_pwmdiv		*/ 0x8000063f,
		/* mode_pwmhi		*/ 0x03400000,
		/* mode_outmask		*/ 0x00fcfcfc,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* 96MHz AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ board_au1200fb_panel_init,
		/* device_shutdown	*/ board_au1200fb_panel_shutdown,
	},
	{ /* Index 8: Toppoly TD070WGCB2 7" 854x480 TFT */
		"Toppoly_TD070WGCB2",
		854, 480,
		/* mode_screen 		*/ LCD_SCREEN_SX_N(854) | LCD_SCREEN_SY_N(480),
		/* mode_horztiming	*/ LCD_HORZTIMING_HND2_N(44) | LCD_HORZTIMING_HND1_N(44) | LCD_HORZTIMING_HPW_N(114),
		/* mode_verttiming	*/ LCD_VERTTIMING_VND2_N(20) | LCD_VERTTIMING_VND1_N(21) | LCD_VERTTIMING_VPW_N(4),
		/* mode_clkcontrol	*/ 0x00020001, /* /4=24Mhz */
		/* mode_pwmdiv		*/ 0x8000063f,
		/* mode_pwmhi		*/ 0x03400000,
		/* mode_outmask		*/ 0x00FCFCFC,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000004, /* AUXPLL directly */
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ board_au1200fb_panel_init,
		/* device_shutdown	*/ board_au1200fb_panel_shutdown,
	},
#if defined(CONFIG_FOCUS_ENHANCEMENTS)
	{ /* Index 9: Focus FS453 TV-Out 640x480 */
		"FS453_640x480 (Composite/S-Video)",
		640, 480,
		/* mode_screen 		*/ LCD_SCREEN_SX_N(640) | LCD_SCREEN_SY_N(480),
		/* mode_horztiming	*/ LCD_HORZTIMING_HND2_N(143) | LCD_HORZTIMING_HND1_N(143) | LCD_HORZTIMING_HPW_N(10),
		/* mode_verttiming	*/ LCD_VERTTIMING_VND2_N(30) | LCD_VERTTIMING_VND1_N(30) | LCD_VERTTIMING_VPW_N(5),
		/* mode_clkcontrol	*/ 0x00480000 | (1<<17) | (1<<18), /* External Clock, 1:1 clock ratio */
		/* mode_pwmdiv		*/ 0x00000000,
		/* mode_pwmhi		*/ 0x00000000,
		/* mode_outmask		*/ 0x00FFFFFF,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000000,
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ board_au1200fb_focus_init_cvsv,
		/* device_shutdown	*/ board_au1200fb_focus_shutdown,
	},
	
	{ /* Index 10: Focus FS453 TV-Out 640x480 */
		"FS453_640x480 (Component Video)",
		640, 480,
		/* mode_screen 		*/ LCD_SCREEN_SX_N(640) | LCD_SCREEN_SY_N(480),
		/* mode_horztiming	*/ LCD_HORZTIMING_HND2_N(143) | LCD_HORZTIMING_HND1_N(143) | LCD_HORZTIMING_HPW_N(10),
		/* mode_verttiming	*/ LCD_VERTTIMING_VND2_N(30) | LCD_VERTTIMING_VND1_N(30) | LCD_VERTTIMING_VPW_N(5),
		/* mode_clkcontrol	*/ 0x00480000 | (1<<17) | (1<<18), /* External Clock, 1:1 clock ratio */
		/* mode_pwmdiv		*/ 0x00000000,
		/* mode_pwmhi		*/ 0x00000000,
		/* mode_outmask		*/ 0x00FFFFFF,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000000,
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ board_au1200fb_focus_init_component,
		/* device_shutdown	*/ board_au1200fb_focus_shutdown,
	},
	
	{ /* Index 11: Focus FS453 TV-Out 640x480 */
		"FS453_640x480 (HDTV)",
		720, 480,
		/* mode_screen 		*/ LCD_SCREEN_SX_N(720) | LCD_SCREEN_SY_N(480),
		/* mode_horztiming	*/ LCD_HORZTIMING_HND2_N(28) | LCD_HORZTIMING_HND1_N(46) | LCD_HORZTIMING_HPW_N(64),
		/* mode_verttiming	*/ LCD_VERTTIMING_VND2_N(7) | LCD_VERTTIMING_VND1_N(31) | LCD_VERTTIMING_VPW_N(7),
		/* mode_clkcontrol	*/ 0x00480000 | (1<<17) | (1<<18), /* External Clock, 1:1 clock ratio */
		/* mode_pwmdiv		*/ 0x00000000,
		/* mode_pwmhi		*/ 0x00000000,
		/* mode_outmask		*/ 0x00FFFFFF,
		/* mode_fifoctrl	*/ 0x2f2f2f2f,
		/* mode_toyclksrc   */ 0x00000000,
		/* mode_backlight   */ 0x00000000,
		/* mode_auxpll		*/ 8, /* 96MHz AUXPLL */
		/* device_init		*/ board_au1200fb_focus_init_hdtv,
		/* device_shutdown	*/ board_au1200fb_focus_shutdown,
	},
#endif
};

#define NUM_PANELS (sizeof(panels) / sizeof(struct panel_settings))

static struct window_settings *win;
static struct panel_settings *panel;

struct au1200fb_info {
	struct fb_info_gen gen;
	unsigned long fb_virt_start;
	unsigned long fb_size;
	unsigned long fb_phys;
	int mmaped;
	int nohwcursor;
	int noblanking;

	struct { unsigned red, green, blue, pad; } palette[256];

#if defined(FBCON_HAS_CFB16)
	u16 fbcon_cmap16[16];
#endif
#if defined(FBCON_HAS_CFB32)
	u32 fbcon_cmap32[16];
#endif
};


struct au1200fb_par {
    struct fb_var_screeninfo var;
	
	int line_length;  /* in bytes */
	int cmap_len;     /* color-map length */
};

#ifndef CONFIG_FB_AU1200_DEVS
#define CONFIG_FB_AU1200_DEVS 1
#endif

static struct au1200fb_info fb_infos[CONFIG_FB_AU1200_DEVS];
static struct au1200fb_par fb_pars[CONFIG_FB_AU1200_DEVS];
static struct display disps[CONFIG_FB_AU1200_DEVS];

int au1200fb_init(void);
void au1200fb_setup(char *options, int *ints);
static int au1200fb_mmap(struct fb_info *fb, struct file *file, 
		struct vm_area_struct *vma);
static int au1200_blank(int blank_mode, struct fb_info_gen *info);
static int au1200fb_ioctl(struct inode *inode, struct file *file, u_int cmd,
			  u_long arg, int con, struct fb_info *info);

void au1200_nocursor(struct display *p, int mode, int xx, int yy){};

static int au1200_setlocation (int plane, int xpos, int ypos);
static int au1200_setsize (int plane, int xres, int yres);
static void au1200_setmode(int plane);
static void au1200_setpanel (struct panel_settings *newpanel);

static struct fb_ops au1200fb_ops = {
	owner:		THIS_MODULE,
	fb_get_fix:	fbgen_get_fix,
	fb_get_var:	fbgen_get_var,
	fb_set_var:	fbgen_set_var,
	fb_get_cmap:	fbgen_get_cmap,
	fb_set_cmap:	fbgen_set_cmap,
	fb_pan_display: fbgen_pan_display,
        fb_ioctl:       au1200fb_ioctl,
	fb_mmap:        au1200fb_mmap,
};


static int
winbpp (unsigned int winctrl1)
{
	/* how many bytes of memory are needed for each pixel format */
	switch (winctrl1 & LCD_WINCTRL1_FRM)
	{
		case LCD_WINCTRL1_FRM_1BPP: return 1; break;
		case LCD_WINCTRL1_FRM_2BPP: return 2; break;
		case LCD_WINCTRL1_FRM_4BPP: return 4; break;
		case LCD_WINCTRL1_FRM_8BPP: return 8; break;
		case LCD_WINCTRL1_FRM_12BPP: return 16; break;
		case LCD_WINCTRL1_FRM_16BPP655: return 16; break;
		case LCD_WINCTRL1_FRM_16BPP565: return 16; break;
		case LCD_WINCTRL1_FRM_16BPP556: return 16; break;
		case LCD_WINCTRL1_FRM_16BPPI1555: return 16; break;
		case LCD_WINCTRL1_FRM_16BPPI5551: return 16; break;
		case LCD_WINCTRL1_FRM_16BPPA1555: return 16; break;
		case LCD_WINCTRL1_FRM_16BPPA5551: return 16; break;
		case LCD_WINCTRL1_FRM_24BPP: return 32; break;
		case LCD_WINCTRL1_FRM_32BPP: return 32; break;
		default: return 0; break;
	}
}

static int
fbinfo2index (struct fb_info *fb_info)
{
	int i;
	for (i = 0; i < CONFIG_FB_AU1200_DEVS; ++i)
	{
		if (fb_info == (struct fb_info *)(&fb_infos[i]))
			return i;
	}
	printk("au1200fb: ERROR: fbinfo2index failed!\n");
	return -1;
}

static void au1200_detect(void)
{
	/*
	 *  This function should detect the current video mode settings 
	 *  and store it as the default video mode
	 * Yeh, well, we're not going to change any settings so we're
	 * always stuck with the default ...
	 */
}

static int au1200_encode_fix(struct fb_fix_screeninfo *fix, 
		const void *_par, struct fb_info_gen *_info)
{
    struct au1200fb_info *info = (struct au1200fb_info *) _info;
    struct au1200fb_par *par = (struct au1200fb_par *) _par;
	int plane;

	plane = fbinfo2index(info);

	memset(fix, 0, sizeof(struct fb_fix_screeninfo));

	fix->smem_start = info->fb_phys;
	fix->smem_len = info->fb_size;
	fix->type = FB_TYPE_PACKED_PIXELS;
	fix->type_aux = 0;
        fix->visual = (par->var.bits_per_pixel == 8) ?
	       	FB_VISUAL_PSEUDOCOLOR	: FB_VISUAL_TRUECOLOR;
	fix->ywrapstep = 0;
	fix->xpanstep = 1;
	fix->ypanstep = 1;
	/* FIX!!!! why doesn't par->line_length work???? it does for au1100 */
	fix->line_length = fb_pars[plane].line_length; /*par->line_length;*/
	return 0;
}

static void set_color_bitfields(struct fb_var_screeninfo *var, int plane)
{
	if (var->bits_per_pixel == 8)
	{
		var->red.offset = 0;
		var->red.length = 8;
		var->green.offset = 0;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
	}
	else
	
	if (var->bits_per_pixel == 16)
	{
		/* FIX!!! How does CCO affect this ? */
		/* FIX!!! Not exactly sure how many of these work with FB */
		switch (win->w[plane].mode_winctrl1 & LCD_WINCTRL1_FRM)
		{
			case LCD_WINCTRL1_FRM_16BPP655:
				var->red.offset = 10;
				var->red.length = 6;
				var->green.offset = 5;
				var->green.length = 5;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 0;
				break;

			case LCD_WINCTRL1_FRM_16BPP565:
				var->red.offset = 11;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 6;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 0;
				break;

			case LCD_WINCTRL1_FRM_16BPP556:
				var->red.offset = 11;
				var->red.length = 5;
				var->green.offset = 6;
				var->green.length = 5;
				var->blue.offset = 0;
				var->blue.length = 6;
				var->transp.offset = 0;
				var->transp.length = 0;
				break;

			case LCD_WINCTRL1_FRM_16BPPI1555:
				var->red.offset = 10;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 5;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 0;
				break;

			case LCD_WINCTRL1_FRM_16BPPI5551:
				var->red.offset = 11;
				var->red.length = 5;
				var->green.offset = 6;
				var->green.length = 5;
				var->blue.offset = 1;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 0;
				break;

			case LCD_WINCTRL1_FRM_16BPPA1555:
				var->red.offset = 10;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 5;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 15;
				var->transp.length = 1;
				break;

			case LCD_WINCTRL1_FRM_16BPPA5551:
				var->red.offset = 11;
				var->red.length = 5;
				var->green.offset = 6;
				var->green.length = 5;
				var->blue.offset = 1;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 1;
				break;

			default:
				printk("ERROR: Invalid PIXEL FORMAT!!!\n"); break;
		}
	}
	else

	if (var->bits_per_pixel == 32)
	{
		switch (win->w[plane].mode_winctrl1 & LCD_WINCTRL1_FRM)
		{
		case LCD_WINCTRL1_FRM_24BPP:
			var->red.offset = 16;
			var->red.length = 8;
			var->green.offset = 8;
			var->green.length = 8;
			var->blue.offset = 0;
			var->blue.length = 8;
			var->transp.offset = 0;
			var->transp.length = 0;
			break;

		case LCD_WINCTRL1_FRM_32BPP:
			var->red.offset = 16;
			var->red.length = 8;
			var->green.offset = 8;
			var->green.length = 8;
			var->blue.offset = 0;
			var->blue.length = 8;
			var->transp.offset = 24;
			var->transp.length = 8;
			break;
		}
	}
	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;
	var->transp.msb_right = 0;
#if 0
printk("set_color_bitfields(a=%d, r=%d..%d, g=%d..%d, b=%d..%d)\n",
	var->transp.offset,
	var->red.offset+var->red.length-1, var->red.offset,
	var->green.offset+var->green.length-1, var->green.offset,
	var->blue.offset+var->blue.length-1, var->blue.offset);
#endif
}

static int au1200_decode_var(const struct fb_var_screeninfo *var, 
		void *_par, struct fb_info_gen *_info)
{
	struct au1200fb_par *par = (struct au1200fb_par *)_par;
	int plane, bpp;

	plane = fbinfo2index((struct fb_info *)_info);

	/*
	 * Don't allow setting any of these yet: xres and yres don't
	 * make sense for LCD panels.
	 */
	if (var->xres != win->w[plane].xres ||
	    var->yres != win->w[plane].yres ||
	    var->xres != win->w[plane].xres ||
	    var->yres != win->w[plane].yres) {
		return -EINVAL;
	}

	bpp = winbpp(win->w[plane].mode_winctrl1);
	if(var->bits_per_pixel != bpp) {
		/* on au1200, window pixel format is independent of panel pixel */
		printk("WARNING: bits_per_pizel != panel->bpp\n");
	}

	memset(par, 0, sizeof(struct au1200fb_par));
	par->var = *var;
	
	/* FIX!!! */
	switch (var->bits_per_pixel) {
		case 8:
			par->var.bits_per_pixel = 8;
			break;
		case 16:
			par->var.bits_per_pixel = 16;
			break;
		case 24:
		case 32:
			par->var.bits_per_pixel = 32;
			break;
		default:
			printk("color depth %d bpp not supported\n",
					var->bits_per_pixel);
			return -EINVAL;

	}
	set_color_bitfields(&par->var, plane);
	/* FIX!!! what is this for 24/32bpp? */
	par->cmap_len = (par->var.bits_per_pixel == 8) ? 256 : 16;
	return 0;
}

static int au1200_encode_var(struct fb_var_screeninfo *var, 
		const void *par, struct fb_info_gen *_info)
{
	*var = ((struct au1200fb_par *)par)->var;
	return 0;
}

static void 
au1200_get_par(void *_par, struct fb_info_gen *_info)
{
	int index;

	index = fbinfo2index((struct fb_info *)_info);
	*(struct au1200fb_par *)_par = fb_pars[index];
}

static void au1200_set_par(const void *par, struct fb_info_gen *info)
{
	/* nothing to do: we don't change any settings */
}

static int au1200_getcolreg(unsigned regno, unsigned *red, unsigned *green,
			 unsigned *blue, unsigned *transp,
			 struct fb_info *info)
{
	struct au1200fb_info* i = (struct au1200fb_info*)info;

	if (regno > 255)
		return 1;
   
	*red    = i->palette[regno].red; 
	*green  = i->palette[regno].green; 
	*blue   = i->palette[regno].blue; 
	*transp = 0;

	return 0;
}

static int au1200_setcolreg(unsigned regno, unsigned red, unsigned green,
			 unsigned blue, unsigned transp,
			 struct fb_info *info)
{
	struct au1200fb_info* i = (struct au1200fb_info *)info;
	u32 rgbcol;
	int plane, bpp;

	plane = fbinfo2index((struct fb_info *)info);
	bpp = winbpp(win->w[plane].mode_winctrl1);

	if (regno > 255)
		return 1;

	i->palette[regno].red    = red;
	i->palette[regno].green  = green;
	i->palette[regno].blue   = blue;
   
	switch(bpp) {
#ifdef FBCON_HAS_CFB8
	case 8:
		red >>= 10;
		green >>= 10;
		blue >>= 10;
		panel_reg->lcd_pallettebase[regno] = (blue&0x1f) | 
			((green&0x3f)<<5) | ((red&0x1f)<<11);
		break;
#endif
#ifdef FBCON_HAS_CFB16
/* FIX!!!! depends upon pixel format */
	case 16:
		i->fbcon_cmap16[regno] =
			((red & 0xf800) >> 0) |
			((green & 0xfc00) >> 5) |
			((blue & 0xf800) >> 11);
		break;
#endif
#ifdef FBCON_HAS_CFB32
	case 32:
		i->fbcon_cmap32[regno] =
            (((u32 )transp & 0xff00) << 16) |
            (((u32 )red & 0xff00) << 8) |
            (((u32 )green & 0xff00)) |
            (((u32 )blue & 0xff00) >> 8);
		break;
#endif
	default:
	printk("unsupported au1200_setcolreg(%d)\n", bpp);
		break;
	}

	return 0;
}


static int  au1200_blank(int blank_mode, struct fb_info_gen *_info)
{
	struct au1200fb_info *fb_info = (struct au1200fb_info *)_info;
	int plane;

	/* Short-circuit screen blanking */
	if (fb_info->noblanking)
		return 0;

	plane = fbinfo2index((struct fb_info *)_info);

	switch (blank_mode) {
	case VESA_NO_BLANKING:
		/* printk("turn on panel\n"); */
		au1200_setpanel(panel);
		break;

	case VESA_VSYNC_SUSPEND:
	case VESA_HSYNC_SUSPEND:
	case VESA_POWERDOWN:
		/* printk("turn off panel\n"); */
		au1200_setpanel(NULL);
        break;
	default: 
		break;

	}
	return 0;
}

static void au1200_set_disp(const void *unused, struct display *disp,
			 struct fb_info_gen *info)
{
	struct au1200fb_info *fb_info;
	int plane;

	fb_info = (struct au1200fb_info *)info;

	disp->screen_base = (char *)fb_info->fb_virt_start;

	switch (disp->var.bits_per_pixel) {
#ifdef FBCON_HAS_CFB8
	case 8:
		disp->dispsw = &fbcon_cfb8;
		if (fb_info->nohwcursor)
			fbcon_cfb8.cursor = au1200_nocursor;
		break;
#endif
#ifdef FBCON_HAS_CFB16
	case 16:
		disp->dispsw = &fbcon_cfb16;
		disp->dispsw_data = fb_info->fbcon_cmap16;
		if (fb_info->nohwcursor)
			fbcon_cfb16.cursor = au1200_nocursor;
		break;
#endif
#ifdef FBCON_HAS_CFB32
	case 32:
		disp->dispsw = &fbcon_cfb32;
		disp->dispsw_data = fb_info->fbcon_cmap32;
		if (fb_info->nohwcursor)
			fbcon_cfb32.cursor = au1200_nocursor;
		break;
#endif
	default:
		disp->dispsw = &fbcon_dummy;
		disp->dispsw_data = NULL;
		break;
	}
}

static int
au1200fb_mmap(struct fb_info *_fb,
	     struct file *file,
	     struct vm_area_struct *vma)
{
	unsigned int len;
	unsigned long start=0, off;

	struct au1200fb_info *fb_info = (struct au1200fb_info *)_fb;

	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) {
		return -EINVAL;
	}
    
	start = fb_info->fb_phys & PAGE_MASK;
	len = PAGE_ALIGN((start & ~PAGE_MASK) + fb_info->fb_size);

	off = vma->vm_pgoff << PAGE_SHIFT;

	if ((vma->vm_end - vma->vm_start + off) > len) {
		return -EINVAL;
	}

	off += start;
	vma->vm_pgoff = off >> PAGE_SHIFT;

	pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
	pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;

	/* This is an IO map - tell maydump to skip this VMA */
	vma->vm_flags |= VM_IO;
    
	if (io_remap_page_range(vma->vm_start, off,
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot)) {
		return -EAGAIN;
	}

	fb_info->mmaped = 1;
	return 0;
}

int au1200_pan_display(const struct fb_var_screeninfo *var,
		       struct fb_info_gen *info)
{
	return 0;
}


static int au1200fb_ioctl(struct inode *inode, struct file *file, u_int cmd,
			  u_long arg, int con, struct fb_info *info)
{
	int plane;

	plane = fbinfo2index(info);

	/* printk("au1200fb: ioctl %d on plane %d\n", cmd, plane); */

	if (cmd == 0x46FF)
	{
		au1200_lcd_getset_t iodata;

		if (copy_from_user(&iodata, (void *) arg, sizeof(au1200_lcd_getset_t)))
			return -EFAULT;

		switch (iodata.subcmd)
		{
			case AU1200_LCD_GET_WINENABLE:
				iodata.winenable.enable = (lcd->winenable & (1<<plane)) ? 1 : 0;
				break;
			case AU1200_LCD_SET_WINENABLE:
				{
				u32 winenable;
				winenable = lcd->winenable;
				winenable &= ~(1<<plane);
				winenable |= (iodata.winenable.enable) ? (1<<plane) : 0;
				lcd->winenable = winenable;
				}
				break;
			case AU1200_LCD_GET_WINLOCATION:
				iodata.winlocation.x =
					(lcd->window[plane].winctrl0 & LCD_WINCTRL0_OX) >> 21;
				iodata.winlocation.y =
					(lcd->window[plane].winctrl0 & LCD_WINCTRL0_OY) >> 10;
				break;
			case AU1200_LCD_SET_WINLOCATION:
				au1200_setlocation(plane, iodata.winlocation.x, iodata.winlocation.y);
				break;
			case AU1200_LCD_GET_WINSIZE:
				iodata.winsize.hsz =
					(lcd->window[plane].winctrl1 & LCD_WINCTRL1_SZX) >> 11;
				iodata.winsize.vsz =
					(lcd->window[plane].winctrl0 & LCD_WINCTRL1_SZY) >> 0;
				break;
			case AU1200_LCD_SET_WINSIZE:
				au1200_setsize(plane, iodata.winsize.hsz, iodata.winsize.vsz);
				break;
			case AU1200_LCD_GET_BACKCOLOR:
				iodata.backcolor.color = lcd->backcolor;
				break;
			case AU1200_LCD_SET_BACKCOLOR:
				 lcd->backcolor = iodata.backcolor.color;
				break;
			case AU1200_LCD_GET_COLORKEY:
				iodata.colorkey.key = lcd->colorkey;
				iodata.colorkey.mask = lcd->colorkeymsk;
				break;
			case AU1200_LCD_SET_COLORKEY:
				lcd->colorkey = iodata.colorkey.key;
				lcd->colorkeymsk = iodata.colorkey.mask;
				break;
			case AU1200_LCD_GET_PANEL:
				iodata.panel.panel = panel_index;
				break;
			case AU1200_LCD_SET_PANEL:
				if ((iodata.panel.panel >= 0) && (iodata.panel.panel < NUM_PANELS))
				{
					struct panel_settings *newpanel;
					panel_index = iodata.panel.panel;
					newpanel = &panels[panel_index];
					au1200_setpanel(newpanel);
				}
				break;
		}

		return copy_to_user((void *) arg, &iodata, sizeof(au1200_lcd_getset_t)) ? -EFAULT : 0;
	}

	return -EINVAL;
}

static struct fbgen_hwswitch au1200_switch = {
	au1200_detect, 
	au1200_encode_fix, 
	au1200_decode_var, 
	au1200_encode_var, 
	au1200_get_par, 
	au1200_set_par, 
	au1200_getcolreg, 
	au1200_setcolreg, 
	au1200_pan_display, 
	au1200_blank, 
	au1200_set_disp
};

static void au1200_setpanel (struct panel_settings *newpanel)
{
	/*
	 * Perform global setup/init of LCD controller
	 */
	uint32 winenable;

	/* Make sure all windows disabled */
	winenable = lcd->winenable;
	lcd->winenable = 0;

	/*
	 * Ensure everything is disabled before reconfiguring
	 */
	if (lcd->screen & LCD_SCREEN_SEN)
	{
		/* Wait for vertical sync period */
		lcd->intstatus = LCD_INT_SS;
		while ((lcd->intstatus & LCD_INT_SS) == 0)
			;		
		
		lcd->screen &= ~LCD_SCREEN_SEN;	/*disable the controller*/
		
		do
		{
			lcd->intstatus = lcd->intstatus; /*clear interrupts*/
		}
		/*wait for controller to shut down*/
		while ((lcd->intstatus & LCD_INT_SD) == 0);
		
		/* Call shutdown of current panel (if up) */
		/* this must occur last, because if an external clock is driving
		    the controller, the clock cannot be turned off before first
			shutting down the controller.
		 */
		if (panel->device_shutdown != NULL) panel->device_shutdown();
	}

	/* Check if only needing to turn off panel */
	if (panel == NULL) return;

	panel = newpanel;
	
	printk("Panel(%s), %dx%d\n", panel->name, panel->Xres, panel->Yres);

	/*
	 * Setup clocking if internal LCD clock source (assumes sys_auxpll valid)
	 */
	if (!(panel->mode_clkcontrol & LCD_CLKCONTROL_EXT))
	{
		uint32 sys_clksrc;
		/* WARNING! This should really be a check since other peripherals can 
		   be affected by changins sys_auxpll  */
		au_writel(panel->mode_auxpll, SYS_AUXPLL);
		sys_clksrc = au_readl(SYS_CLKSRC) & ~0x0000001f; 
		sys_clksrc |= panel->mode_toyclksrc;
		au_writel(sys_clksrc, SYS_CLKSRC);
	}

	/*
	 * Configure panel timings
	 */
	lcd->screen = panel->mode_screen;
	lcd->horztiming = panel->mode_horztiming;
	lcd->verttiming = panel->mode_verttiming;
	lcd->clkcontrol = panel->mode_clkcontrol;
	lcd->pwmdiv = panel->mode_pwmdiv;
	lcd->pwmhi = panel->mode_pwmhi;
	lcd->outmask = panel->mode_outmask;
	lcd->fifoctrl = panel->mode_fifoctrl;
	au_sync();

	/* FIX!!! Check window settings to make sure still valid for new geometry */
	au1200_setlocation(0, win->w[0].xpos, win->w[0].ypos);
	au1200_setlocation(1, win->w[1].xpos, win->w[1].ypos);
	au1200_setlocation(2, win->w[2].xpos, win->w[2].ypos);
	au1200_setlocation(3, win->w[3].xpos, win->w[3].ypos);
	lcd->winenable = winenable;

	/*
	 * Re-enable screen now that it is configured
	 */
	lcd->screen |= LCD_SCREEN_SEN;
	au_sync();

	/* Call init of panel */
	if (panel->device_init != NULL) panel->device_init();

#if 0
#define D(X) printk("%25s: %08X\n", #X, X)
	D(lcd->screen);
	D(lcd->horztiming);
	D(lcd->verttiming);
	D(lcd->clkcontrol);
	D(lcd->pwmdiv);
	D(lcd->pwmhi);
	D(lcd->outmask);
	D(lcd->fifoctrl);
	D(lcd->window[0].winctrl0);
	D(lcd->window[0].winctrl1);
	D(lcd->window[0].winctrl2);
	D(lcd->window[0].winbuf0);
	D(lcd->window[0].winbuf1);
	D(lcd->window[0].winbufctrl);
	D(lcd->window[1].winctrl0);
	D(lcd->window[1].winctrl1);
	D(lcd->window[1].winctrl2);
	D(lcd->window[1].winbuf0);
	D(lcd->window[1].winbuf1);
	D(lcd->window[1].winbufctrl);
	D(lcd->window[2].winctrl0);
	D(lcd->window[2].winctrl1);
	D(lcd->window[2].winctrl2);
	D(lcd->window[2].winbuf0);
	D(lcd->window[2].winbuf1);
	D(lcd->window[2].winbufctrl);
	D(lcd->window[3].winctrl0);
	D(lcd->window[3].winctrl1);
	D(lcd->window[3].winctrl2);
	D(lcd->window[3].winbuf0);
	D(lcd->window[3].winbuf1);
	D(lcd->window[3].winbufctrl);
	D(lcd->winenable);
	D(lcd->intenable);
	D(lcd->intstatus);
	D(lcd->backcolor);
	D(lcd->winenable);
	D(lcd->colorkey);
    D(lcd->colorkeymsk);
	D(lcd->hwc.cursorctrl);
	D(lcd->hwc.cursorpos);
	D(lcd->hwc.cursorcolor0);
	D(lcd->hwc.cursorcolor1);
	D(lcd->hwc.cursorcolor2);
	D(lcd->hwc.cursorcolor3);
#endif
}

static int au1200_setsize (int plane, int xres, int yres)
{
#if 0
	uint32 winctrl0, winctrl1, winenable;
	int xsz, ysz;

	/* FIX!!! X*Y can not surpass allocated memory */

	printk("setsize: x %d y %d\n", xres, yres);
	winctrl1 = lcd->window[plane].winctrl1;
	printk("org winctrl1 %08X\n", winctrl1);
	winctrl1 &= ~(LCD_WINCTRL1_SZX | LCD_WINCTRL1_SZY);

	xres -= 1;
	yres -= 1;
	winctrl1 |= (xres << 11);
	winctrl1 |= (yres << 0);

	printk("new winctrl1 %08X\n", winctrl1);

	/*winenable = lcd->winenable & (1 << plane); */
	/*lcd->winenable &= ~(1 << plane); */
	lcd->window[plane].winctrl1 = winctrl1;
	/*lcd->winenable |= winenable; */
#endif
	return 0;
}

static int au1200_setlocation (int plane, int xpos, int ypos)
{
	uint32 winctrl0, winctrl1, winenable, fb_offset = 0;
	int xsz, ysz;

	/* FIX!!! NOT CHECKING FOR COMPLETE OFFSCREEN YET */

	winctrl0 = lcd->window[plane].winctrl0;
	winctrl1 = lcd->window[plane].winctrl1;
	winctrl0 &= (LCD_WINCTRL0_A | LCD_WINCTRL0_AEN);
	winctrl1 &= ~(LCD_WINCTRL1_SZX | LCD_WINCTRL1_SZY);

	/* Check for off-screen adjustments */
	xsz = win->w[plane].xres;
	ysz = win->w[plane].yres;
	if ((xpos + win->w[plane].xres) > panel->Xres)
	{
		/* Off-screen to the right */
		xsz = panel->Xres - xpos; /* off by 1 ??? */
		/*printk("off screen right\n");*/
	}

	if ((ypos + win->w[plane].yres) > panel->Yres)
	{
		/* Off-screen to the bottom */
		ysz = panel->Yres - ypos; /* off by 1 ??? */
		/*printk("off screen bottom\n");*/
	}

	if (xpos < 0)
	{
		/* Off-screen to the left */
		xsz = win->w[plane].xres + xpos;
		fb_offset += (((0 - xpos) * winbpp(lcd->window[plane].winctrl1))/8);
		xpos = 0;
		/*printk("off screen left\n");*/
	}

	if (ypos < 0)
	{
		/* Off-screen to the top */
		ysz = win->w[plane].yres + ypos;
		fb_offset += ((0 - ypos) *	fb_pars[plane].line_length);
		ypos = 0;
		/*printk("off screen top\n");*/
	}

	/* record settings */
	win->w[plane].xpos = xpos;
	win->w[plane].ypos = ypos;

	xsz -= 1;
	ysz -= 1;
	winctrl0 |= (xpos << 21);
	winctrl0 |= (ypos << 10);
	winctrl1 |= (xsz << 11);
	winctrl1 |= (ysz << 0);

	/* Disable the window while making changes, then restore WINEN */
	winenable = lcd->winenable & (1 << plane);
	lcd->winenable &= ~(1 << plane);
	lcd->window[plane].winctrl0 = winctrl0;
	lcd->window[plane].winctrl1 = winctrl1;
	lcd->window[plane].winbuf0 =
	lcd->window[plane].winbuf1 = fb_infos[plane].fb_phys + fb_offset;
	lcd->window[plane].winbufctrl = 0; /* select winbuf0 */
	lcd->winenable |= winenable;

	return 0;
}

static void au1200_setmode(int plane)
{
	/* Window/plane setup */
	lcd->window[plane].winctrl1 = ( 0
		| LCD_WINCTRL1_PRI_N(plane)
		| win->w[plane].mode_winctrl1 /* FRM,CCO,PO,PIPE */
		) ;

	au1200_setlocation(plane, win->w[plane].xpos, win->w[plane].ypos);

	lcd->window[plane].winctrl2 = ( 0
		| LCD_WINCTRL2_CKMODE_00
		| LCD_WINCTRL2_DBM
/*			| LCD_WINCTRL2_RAM */
		| LCD_WINCTRL2_BX_N(fb_pars[plane].line_length)
		| LCD_WINCTRL2_SCX_1
		| LCD_WINCTRL2_SCY_1
		) ;
	lcd->winenable |= win->w[plane].mode_winenable;
	au_sync();

}

static unsigned long
au1200fb_alloc_fbmem (unsigned long size)
{
	/* __get_free_pages() fulfills a max request of 2MB */
	/* do multiple requests to obtain large contigous mem */
#define MAX_GFP 0x00200000

	unsigned long mem, amem, alloced = 0, allocsize;

	size += 0x1000;
	allocsize = (size < MAX_GFP) ? size : MAX_GFP;

	/* Get first chunk */
	mem = (unsigned long )
		__get_free_pages(GFP_ATOMIC | GFP_DMA, get_order(allocsize));
	if (mem != 0) alloced = allocsize;

	/* Get remaining, contiguous chunks */
	while (alloced < size)
	{
		amem = (unsigned long )
			__get_free_pages(GFP_ATOMIC | GFP_DMA, get_order(allocsize));
		if (amem != 0)
			alloced += allocsize;

		/* check for contiguous mem alloced */
		if ((amem == 0) || (amem + allocsize) != mem)
			break;
		else
			mem = amem;
	}
	return mem;
}

int __init au1200fb_init(void)
{
	int num_panels = sizeof(panels)/sizeof(struct panel_settings);
	struct au1200fb_info *fb_info;
	struct display *disp;
	struct au1200fb_par *par;
	unsigned long page;
	int plane, bpp;

    /*
	* Get the panel information/display mode
	*/
	if (panel_index < 0)
		panel_index = board_au1200fb_panel();
	if ((panel_index < 0) || (panel_index >= num_panels)) {
		printk("ERROR: INVALID PANEL %d\n", panel_index);
		return -EINVAL;
	}
	panel = &panels[panel_index];
	win = &windows[window_index];

	printk("au1200fb: Panel %d %s\n", panel_index, panel->name);
	printk("au1200fb: Win %d %s\n", window_index, win->name);

	/* Global setup/init */
	au1200_setpanel(panel);
	lcd->intenable = 0;
	lcd->intstatus = ~0;
	lcd->backcolor = win->mode_backcolor;
	lcd->winenable = 0;

	/* Setup Color Key - FIX!!! */
	lcd->colorkey = win->mode_colorkey;
	lcd->colorkeymsk = win->mode_colorkeymsk;

	/* Setup HWCursor - FIX!!! Need to support this eventually */
	lcd->hwc.cursorctrl = 0;
	lcd->hwc.cursorpos = 0;
	lcd->hwc.cursorcolor0 = 0;
	lcd->hwc.cursorcolor1 = 0;
	lcd->hwc.cursorcolor2 = 0;
	lcd->hwc.cursorcolor3 = 0;

	/* Register each plane as a frame buffer device */
	for (plane = 0; plane < CONFIG_FB_AU1200_DEVS; ++plane)
	{
		fb_info = &fb_infos[plane];
		disp = &disps[plane];
		par = &fb_pars[plane];

		bpp = winbpp(win->w[plane].mode_winctrl1);
		if (win->w[plane].xres == 0)
			win->w[plane].xres = panel->Xres;
		if (win->w[plane].yres == 0)
			win->w[plane].yres = panel->Yres;

		par->var.xres =
		par->var.xres_virtual = win->w[plane].xres;
		par->var.yres =
		par->var.yres_virtual = win->w[plane].yres;
		par->var.bits_per_pixel = bpp;
		par->line_length = win->w[plane].xres * bpp / 8; /* in bytes */
		/*
		 * Allocate LCD framebuffer from system memory
		 * Set page reserved so that mmap will work. This is necessary
		 * since we'll be remapping normal memory.
		 */
		fb_info->fb_size = (win->w[plane].xres * win->w[plane].yres * bpp) / 8;
		fb_info->fb_virt_start = au1200fb_alloc_fbmem(fb_info->fb_size);
		if (!fb_info->fb_virt_start) {
			printk("Unable to allocate fb memory\n");
			return -ENOMEM;
		}
		fb_info->fb_phys = virt_to_bus((void *)fb_info->fb_virt_start);
		for (page = fb_info->fb_virt_start;
		     page < PAGE_ALIGN(fb_info->fb_virt_start + fb_info->fb_size); 
		     page += PAGE_SIZE) {
			SetPageReserved(virt_to_page(page));
		}
		/* Convert to kseg1 */
		fb_info->fb_virt_start =
			(void *)((u32)fb_info->fb_virt_start | 0xA0000000);
		/* FIX!!! may wish to avoid this to save startup time??? */
		memset((void *)fb_info->fb_virt_start, 0, fb_info->fb_size);

		fb_info->gen.parsize = sizeof(struct au1200fb_par);
		fb_info->gen.fbhw = &au1200_switch;
		strcpy(fb_info->gen.info.modename, "Au1200 LCD");
		fb_info->gen.info.changevar = NULL;
		fb_info->gen.info.node = -1;

		fb_info->gen.info.fbops = &au1200fb_ops;
		fb_info->gen.info.disp = disp;
		fb_info->gen.info.switch_con = &fbgen_switch;
		fb_info->gen.info.updatevar = &fbgen_update_var;
		fb_info->gen.info.blank = &fbgen_blank;
		fb_info->gen.info.flags = FBINFO_FLAG_DEFAULT;

		fb_info->nohwcursor = 1;
		fb_info->noblanking = 1;

		/* This should give a reasonable default video mode */
		fbgen_get_var(&disp->var, -1, &fb_info->gen.info);
		fbgen_do_set_var(&disp->var, 1, &fb_info->gen);
		fbgen_set_disp(-1, &fb_info->gen);
		fbgen_install_cmap(0, &fb_info->gen);

		/* Turn on plane */
		au1200_setmode(plane);

		if (register_framebuffer(&fb_info->gen.info) < 0)
			return -EINVAL;

		printk(KERN_INFO "fb%d: %s plane %d @ %08X (%d x %d x %d)\n", 
				GET_FB_IDX(fb_info->gen.info.node), 
				fb_info->gen.info.modename, plane, fb_info->fb_phys,
				win->w[plane].xres, win->w[plane].yres, bpp);
	}
	/* uncomment this if your driver cannot be unloaded */
	/* MOD_INC_USE_COUNT; */
	return 0;
}

void au1200fb_setup(char *options, int *ints)
{
	char* this_opt;
	int i;
	int num_panels = sizeof(panels)/sizeof(struct panel_settings);

	if (!options || !*options)
		return;

	for(this_opt=strtok(options, ","); this_opt;
	    this_opt=strtok(NULL, ",")) {
		if (!strncmp(this_opt, "panel:", 6)) {
			int i;
			long int li;
			char *endptr;
			this_opt += 6;

			/* Panel name can be name, "bs" for board-switch, or number/index */
			li = simple_strtol(this_opt, &endptr, 0);
			if (*endptr == '\0') {
				panel_index = (int)li;
			}
			else if (strcmp(this_opt, "bs") == 0) {
				panel_index = board_au1200fb_panel();
			}
			else
			for (i=0; i<num_panels; i++) {
				if (!strcmp(this_opt, panels[i].name)) {
					panel_index = i;
					break;
				}
			}
		}
		else if (!strncmp(this_opt, "nohwcursor", 10)) {
			printk("nohwcursor\n");
			fb_infos[0].nohwcursor = 1;
		}
	}

	printk("au1200fb: Panel %d %s\n", panel_index,
		panels[panel_index].name);
}



#ifdef MODULE
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Au1200 LCD framebuffer driver");

void au1200fb_cleanup(struct fb_info *info)
{
	unregister_framebuffer(info);
}

module_init(au1200fb_init);
module_exit(au1200fb_cleanup);
#endif /* MODULE */



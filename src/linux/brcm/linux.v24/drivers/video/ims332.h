/*
 *	linux/drivers/video/ims332.h
 *
 *	Copyright 2003  Thiemo Seufer <seufer@csv.ica.uni-stuttgart.de>
 *
 *	This file is subject to the terms and conditions of the GNU General
 *	Public License. See the file COPYING in the main directory of this
 *	archive for more details.
 */
#include <linux/types.h>

/*
 * IMS332 16-bit wide, 128-bit aligned registers.
 */
struct _ims332_reg {
	volatile u16 r;
	u16 pad[7];
};

struct _ims332_regs {
#define IMS332_BOOT_PLL_MUTLIPLIER	0x00001f
#define IMS332_BOOT_CLOCK_SOURCE_SEL	0x000020
#define IMS332_BOOT_ADDRESS_ALIGNMENT	0x000040
#define IMS332_BOOT_WRITE_ZERO		0xffff80
	struct _ims332_reg boot;
	struct _ims332_reg pad0[0x020 - 0x000];
	struct _ims332_reg half_sync;
	struct _ims332_reg back_porch;
	struct _ims332_reg display;
	struct _ims332_reg short_display;
	struct _ims332_reg broad_pulse;
	struct _ims332_reg vsync;
	struct _ims332_reg vpre_equalise;
	struct _ims332_reg vpost_equalise;
	struct _ims332_reg vblank;
	struct _ims332_reg vdisplay;
	struct _ims332_reg line_time;
	struct _ims332_reg line_start;
	struct _ims332_reg mem_init;
	struct _ims332_reg transfer_delay;
	struct _ims332_reg pad1[0x03f - 0x02e];
	struct _ims332_reg pixel_address_mask;
	struct _ims332_reg pad2[0x05f - 0x040];

#define IMS332_CTRL_A_BOOT_ENABLE_VTG		0x000001
#define IMS332_CTRL_A_SCREEN_FORMAT		0x000002
#define IMS332_CTRL_A_INTERLACED_STANDARD	0x000004
#define IMS332_CTRL_A_OPERATING_MODE		0x000008
#define IMS332_CTRL_A_FRAME_FLYBACK_PATTERN	0x000010
#define IMS332_CTRL_A_DIGITAL_SYNC_FORMAT	0x000020
#define IMS332_CTRL_A_ANALOGUE_VIDEO_FORMAT	0x000040
#define IMS332_CTRL_A_BLANK_LEVEL		0x000080
#define IMS332_CTRL_A_BLANK_IO			0x000100
#define IMS332_CTRL_A_BLANK_FUNCTION_SWITCH	0x000200
#define IMS332_CTRL_A_FORCE_BLANKING		0x000400
#define IMS332_CTRL_A_TURN_OFF_BLANKING		0x000800
#define IMS332_CTRL_A_VRAM_ADDRESS_INCREMENT	0x003000
#define IMS332_CTRL_A_TURN_OFF_DMA		0x004000
#define IMS332_CTRL_A_SYNC_DELAY		0x038000
#define IMS332_CTRL_A_PIXEL_PORT_INTERLEAVING	0x040000
#define IMS332_CTRL_A_DELAYED_SAMPLING		0x080000
#define IMS332_CTRL_A_BITS_PER_PIXEL		0x700000
#define IMS332_CTRL_A_CURSOR_DISABLE		0x800000
	struct _ims332_reg config_control_a;
	struct _ims332_reg pad3[0x06f - 0x060];

#define IMS332_CTRL_B_WRITE_ZERO	0xffffff
	struct _ims332_reg config_control_b;
	struct _ims332_reg pad4[0x07f - 0x070];
	struct _ims332_reg screen_top;
	struct _ims332_reg pad5[0x0a0 - 0x080];
	/* cursor color palette, 3 entries, reg no. 0xa1 - 0xa3 */
	struct _ims332_reg cursor_color_palette0;
	struct _ims332_reg cursor_color_palette1;
	struct _ims332_reg cursor_color_palette2;
	struct _ims332_reg pad6[0x0bf - 0x0a3];
	struct _ims332_reg rgb_frame_checksum0;
	struct _ims332_reg rgb_frame_checksum1;
	struct _ims332_reg rgb_frame_checksum2;
	struct _ims332_reg pad7[0x0c6 - 0x0c2];
	struct _ims332_reg cursor_start;
	struct _ims332_reg pad8[0x0ff - 0x0c7];
	/* color palette, 256 entries of form 0x00BBGGRR, reg no. 0x100 - 0x1ff */
	struct _ims332_reg color_palette[0x1ff - 0x0ff];
	/* hardware cursor bitmap, reg no. 0x200 - 0x3ff */
	struct _ims332_reg cursor_ram[0x3ff - 0x1ff];
};

/*
 * In the functions below we use some weird looking helper variables to
 * access most members of this struct, otherwise the compiler splits
 * the read/write in two byte accesses.
 */
struct ims332_regs {
	struct _ims332_regs rw;
	char pad0[0x80000 - sizeof (struct _ims332_regs)];
	struct _ims332_regs r;
	char pad1[0xa0000 - (sizeof (struct _ims332_regs) + 0x80000)];
	struct _ims332_regs w;
} __attribute__((packed));

static inline void ims332_control_reg_bits(struct ims332_regs *regs, u32 mask,
					   u32 val)
{
	volatile u16 *ctr = &(regs->r.config_control_a.r);
	volatile u16 *ctw = &(regs->w.config_control_a.r);
	u32 ctrl;

	mb();
	ctrl = *ctr;
	rmb();
	ctrl |= ((regs->rw.boot.r << 8) & 0x00ff0000);
	ctrl |= val & mask;
	ctrl &= ~(~val & mask);
	wmb();
	regs->rw.boot.r = (ctrl >> 8) & 0xff00;
	wmb();
	*ctw = ctrl & 0xffff;
}

/* FIXME: This is maxinefb specific. */
static inline void ims332_bootstrap(struct ims332_regs *regs)
{
	volatile u16 *ctw = &(regs->w.config_control_a.r);
	u32 ctrl = IMS332_CTRL_A_BOOT_ENABLE_VTG | IMS332_CTRL_A_TURN_OFF_DMA;

	/* bootstrap sequence */
	mb();
	regs->rw.boot.r = 0;
	wmb();
	*ctw = 0;

	/* init control A register */
	wmb();
	regs->rw.boot.r = (ctrl >> 8) & 0xff00;
	wmb();
	*ctw = ctrl & 0xffff;
}

static inline void ims332_blank_screen(struct ims332_regs *regs, int blank)
{
	ims332_control_reg_bits(regs, IMS332_CTRL_A_FORCE_BLANKING,
				blank ? IMS332_CTRL_A_FORCE_BLANKING : 0);
}

static inline void ims332_set_color_depth(struct ims332_regs *regs, u32 depth)
{
	u32 dp;
	u32 mask = (IMS332_CTRL_A_PIXEL_PORT_INTERLEAVING
		    | IMS332_CTRL_A_DELAYED_SAMPLING
		    | IMS332_CTRL_A_BITS_PER_PIXEL);

	switch (depth) {
	case 1: dp = 0 << 20; break;
	case 2: dp = 1 << 20; break;
	case 4: dp = 2 << 20; break;
	case 8: dp = 3 << 20; break;
	case 15: dp = (4 << 20) | IMS332_CTRL_A_PIXEL_PORT_INTERLEAVING; break;
	case 16: dp = (5 << 20) | IMS332_CTRL_A_PIXEL_PORT_INTERLEAVING; break;
	default: return;
	}
	ims332_control_reg_bits(regs, mask, dp);

	if (depth <= 8) {
		volatile u16 *pmask = &(regs->w.pixel_address_mask.r);
		u32 dm = (1 << depth) - 1;

		wmb();
		regs->rw.boot.r = dm << 8;
		wmb();
		*pmask = dm << 8 | dm;
	}
}

static inline void ims332_set_screen_top(struct ims332_regs *regs, u16 top)
{
	volatile u16 *st = &(regs->w.screen_top.r);

	mb();
	*st = top & 0xffff;
}

static inline void ims332_enable_cursor(struct ims332_regs *regs, int on)
{
	ims332_control_reg_bits(regs, IMS332_CTRL_A_CURSOR_DISABLE,
				on ? 0 : IMS332_CTRL_A_CURSOR_DISABLE);
}

static inline void ims332_position_cursor(struct ims332_regs *regs,
					  u16 x, u16 y)
{
	volatile u16 *cp = &(regs->w.cursor_start.r);
	u32 val = ((x & 0xfff) << 12) | (y & 0xfff);

	if (x > 2303 || y > 2303)
		return;

	mb();
	regs->rw.boot.r = (val >> 8) & 0xff00;
	wmb();
	*cp = val & 0xffff;
}

static inline void ims332_set_font(struct ims332_regs *regs, u8 fgc,
				   u16 width, u16 height)
{
	volatile u16 *cp0 = &(regs->w.cursor_color_palette0.r);
	int i;

	mb();
	for (i = 0; i < 0x200; i++) {
		volatile u16 *cram = &(regs->w.cursor_ram[i].r);

		if (height << 6 <= i << 3)
			*cram = 0x0000;
		else if (width <= i % 8 << 3)
			*cram = 0x0000;
		else if (((width >> 3) & 0xffff) > i % 8)
			*cram = 0x5555;
		else
			*cram = 0x5555 & ~(0xffff << (width % 8 << 1));
		wmb();
	}
	regs->rw.boot.r = fgc << 8;
	wmb();
	*cp0 = fgc << 8 | fgc;
}

static inline void ims332_read_cmap(struct ims332_regs *regs, u8 reg,
				    u8* red, u8* green, u8* blue)
{
	volatile u16 *rptr = &(regs->r.color_palette[reg].r);
	u16 val;

	mb();
	val = *rptr;
	*red = val & 0xff;
	*green = (val >> 8) & 0xff;
	rmb();
	*blue = (regs->rw.boot.r >> 8) & 0xff;
}

static inline void ims332_write_cmap(struct ims332_regs *regs, u8 reg,
				     u8 red, u8 green, u8 blue)
{
	volatile u16 *wptr = &(regs->w.color_palette[reg].r);

	mb();
	regs->rw.boot.r = blue << 8;
	wmb();
	*wptr = (green << 8) + red;
}

static inline void ims332_dump_regs(struct ims332_regs *regs)
{
	int i;

	printk(__FUNCTION__);
	ims332_control_reg_bits(regs, IMS332_CTRL_A_BOOT_ENABLE_VTG, 0);
	for (i = 0; i < 0x100; i++) {
		volatile u16 *cpad = (u16 *)((char *)(&regs->r) + sizeof(struct _ims332_reg) * i);
		u32 val;

		val = *cpad;
		rmb();
		val |= regs->rw.boot.r << 8;
		rmb();
		if (! (i % 8))
			printk("\n%02x:", i);
		printk(" %06x", val);
	}
	ims332_control_reg_bits(regs, IMS332_CTRL_A_BOOT_ENABLE_VTG,
				IMS332_CTRL_A_BOOT_ENABLE_VTG);
	printk("\n");
}

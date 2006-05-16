/*
 * fb.c - Frame-buffer driver for Nokia 3310 LCD
 *
 */

#include <linux/config.h>
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
#include <linux/ioctl.h>
#include <linux/proc_fs.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include <video/fbcon.h>
#include <video/fbcon-mfb.h>

#define LCD_WIDTH 84
#define LCD_HEIGHT 48

/* allow for 1bpp */
static unsigned char *lcd_scr;

#define SD_DI 0x20
#define SD_DO 0x10
#define SD_CLK 0x08
#define SD_CS 0x80

typedef unsigned int uint32;
static unsigned char port_state = 0x00;
static volatile uint32 *gpioaddr_input = (uint32 *)0xb8000060;
static volatile uint32 *gpioaddr_output = (uint32 *)0xb8000064;
static volatile uint32 *gpioaddr_enable = (uint32 *)0xb8000068;
static volatile uint32 *gpioaddr_control = (uint32 *)0xb800006c;

static inline void lcd_spi_cs_low(void)
{
  port_state &= ~(SD_CS);
  *gpioaddr_output = port_state;
}

static inline void lcd_spi_cs_high(void)
{
  port_state |= SD_CS;
  *gpioaddr_output = port_state;
}

static inline void lcd_data_mode(void)
{
  port_state |= SD_DO;
  *gpioaddr_output = port_state;
}

static inline void lcd_command_mode(void)
{
  port_state &= ~(SD_DO);
  *gpioaddr_output = port_state;
}

static unsigned char lcd_spi_io(unsigned char data_out)
{
  int i;
  unsigned char result = 0, tmp_data = 0;
  
  for(i=0; i<8; i++) {
	  if(data_out & (0x01 << (7-i)))
		  port_state |= SD_DI;
	  else
		  port_state &= ~SD_DI;
	  
	  *gpioaddr_output = port_state;
	  port_state |= SD_CLK;
	  *gpioaddr_output = port_state;
	  
	  tmp_data = *gpioaddr_input;
	  
	  port_state &= ~SD_CLK;
	  *gpioaddr_output = port_state;
	  
	  result <<= 1;
	  
	  if(tmp_data & SD_DO)
		  result |= 1;
  }
  
  return(result);
}

#if 0
static void lcd_out(int x, int y, const char c)
{
	lcd_command_mode();

	lcd_spi_io(x | 0x80);
	lcd_spi_io(y | 0x40);

	lcd_data_mode();

	lcd_spi_io(c);
}
#endif

static void lcd_clear(void)
{
	int p, q;
	port_state &= ~(SD_DI);
	*gpioaddr_output = port_state;	

	lcd_data_mode();

	lcd_spi_cs_low();

	for(p=0; p<6; p++) {
		for(q=0; q<84; q++) {
			lcd_spi_io(0);
		}
	}
}


/* initialise the LCD */
static void
init_lcd(void)
{
	*gpioaddr_enable |= SD_DI | SD_CLK | SD_CS | SD_DO;

	port_state = *gpioaddr_output;
	port_state &= ~(SD_CLK | SD_DI | SD_CS);
	*gpioaddr_output = port_state;

	lcd_spi_cs_low();
	lcd_command_mode();

	lcd_spi_io(0x21);
	lcd_spi_io(0xc5);
	lcd_spi_io(0x06);
	lcd_spi_io(0x13);
	lcd_spi_io(0x20);
	lcd_spi_io(0x0c);

	lcd_clear();

	lcd_spi_cs_high();
}

static void lcd_detect(void)
{

}

static void lcd_update_display(struct display *p, int sx, int sy, int mx, int my)
{
	int x, y, i, bit;

	*gpioaddr_enable |= SD_DI | SD_CLK | SD_CS | SD_DO;
	port_state = *gpioaddr_output;

	lcd_spi_cs_low();

	unsigned char *scr = lcd_scr;

	lcd_command_mode();

	lcd_spi_io(0x80);
	lcd_spi_io(0x40);

	lcd_data_mode();

	// For now we just do braindead updating of the entire screen
	for(y = 0; y<LCD_HEIGHT/8; y++) {
		for(x = 0; x < (LCD_WIDTH+4)/8; x++) {
			for(bit = (x == LCD_WIDTH/8) ? 0x8 : 0x80;
			    bit != 0;
			    bit >>= 1) {
				for(i=7; i>=0; i--) {
					if(scr[x+(LCD_WIDTH+4)*i/8] & bit)
						port_state |= SD_DI;
					else
						port_state &= ~SD_DI;
					
					*gpioaddr_output = port_state;
					
					*gpioaddr_output = port_state | SD_CLK;
					*gpioaddr_output = port_state;
				}
			}
		}
		scr+=(LCD_WIDTH+4);
	}

	lcd_spi_cs_high();
}

struct lcdfb_info {
	struct fb_info_gen gen;
};


struct lcdfb_par {
	/*
	 *  The hardware specific data in this structure uniquely defines a video
	 *  mode.
	 *
	 *  If your hardware supports only one video mode, you can leave it empty.
	 */
};


void lcd_fb_setup(struct display *p)
{
	fbcon_mfb.setup(p);
}

void lcd_fb_bmove(struct display *p, int sy, int sx, int dy, int dx,
		     int height, int width)
{
	fbcon_mfb.bmove(p, sy, sx, dy, dx, height, width);
	lcd_update_display(p, 0, 0, LCD_WIDTH/8, LCD_HEIGHT/fontheight(p));
}

void lcd_fb_clear(struct vc_data *conp, struct display *p, int sy, int sx,
		     int height, int width)
{
	fbcon_mfb.clear(conp, p, sy, sx, height, width);
	lcd_update_display(p, sx, sy, sx+width, sy+height);
}

void lcd_fb_putc(struct vc_data *conp, struct display *p, int c, int yy,
		    int xx)
{
	fbcon_mfb.putc(conp, p, c, yy, xx);
	lcd_update_display(p, xx, yy, xx+1, yy+1);
}

void lcd_fb_putcs(struct vc_data *conp, struct display *p, 
		     const unsigned short *s, int count, int yy, int xx)
{
	fbcon_mfb.putcs(conp, p, s, count, yy, xx);
	lcd_update_display(p, xx, yy, xx+count, yy+1);
}

void lcd_fb_revc(struct display *p, int xx, int yy)
{
	fbcon_mfb.revc(p, xx, yy);
	lcd_update_display(p, xx, yy, xx+1, yy+1);
}

void lcd_fb_clear_margins(struct vc_data *conp, struct display *p,
			  int bottom_only)
{
	fbcon_mfb.clear_margins(conp, p, bottom_only);
	lcd_update_display(p, 0, 0, LCD_WIDTH, LCD_HEIGHT);
}

/*
 *  `switch' for the low level operations
 */

struct display_switch fbcon_lcd = {
	setup:		lcd_fb_setup,
	bmove:		lcd_fb_bmove,
	clear:		lcd_fb_clear,
	putc:		lcd_fb_putc,
	putcs:		lcd_fb_putcs,
	revc:		lcd_fb_revc,
	clear_margins:  lcd_fb_clear_margins,
	fontwidthmask:	FONTWIDTH(8)
};

static struct lcdfb_info fb_info;
static struct lcdfb_par current_par;
static int current_par_valid = 0;
static struct display disp;

int lcdfb_init(void);
int lcdfb_setup(char*);

static int lcd_encode_fix(struct fb_fix_screeninfo *fix, struct lcdfb_par *par,
			  const struct fb_info *info)
{
	/*
	 *  This function should fill in the 'fix' structure based on the values
	 *  in the `par' structure.
	 */


	memset(fix, 0x0, sizeof(*fix));

	strcpy(fix->id, "lcd");
	/* required for mmap() */
	fix->smem_start = lcd_scr;
	fix->smem_len = LCD_HEIGHT * (LCD_WIDTH+4)/8;

	fix->type= FB_TYPE_PACKED_PIXELS;

	fix->visual = FB_VISUAL_MONO10;	/* fixed visual */
	fix->line_length = (LCD_WIDTH+4) / 8;

	fix->xpanstep = 0;	/* no hardware panning */
	fix->ypanstep = 0;	/* no hardware panning */
	fix->ywrapstep = 0;	/* */

	fix->accel = FB_ACCEL_NONE;

	return 0;
}

static int lcd_decode_var(struct fb_var_screeninfo *var, struct lcdfb_par *par,
			  const struct fb_info *info)
{
	/*
	 *  Get the video params out of 'var'. If a value doesn't fit, round it up,
	 *  if it's too big, return -EINVAL.
	 *
	 *  Suggestion: Round up in the following order: bits_per_pixel, xres,
	 *  yres, xres_virtual, yres_virtual, xoffset, yoffset, grayscale,
	 *  bitfields, horizontal timing, vertical timing.
	 */


	if ( var->xres > LCD_WIDTH ||
		var->yres > LCD_HEIGHT ||
		var->xres_virtual != var->xres ||
		var->yres_virtual != var->yres ||
		var->xoffset != 0 ||
		var->yoffset != 0 ) {
		return -EINVAL;
	}

	if ( var->bits_per_pixel != 1 ) {
		return -EINVAL;
	}


	return 0;
}

static int lcd_encode_var(struct fb_var_screeninfo *var, struct lcdfb_par *par,
			  const struct fb_info *info)
{

	/*
	 *  Fill the 'var' structure based on the values in 'par' and maybe other
	 *  values read out of the hardware.
	 */

	var->xres = LCD_WIDTH;
	var->yres = LCD_HEIGHT;
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres;
	var->xoffset = 0;
	var->yoffset = 0;

	var->bits_per_pixel = 1;
	var->grayscale = 1;

	return 0;
}

static void lcd_get_par(struct lcdfb_par *par, const struct fb_info *info)
{
	/*
	 *  Fill the hardware's 'par' structure.
	 */

	if ( current_par_valid ) {
		*par = current_par;
	}
	else {
		/* ... */
	}
}

static void lcd_set_par(struct lcdfb_par *par, const struct fb_info *info)
{
	/*
	 *  Set the hardware according to 'par'.
	 */

	current_par = *par;
	current_par_valid = 1;

	/* ... */
}

static int lcd_getcolreg(unsigned regno, unsigned *red, unsigned *green,
			 unsigned *blue, unsigned *transp,
			 const struct fb_info *info)
{
	/*
	 *  Read a single color register and split it into colors/transparent.
	 *  The return values must have a 16 bit magnitude.
	 *  Return != 0 for invalid regno.
	 */

	/* ... */
	return 0;
}

static int lcd_setcolreg(unsigned regno, unsigned red, unsigned green,
			 unsigned blue, unsigned transp,
			 const struct fb_info *info)
{
	/*
	 *  Set a single color register. The values supplied have a 16 bit
	 *  magnitude.
	 *  Return != 0 for invalid regno.
	 */

	/* ... */
	return 0;
}

static int lcd_pan_display(struct fb_var_screeninfo *var,
			   struct lcdfb_par *par, const struct fb_info *info)
{
	/*
	 *  Pan (or wrap, depending on the `vmode' field) the display using the
	 *  `xoffset' and `yoffset' fields of the `var' structure.
	 *  If the values don't fit, return -EINVAL.
	 */

	/* ... */
	return -EINVAL;
}

static int lcd_blank(int blank_mode, const struct fb_info *info)
{

	switch (blank_mode) {
	case VESA_NO_BLANKING:
		break;

	case VESA_VSYNC_SUSPEND:
	case VESA_HSYNC_SUSPEND:
		break;

	case VESA_POWERDOWN:
		break;

	default:
		/* printk(KERN_ERR "unknown blank value %d\n", blank_mode); */
		return -EINVAL;
	}

	return 0;
}

static void lcd_set_disp(const void *par, struct display *disp,
			 struct fb_info_gen *info)
{

	/*
	 *  Fill in a pointer with the virtual address of the mapped frame buffer.
	 *  Fill in a pointer to appropriate low level text console operations (and
	 *  optionally a pointer to help data) for the video mode `par' of your
	 *  video hardware. These can be generic software routines, or hardware
	 *  accelerated routines specifically tailored for your hardware.
	 *  If you don't have any appropriate operations, you must fill in a
	 *  pointer to dummy operations, and there will be no text output.
	 */

	disp->screen_base = lcd_scr;
	disp->dispsw = &fbcon_lcd;
}


/* ------------ Interfaces to hardware functions ------------ */

struct fbgen_hwswitch lcd_switch = {
	detect:		lcd_detect,
	encode_fix:	lcd_encode_fix,
	decode_var:	lcd_decode_var,
	encode_var:	lcd_encode_var,
	get_par:	lcd_get_par,
	set_par:	lcd_set_par,
	getcolreg:	lcd_getcolreg,
	setcolreg:	lcd_setcolreg,
	pan_display:	lcd_pan_display,
	blank:		lcd_blank,
	set_disp:	lcd_set_disp,
};


/* ------------------------------------------------------------------------- */


/*
 *  Frame buffer operations
 */

static int lcd_fp_open(const struct fb_info *info, int user)
{

	return 0;
}

static int lcd_fb_ioctl(struct inode *inode, struct file *file, u_int cmd,
	u_long arg, int con, struct fb_info *info)

{
	return -EINVAL;
}


/*
 *  In most cases the `generic' routines (fbgen_*) should be satisfactory.
 *  However, you're free to fill in your own replacements.
 */

static struct fb_ops lcdfb_ops = {
	owner:		THIS_MODULE,
	fb_open:	lcd_fp_open,
	fb_get_fix:	fbgen_get_fix,
	fb_get_var:	fbgen_get_var,
	fb_set_var:	fbgen_set_var,
	fb_get_cmap:	fbgen_get_cmap,
	fb_set_cmap:	fbgen_set_cmap,
	fb_pan_display:	fbgen_pan_display,
	fb_ioctl:	lcd_fb_ioctl,
};


/* ------------ Hardware Independent Functions ------------ */


/*
 *  Initialization
 */

int read_proc(char *buf, char **start, off_t offset, int count,
		     int *eof, void *data)
{
	int length = sprintf(buf, "No debug at this point\n");

	/* housekeeping */
	if (length <= offset + count)
		*eof = 1;
	*start = buf + offset;
	length -= offset;
	if (length > count)
		length = count;
	if (length < 0)
		length = 0;

	return length;
}


int __init lcdfb_init(void)
{
	lcd_scr = kmalloc(LCD_HEIGHT * (LCD_WIDTH+4)/8, GFP_KERNEL);
	if(lcd_scr == NULL)
		return -EINVAL;

	memset(lcd_scr, 0, LCD_HEIGHT * (LCD_WIDTH+4)/8);

        if (!create_proc_read_entry("lcdfb", 0, 0, read_proc, NULL)) {
		// Too bad
	}

	fb_info.gen.fbhw = &lcd_switch;

	fb_info.gen.fbhw->detect();

	strcpy(fb_info.gen.info.modename, "lcd");

	fb_info.gen.info.changevar = NULL;
	fb_info.gen.info.node = -1;
	fb_info.gen.info.fbops = &lcdfb_ops;
	fb_info.gen.info.disp = &disp;
	fb_info.gen.info.switch_con = &fbgen_switch;
	fb_info.gen.info.updatevar = &fbgen_update_var;
	fb_info.gen.info.blank = &fbgen_blank;
	fb_info.gen.info.flags = FBINFO_FLAG_DEFAULT;

	/* This should give a reasonable default video mode */
	fbgen_get_var(&disp.var, -1, &fb_info.gen.info);
	fbgen_do_set_var(&disp.var, 1, &fb_info.gen);
	fbgen_set_disp(-1, &fb_info.gen);
	fbgen_install_cmap(0, &fb_info.gen);

	if ( register_framebuffer(&fb_info.gen.info) < 0 ) {
		kfree(lcd_scr);
		return -EINVAL;
	}

	init_lcd();

	printk(KERN_INFO "fb%d: %s frame buffer device\n", GET_FB_IDX(fb_info.gen.info.node), fb_info.gen.info.modename);

	/* uncomment this if your driver cannot be unloaded */
	/* MOD_INC_USE_COUNT; */
	return 0;
}


/*
 *  Cleanup
 */

void lcdfb_cleanup(struct fb_info *info)
{
	/*
	 *  If your driver supports multiple boards, you should unregister and
	 *  clean up all instances.
	 */

	unregister_framebuffer(info);
	kfree(lcd_scr);

	/* ... */
}


/*
 *  Setup
 */

int __init lcdfb_setup(char *options)
{
	/* Parse user speficied options (`video=lcdfb:') */
	return 0;
}

#ifdef MODULE
MODULE_LICENSE("GPL");
int init_module(void)
{
	return lcdfb_init();
}

void cleanup_module(void)
{
	
}

#endif /* MODULE */

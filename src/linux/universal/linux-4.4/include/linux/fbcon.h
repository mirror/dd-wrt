#ifndef _LINUX_FBCON_H
#define _LINUX_FBCON_H

#ifdef CONFIG_FRAMEBUFFER_CONSOLE
int  fbcon_modechange_possible(struct fb_info *info,
			       struct fb_var_screeninfo *var);
#else
static inline int  fbcon_modechange_possible(struct fb_info *info,
				struct fb_var_screeninfo *var) { return 0; }
#endif

#endif /* _LINUX_FBCON_H */

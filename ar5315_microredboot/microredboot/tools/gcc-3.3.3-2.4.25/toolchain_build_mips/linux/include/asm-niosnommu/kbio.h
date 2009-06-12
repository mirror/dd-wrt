#ifndef __LINUX_KBIO_H
#define __LINUX_KBIO_H

/* Return keyboard type */
#define KIOCTYPE    _IOR('k', 9, int)
/* Return Keyboard layout */
#define KIOCLAYOUT  _IOR('k', 20, int)

enum {
    TR_NONE,
    TR_ASCII,			/* keyboard is in regular state */
    TR_EVENT,			/* keystrokes sent as firm events */
    TR_UNTRANS_EVENT		/* EVENT+up and down+no translation */
};

/* Return the current keyboard translation */
#define KIOCGTRANS  _IOR('k', 5, int)
/* Set the keyboard translation */
#define KIOCTRANS   _IOW('k', 0, int)

/* Send a keyboard command */
#define KIOCCMD	    _IOW('k', 8, int)

/* Return if keystrokes are being sent to /dev/kbd */

/* Set routing of keystrokes to /dev/kbd */
#define KIOCSDIRECT _IOW('k', 10, int)

/* Top bit records if the key is up or down */
#define KBD_UP	    0x80

/* Usable information */
#define KBD_KEYMASK 0x7f

/* All keys up */
#define KBD_IDLE    0x75
#endif /* __LINUX_KBIO_H */

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * USB name database manipulation routines
 *
 * Copyright (C) 1999, 2000 Thomas Sailer (sailer@ife.ee.ethz.ch)
 */

#ifndef _NAMES_H
#define _NAMES_H

/* ---------------------------------------------------------------------- */

extern const char *names_vendor(uint16_t vendorid);
extern const char *names_product(uint16_t vendorid, uint16_t productid);
extern const char *names_class(uint8_t classid);
extern const char *names_subclass(uint8_t classid, uint8_t subclassid);
extern const char *names_protocol(uint8_t classid, uint8_t subclassid,
				  uint8_t protocolid);
extern const char *names_audioterminal(uint16_t termt);
extern const char *names_videoterminal(uint16_t termt);
extern const char *names_hid(uint8_t hidd);
extern const char *names_reporttag(uint8_t rt);
extern const char *names_huts(unsigned int data);
extern const char *names_hutus(unsigned int data);
extern const char *names_langid(uint16_t langid);
extern const char *names_physdes(uint8_t ph);
extern const char *names_bias(uint8_t b);
extern const char *names_countrycode(unsigned int countrycode);

extern int get_vendor_string(char *buf, size_t size, uint16_t vid);
extern int get_product_string(char *buf, size_t size, uint16_t vid, uint16_t pid);
extern int get_class_string(char *buf, size_t size, uint8_t cls);
extern int get_subclass_string(char *buf, size_t size, uint8_t cls, uint8_t subcls);
extern void get_vendor_product_with_fallback(char *vendor, int vendor_len,
					     char *product, int product_len,
					     libusb_device *dev);

extern int names_init(void);
extern void names_exit(void);

/* ---------------------------------------------------------------------- */
#endif /* _NAMES_H */

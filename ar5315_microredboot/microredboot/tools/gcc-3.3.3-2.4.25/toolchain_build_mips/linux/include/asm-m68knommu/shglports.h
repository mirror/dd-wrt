
/* Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *         1997, 1998  D. Jeff Dionne <jeff@uClinux.org>,
 */

#ifndef _M68K_SHGLPORTS_H
#define _M68K_SHGLPORTS_H

#include <linux/config.h>
#include <linux/sched.h>

#ifdef CONFIG_SHGLCORE

extern struct semaphore porte_interlock;

struct SHGLCORE_PORT_QS { 
  unsigned char
  nullqs:1, /* COM1TX */ 
  sbin:1,   /* PQS6 (PCS3) */
  sbclk:1,  /* PQS5 (PCS2) */
  sbout:1,  /* PQS4 (PCS1) */
  null4:4;  /* MISO, MOSI, SCLK, /SS=PCS0 */
};

#define PORT_QS ((volatile struct SHGLCORE_PORT_QS*)PORTQS_ADDR)

struct SHGLCORE_PORT_E {
  unsigned char
  dead:1,       /* LED */
  sbirigb:1,    /* PE6 */
  ds:1,         /* /DS */
  nulle1:1,     /* na */ 
  sbpll:1,      /* PE3 */
  avec:1,       /* /AVEC */
  sbsrom:1,     /* PE1 */
  sbpanel:1;    /* PE0 */
};

#define PORT_E ((volatile struct SHGLCORE_PORT_E*)PORTE_ADDR)

struct SHGLCORE_PORT_F {
  unsigned char
  nullf1:4,
  nullf2:4;
};

#define PORT_F ((volatile struct SHGLCORE_PORT_F*)PORTF_ADDR)

extern int comm_status_led, comm_error_led, alarm_led;

static inline void SET_COMM_STATUS_LED(int value) {
	BYTE_REF(SHGLCORE_ACC_ADDR+0x100+0) = comm_status_led = value;
}
static inline int GET_COMM_STATUS_LED(void) {
	return comm_status_led;
}


static inline void SET_COMM_ERROR_LED(int value) {
	BYTE_REF(SHGLCORE_ACC_ADDR+0x100+1) = comm_error_led = value;
}
static inline int GET_COMM_ERROR_LED(void) {
	return comm_error_led;
}


static inline void SET_ALARM_LED(int value) {
	BYTE_REF(SHGLCORE_ACC_ADDR+0x100+2) = alarm_led = value;
}
static inline int GET_ALARM_LED(void) {
	return alarm_led;
}

#endif

#endif /* _M68K_SHGLPORTS_H */

/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: sens.h 690 2008-03-31 18:36:43Z  $
 *
 */

#ifndef __SENS_H__
#define __SENS_H__

#include "data.h"

/* load data aor acls
 * @param status:
 *        SENS_TEST: test file
 *        SENS_LOAD: load file
 * @return:   0 if is ok
 *           -1 if is not ok
 */
#define SENS_TEST   0
#define SENS_LOAD   1
int sens_init(int status);

// free data memory
void sens_free(void);

// reload data
void sens_reload(void);

// add sens to hash
void sens_add(struct ether_addr *mac, struct in_addr ip,
              struct in_addr mask, struct capt *idcap);

// test if sens exists
int  sens_exist(struct ether_addr *mac, struct in_addr ip,
                struct capt *idcap);

#endif

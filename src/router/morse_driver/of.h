#ifndef _MORSE_OF_H_
#define _MORSE_OF_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "hw.h"

/**
 * morse_of_probe - reads of pins in compatible device tree.
 * @dev: device struct containing the of_node
 * @cfg: morse_hw_config struct to be updated from of_node
 * @match_table: match table containing the compatibility strings
 *
 * Return:
 * * 0 - if device tree parsed successfully
 * * -ENOENT - required item in device tree was missing
 */
int morse_of_probe(struct device *dev, struct morse_hw_cfg *cfg,
		   const struct of_device_id *match_table);

#endif /* !_MORSE_OF_H_ */

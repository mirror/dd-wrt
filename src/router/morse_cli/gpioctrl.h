/*
 * Copyright 2020 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#pragma once

#define RESET_GPIO "MM_RESET_PIN"
#define JTAG_GPIO "MM_JTAG_PIN"

int gpio_export(int pin);
int gpio_unexport(int pin);

int gpio_set_dir(int pin, const char dirc[]);
int gpio_set_val(int pin, int val);
int gpio_get_env(char env_var[]);

int path_exists(char path[]);

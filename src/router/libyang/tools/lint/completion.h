/**
 * @file main.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool auto completion header
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef COMPLETION_H_
#define COMPLETION_H_

#include "../../linenoise/linenoise.h"

void complete_cmd(const char *buf, const char *hint, linenoiseCompletions *lc);

#endif /* COMPLETION_H_ */

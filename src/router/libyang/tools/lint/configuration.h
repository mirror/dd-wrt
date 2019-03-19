/**
 * @file configuration.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yanglint configuration header
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_


/**
 * @brief Finds the current user's yanglint dir
 * @return NULL on failure, dynamically allocated yanglint dir path
 * otherwise
 */
char *get_yanglint_dir(void);

/**
 * @brief Checks all the relevant files and directories creating any
 * that are missing, sets the saved configuration (currently only history)
 */
void load_config(void);

/**
 * @brief Saves the current configuration (currently only history)
 */
void store_config(void);

#endif /* CONFIGURATION_H_ */

/***********************license start***************
 * Copyright (c) 2012  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

#ifndef __CVMX_APP_CONFIG_H__
#define __CVMX_APP_CONFIG_H__

#ifdef    __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* This defines the name of the named block from which config(pko, pools)
   is exported/imported */
#define CVMX_APP_CONFIG "cvmx-app-config"

/* skip_app_config */
extern int skip_app_config;

/**
 * Sets the skip_app_config. This can be called by the application when
 * it wants to skip the configuration.
 */
void cvmx_skip_app_config_set(void);

/**
 * @INTERNAL
 * Called by apps to export app config to other
 * cooperating applications using a named block
 * defined by param block_name.
 *
 * @param block_name Name of the named block to use for exporting config.
 *
 * @return 0 on success.
 */
int __cvmx_export_app_config_to_named_block(char * block_name);

/**
 * @INTERNAL
 * Called by apps to import app config from other
 * cooperating applications using a named block
 * defined by param block_name.
 *
 * @param block_name Name of the named block to use for exporting config.
 *
 * @return 0 on success.
 */
int __cvmx_import_app_config_from_named_block(char * block_name);

/**
 * @INTERNAL
 * Called by apps to clean app config named block.
 */
void __cvmx_export_app_config_cleanup(void);

int __cvmx_export_config(void);

#ifdef  __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /* __CVMX_APP_CONFIG_H__ */

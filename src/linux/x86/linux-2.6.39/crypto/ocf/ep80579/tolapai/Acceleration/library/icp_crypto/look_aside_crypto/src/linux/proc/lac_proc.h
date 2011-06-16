/******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 *****************************************************************************/

/**
 *******************************************************************************
 * @file lac_proc.h    
 * 
 * @defgroup LacProc   Proc stats
 * 
 * @ingroup Lac
 * 
 * 
 *  
 ******************************************************************************/

#ifndef LAC_PROC_H
#define LAC_PROC_H

/* 
********************************************************************************
* Include public/global header files 
********************************************************************************
*/ 

/* 
********************************************************************************
* Include private header files 
********************************************************************************
*/ 

/******************************************************************************/

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise LAC proc stats components
 *
 * @description
 *      Create a subdirectory in /proc filesystem and create a file for
 *      each components.
 *
 * @retval 0                        No error
 * @retval -ENOMEM                  Function fails
 *
 ******************************************************************************/
int LacProc_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Cleanup LAC proc stats components
 *
 * @description
 *      Delete the subdirectory and every files that have been created in the
 *      initialisation procedure.
 *
 ******************************************************************************/
void LacProc_cleanup(void);



/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for QAT component.
 ******************************************************************************/
int lacProcQat_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for Sym component.
 ******************************************************************************/
int lacProcSym_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for RSA component.
 ******************************************************************************/
int lacProcRsa_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for Rand component.
 ******************************************************************************/
int lacProcRand_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for Prime component.
 ******************************************************************************/
int lacProcPrime_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for LN component.
 ******************************************************************************/
int lacProcLn_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for Key component.
 ******************************************************************************/
int lacProcKey_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for DSA component.
 ******************************************************************************/
int lacProcDsa_init(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Initialise proc stats file for DH component.
 ******************************************************************************/
int lacProcDh_init(void);


/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for QAT component.
 ******************************************************************************/
void lacProcQat_cleanup(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for Sym component.
 ******************************************************************************/
void lacProcSym_cleanup(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for RSA component.
 ******************************************************************************/
void lacProcRsa_cleanup(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for Rand component.
 ******************************************************************************/
void lacProcRand_cleanup(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for Prime component.
 ******************************************************************************/
void lacProcPrime_cleanup(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for LN component.
 ******************************************************************************/
void lacProcLn_cleanup(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for Key component.
 ******************************************************************************/
void lacProcKey_cleanup(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for DSA component.
 ******************************************************************************/
void lacProcDsa_cleanup(void);

/**
 *******************************************************************************
 * @ingroup LacProc
 *      Delete proc stats file for DH component.
 ******************************************************************************/
void lacProcDh_cleanup(void);



#endif /* LAC_PROC_H */

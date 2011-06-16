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

===============================================================================

Intel® EP80579 Sample Code for Security Applications on Intel®
QuickAssist Technology

July, 2009

===============================================================================

Reference
=========

 - Intel® EP80579 Software for Security Applications on Intel®
   QuickAssist Technology Getting Started Guide

===============================================================================
     

Installing and Running the Security Sample Functional and Performance Tests

===============================================================================

1) General 
  This is how to generate and run sample code for security on Linux and FreeBSD 


===============================================================================

2) Compilation of the Performance Module



   The following environment variables need to be set in order to build the 
   modules.
   Note: This assumes sources were untarred in /EP80579_release directory.
	 Please ensure that the Acceleration modules are built before building 
         the Sample Performance Code

   setenv ICP_ROOT /EP80579_release

   setenv ICP_BUILDSYSTEM_PATH $ICP_ROOT/build_system

   setenv ICP_ENV_DIR 
            $ICP_ROOT/Acceleration/library/icp_crypto/look_aside_crypto

   To change the Linux's target kernel sources, the sample code to be compiled 
   for, from their default location /usr/src/kernels/2.6.28.10
   set the KERNEL_SOURCE_ROOT environmental variable to point the location of
   desired target kernel sources
   (e.g. setenv KERNEL_SOURCE_ROOT /usr/src/linux/) prior to compilation of
   Performance Module.

   To create the Linux kernel object use the following command from within
   the directories containing the Makefiles:

           make clean && make module

   To create the FreeBSD kernel object use the following command from within
   the directories containing the Makefiles:

           make clean && make

   The kernel object for the performance tests can be created from the 
   following directory : 

       $ICP_ENV_DIR/src/sample_code/performance/

   The generated Linux kernel object is located at:
   
       $ICP_ENV_DIR/src/sample_code/performance/
                        build/linux_2.6/kernel_space/crypto_perf.ko

   The generated FreeBSD kernel object is located at:
   
       $ICP_ENV_DIR/src/sample_code/performance/
                        build/freebsd/kernel_space/crypto_perf.ko


===============================================================================

3) Performance Module Load/Unload Instructions



   To run this performance sample code, on Linux use the following command: 

       insmod crypto_perf.ko 

   To run this performance sample code, on FreeBSD use the following command: 

       kldload ./crypto_perf.ko 

   NOTE: all security components must be initialized before attempting to run 
   the module.

   The following tests will then run:

       Cipher Encrypt 3DES-CBC: 1,000,000 operations of packet sizes 
            64, 128, 256, 512, 1024, 2048, 4096
       Hash HMAC-SHA1: 1,000,000 operations of packet sizes 
            64, 128, 256, 512, 1024, 2048, 4096
       Algorithm Chaining- 3DES-CBC HMAC-SHA1: 1,000,000 operations of packet 
            sizes 64, 128, 256, 512, 1024, 2048, 4096

       RSA CRT Decrypt 1024 bit: 100,000 operations

   Once the test has completed, "Performance Code Complete- All Tests Pass" is
   displayed. The module can then be unloaded.
   
   To unload the kernel module on Linux use the following command:

       rmmod crypto_perf.ko

   To unload the kernel module on FreeBSD use the following command:

       kldunload crypto_perf.ko


===============================================================================

4) Compilation of the Functional Modules

   The following environment variables need to be set in order to build the 
   modules.
   Note: assuming sources were untarred in /EP80579_release directory

   setenv ICP_ROOT /EP80579_release

   setenv ICP_BUILDSYSTEM_PATH $ICP_ROOT/build_system

   setenv ICP_ENV_DIR 
            $ICP_ROOT/Acceleration/library/icp_crypto/look_aside_crypto

   To change the Linux's target kernel sources, the sample code to be compiled 
   for, from their default location /usr/src/kernels/2.6.28.10
   set the KERNEL_SOURCE_ROOT environmental variable to point the location of
   desired target kernel sources
   (e.g. setenv KERNEL_SOURCE_ROOT /usr/src/linux/) prior to compilation of
   a Functional Module.

   Functional tests modules can be created for each component from the 
   following directories:
    
       $ICP_ENV_DIR/src/sample_code/functional/asym/diffie_hellman_sample/ 
       $ICP_ENV_DIR/src/sample_code/functional/asym/prime_sample/ 
       $ICP_ENV_DIR/src/sample_code/functional/sym/alg_chaining_sample/ 
       $ICP_ENV_DIR/src/sample_code/functional/sym/cipher_sample/ 
       $ICP_ENV_DIR/src/sample_code/functional/sym/hash_sample/ 


   To create these modules for Linux use the following command:

       make clean && make module

   To create these modules for FreeBSD use the following command:

       make clean && make

   The kernel objects created for each component will be:
   
        dh_sample.ko 
        prime_sample.ko
        algchaining_sample.ko
        cipher_sample.ko
        hash_sample.ko
         
   Each Linux module can be found within the respective directory at:
    
       build/linux_2.6/kernel_space/

   Each FreeBSD module can be found within the respective directory at:
    
       build/freebsd/kernel_space/


===============================================================================
          
5) Functional Modules Load/Unload Instructions

   To run the functional sample code on Linux, use the following command with
   respect to each kernel module: 

       insmod <kernel_object>

   To run the functional sample code on FreeBSD, use the following command with
   respect to each kernel module: 

       kldload ./<kernel_object>

   These modules do not have any run time functionality contained within them
   so there is no reason to keep this module loaded in the kernel once the 
   functional sample code tests complete.

   Linux:
   The tests run directly from the mod init hook called by the insmod command. 
   Once the tests have completed the module is unloaded by returning the 
   -EAGAIN value in the mod init hook. 

   Hence ignore the error returned by the insmod command

   insmod: error inserting '<module name>': -1 Resource temporarily unavailable

   This is the intended behaviour which results in the module being unloaded 
   once tests complete.

   FreeBSD:
   To unload the kernel module on FreeBSD use the following command:

       kldunload <kernel_object>


===============================================================================

6) Known Issues

   This is sample code and all invalid cases are not fully covered.



Legal/Disclaimers
===================

INFORMATION IN THIS DOCUMENT IS PROVIDED IN CONNECTION WITH INTEL(R) PRODUCTS.
NO LICENSE, EXPRESS OR IMPLIED, BY ESTOPPEL OR OTHERWISE, TO ANY INTELLECTUAL
PROPERTY RIGHTS IS GRANTED BY THIS DOCUMENT. EXCEPT AS PROVIDED IN INTEL'S 
TERMS AND CONDITIONS OF SALE FOR SUCH PRODUCTS, INTEL ASSUMES NO LIABILITY 
WHATSOEVER, AND INTEL DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTY, RELATING TO 
SALE AND/OR USE OF INTEL PRODUCTS INCLUDING LIABILITY OR WARRANTIES RELATING 
TO FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABILITY, OR INFRINGEMENT OF ANY 
PATENT, COPYRIGHT OR OTHER INTELLECTUAL PROPERTY RIGHT. Intel products are 
not intended for use in medical, life saving, life sustaining, critical control
 or safety systems, or in nuclear facility applications.

Intel may make changes to specifications and product descriptions at any time,
without notice.

(C) Intel Corporation 2008  

* Other names and brands may be claimed as the property of others.

===============================================================================



/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   debug.h - Debugging Definitions and Declarations;
 *
 *   this file is a subset of the original that includes only those
 *   definitions and declaration needed for toolkit programs;
 *
 *.  Qualcomm Atheros HomePlug AV Powerline Toolkit
 *:  Copyright (c) 2009-2013 by Qualcomm Atheros Inc. ALL RIGHTS RESERVED;
 *;  For demonstration and evaluation only; Not for production use.
 *
 *--------------------------------------------------------------------*/

#ifndef DEBUG_HEADER
#define DEBUG_HEADER

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define QUALCOMM_DEBUG 1
#define QUALCOMM_TRACE 1

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#if QUALCOMM_TRACE
#define EVENT(s) printk(KERN_DEBUG "---[ %s ]---\n", (s))
#define ENTER printk(KERN_DEBUG "-->[ %s:%d ]\n", __func__,__LINE__)
#define ABORT printk(KERN_DEBUG "<--( %s:%d )\n", __func__,__LINE__)
#define LEAVE printk(KERN_DEBUG "<--[ %s:%d ]\n", __func__,__LINE__)
#define CRUMB printk(KERN_DEBUG "... %s:%d\n",__func__,__LINE__)
#else
#define EVENT {}
#define ENTER {}
#define ABORT {}
#define LEAVE {}
#define CRUMB {}
#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#if QUALCOMM_DEBUG
#define address(e) if (!(e)) { printk ("%s(%d): address %s is null\n",__func__,__LINE__,#e); }
#define confirm(e) if (!(e)) { printk ("%s(%d): assertion %s is wrong\n",__func__,__LINE__, #e); }
#else
#define address(e) {}
#define confirm(e) {}
#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif


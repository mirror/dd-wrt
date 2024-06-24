/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2011, 2012
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "config.h"

#ifdef __ARM_EABI__
#ifdef __ARM_PCS_VFP
#ifdef __ARM_ARCH_7A__
#include "callNativeEABIHard.S"
#else
#include "callNativeEABIHardARM.S"
#endif
#else
#include "callNativeEABI.S"
#endif
#else
#include "callNativeOABI.S"
#endif

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif

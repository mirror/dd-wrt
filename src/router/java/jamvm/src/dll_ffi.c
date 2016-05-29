/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011
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

/* Must be included first to get configure options */
#include "jam.h"

#ifdef USE_FFI
#include <ffi.h>
#include <stdio.h>
#include "sig.h"
#include "excep.h"
#include "properties.h"

ffi_type *sig2ffi(char sig) {
    switch(sig) {
        case 'V':
	    return &ffi_type_void;
        case 'D':
	    return &ffi_type_double;
        case 'J':
	    return &ffi_type_sint64;
        case 'F':
	    return &ffi_type_float;
        default:
            return &ffi_type_pointer;
    }
}

int nativeExtraArg(MethodBlock *mb) {
    char *sig = mb->simple_sig;
    int count = 2;

    SCAN_SIG(sig, count++, count++);

    return count;
}

#define DOUBLE                             \
    *types_pntr++ = sig2ffi(*sig);         \
    *values_pntr++ = opntr;                \
    opntr += 2

#define SINGLE                             \
    *types_pntr++ = sig2ffi(*sig);         \
    if(*sig == 'F' && IS_BE64)             \
        *values_pntr++ = (float*)opntr + 1;\
    else                                   \
        *values_pntr++ = opntr;            \
    opntr += 1

uintptr_t *callJNIMethod(void *env, Class *class, char *sig, int num_args,
                         uintptr_t *ostack, unsigned char *func) {
    ffi_cif cif;
    ffi_status err;
    void *values[num_args];
    ffi_type *types[num_args];
    uintptr_t *opntr = ostack;
    void **values_pntr = &values[2];
    ffi_type **types_pntr = &types[2];

    types[0] = &ffi_type_pointer;
    values[0] = &env;

    types[1] = &ffi_type_pointer;
    values[1] = class ? &class : (void*)opntr++;

    SCAN_SIG(sig, DOUBLE, SINGLE);

    err = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, num_args, sig2ffi(*sig), types);

    /* ffi_prep_cif should never fail, but throw an internal error
       if it does */
    if(err != FFI_OK)
        signalException(java_lang_InternalError, "ffi_prep_cif failed");
    else {
        float *ret = (float*)ostack + (*sig == 'F' && IS_BE64);

        ffi_call(&cif, FFI_FN(func), ret, values);

        switch(*sig) {
            case 'V':
                break;
            case 'J':
            case 'D':
	        ostack += 2;
                break;
#ifdef FFI_RET_EXTEND
            case 'Z':
            case 'B':
                *ostack = (signed char)*ostack;
                ostack++;
                break;
            case 'C':
                *ostack = (unsigned short)*ostack;
                ostack++;
                break;
            case 'S':
                *ostack = (signed short)*ostack;
                ostack++;
                break;
#endif
            default:
	        ostack++;
                break;
        }
    }

    return ostack;
}
#endif

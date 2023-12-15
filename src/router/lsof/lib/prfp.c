/*
 * prfp.c -- process_file() function for lsof library
 */

/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#include "common.h"
#include "machine.h"

#if defined(USE_LIB_PROCESS_FILE)

/*
 * process_file() - process file
 */

/*
 * The caller may define:
 *
 *	FILEPTR	as the name of the location to store a pointer
 *			to the current file struct -- e.g.,
 *
 *			struct file *foobar;
 *			#define FILEPTR	foobar
 */

void process_file(struct lsof_context *ctx,
                  KA_T fp) /* kernel file structure address */
{
    struct file f;
    int flag;
    char tbuf[32];

#    if defined(FILEPTR)
    /*
     * Save file structure address for process_node().
     */
    FILEPTR = &f;
#    endif /* defined(FILEPTR) */

    /*
     * Read file structure.
     */
    if (kread(ctx, (KA_T)fp, (char *)&f, sizeof(f))) {
        (void)snpf(Namech, Namechl, "can't read file struct from %s",
                   print_kptr(fp, (char *)NULL, 0));
        enter_nm(ctx, Namech);
        return;
    }
    Lf->off = (SZOFFTYPE)f.f_offset;
    Lf->off_def = 1;
    if (f.f_count) {

        /*
         * Construct access code.
         */
        if ((flag = (f.f_flag & (FREAD | FWRITE))) == FREAD)
            Lf->access = LSOF_FILE_ACCESS_READ;
        else if (flag == FWRITE)
            Lf->access = LSOF_FILE_ACCESS_WRITE;
        else if (flag == (FREAD | FWRITE))
            Lf->access = LSOF_FILE_ACCESS_READ_WRITE;

#    if defined(HASFSTRUCT)
            /*
             * Save file structure values.
             */

#        if !defined(HASNOFSCOUNT)
        Lf->fct = (long)f.f_count;
        Lf->fsv |= FSV_CT;
#        endif /* !defined(HASNOFSCOUNT) */

#        if !defined(HASNOFSADDR)
        Lf->fsa = fp;
        Lf->fsv |= FSV_FA;
#        endif /* !defined(HASNOFSADDR) */

#        if !defined(HASNOFSFLAGS)
        Lf->ffg = (long)f.f_flag;
        Lf->fsv |= FSV_FG;
#        endif /* !defined(HASNOFSFLAGS) */

#        if !defined(HASNOFSNADDR)
        Lf->fna = (KA_T)f.f_data;
        Lf->fsv |= FSV_NI;
#        endif /* !defined(HASNOFSNADDR) */
#    endif     /* defined(HASFSTRUCT) */

        /*
         * Process structure by its type.
         */
        switch (f.f_type) {

#    if defined(DTYPE_PIPE)
        case DTYPE_PIPE:
#        if defined(HASPIPEFN)
            if (!Selinet)
                HASPIPEFN(ctx, (KA_T)f.f_data);
#        endif /* defined(HASPIPEFN) */
            return;
#    endif /* defined(DTYPE_PIPE) */

#    if defined(DTYPE_PTS)
        case DTYPE_PTS:
#        if defined(HASPTSFN)
            HASPTSFN(ctx, (KA_T)f.f_data);
#        endif /* defined(HASPTSFN) */
            return;
#    endif /* defined(DTYPE_PIPE) */

#    if defined(DTYPE_FIFO)
        case DTYPE_FIFO:
#    endif /* defined(DTYPE_FIFO) */

#    if defined(DTYPE_GNODE)
        case DTYPE_GNODE:
#    endif /* defined(DTYPE_GNODE) */

#    if defined(DTYPE_INODE)
        case DTYPE_INODE:
#    endif /* defined(DTYPE_INODE) */

#    if defined(DTYPE_PORT)
        case DTYPE_PORT:
#    endif /* defined(DTYPE_PORT) */

#    if defined(DTYPE_VNODE)
        case DTYPE_VNODE:
#    endif /* defined(DTYPE_VNODE) */

#    if defined(HASF_VNODE)
            process_node(ctx, (KA_T)f.f_vnode);
#    else  /* !defined(HASF_VNODE) */
            process_node(ctx, (KA_T)f.f_data);
#    endif /* defined(HASF_VNODE) */

            return;
        case DTYPE_SOCKET:
            process_socket(ctx, (KA_T)f.f_data);
            return;

#    if defined(HASKQUEUE)
        case DTYPE_KQUEUE:
            process_kqueue(ctx, (KA_T)f.f_data);
            return;
#    endif /* defined(HASKQUEUE) */

#    if defined(HASPSXSEM)
        case DTYPE_PSXSEM:
            process_psxsem(ctx, (KA_T)f.f_data);
            return;
#    endif /* defined(HASPSXSEM) */

#    if defined(HASPSXSHM)
        case DTYPE_PSXSHM:
            process_psxshm(ctx, (KA_T)f.f_data);
            return;
#    endif /* defined(HASPSXSHM) */

#    if defined(HASPRIVFILETYPE)
        case PRIVFILETYPE:
            HASPRIVFILETYPE(ctx, (KA_T)f.f_data);
            return;
#    endif /* defined(HASPRIVFILETYPE) */

        default:

#    if defined(X_BADFILEOPS)
            if (X_bfopsa && f.f_ops && (X_bfopsa == (KA_T)f.f_ops)) {
                (void)snpf(Namech, Namechl,
                           "no more information; ty=%d file may be closing",
                           (int)f.f_type);
                enter_nm(ctx, Namech);
                return;
            }
#    endif /* defined(X_BADFILEOPS) */

            if (f.f_type || f.f_ops) {
                (void)snpf(Namech, Namechl, "%s file struct, ty=%d, op=%s",
                           print_kptr(fp, tbuf, sizeof(tbuf)), (int)f.f_type,
                           print_kptr((KA_T)f.f_ops, (char *)NULL, 0));
                enter_nm(ctx, Namech);
                return;
            }
        }
    }
    enter_nm(ctx, "no more information");
}
#else  /* !defined(USE_LIB_PROCESS_FILE) */
char prfp_d1[] = "d";
char *prfp_d2 = prfp_d1;
#endif /* defined(USE_LIB_PROCESS_FILE) */

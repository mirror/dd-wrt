/*
 * Copyright (c) 2009, 2011, 2012
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/* $Id: socks_glibc.h,v 1.10 2012/06/01 20:23:05 karls Exp $ */

/*
 * Workaround for limitations in newer glibc versions, where LD_PRELOAD
 * cannot intercept system calls made by glibc.
 *
 * Code contributed by Markus Moeller (markus_moeller at compuserve.com),
 * thanks.
 */

#if HAVE_GSSAPI
#define gets Rgets
#define fgetc Rfgetc
#define fgets Rfgets
#define fputc Rfputc
#define fputs Rfputs
#define fflush Rfflush
#define fclose Rfclose
#define fprintf Rfprintf
#define vfprintf Rvfprintf
#define fwrite Rfwrite
#define fread Rfread
#define puts(x) Rfputs(x, stdout)
#define printf(x, ...) Rfprintf(stdout,x, __VA_ARGS__)
#define vprintf(x, y)  Rvfprintf(stdout,x, y)
/* Undefine any macro */
#ifdef getc
#undef getc
#endif
#define getc Rfgetc
/* Undefine any macro */
#ifdef putc
#undef putc
#endif
#define putc Rfputc
/* Some Linux functions */
#ifdef __fprintf_chk
#undef __fprintf_chk
#endif
#define __fprintf_chk Rfprintf
#ifdef __vfprintf_chk
#undef __vfprintf_chk
#endif
#define __vfprintf_chk Rvfprintf

int Rfgetc(FILE *fp);
int Rfputc(int c, FILE *fp);
char *Rgets(char *s);
char *Rfgets(char *s, int size, FILE *fp);
int Rfputs(const char *s, FILE *fp);
int Rfprintf(FILE *fp, const char *s, ...);
int Rvfprintf(FILE *fp, const char *s, va_list ap);
size_t Rfwrite(const void *ptr, size_t size, size_t nmb, FILE *fp);
size_t Rfread(void *ptr, size_t size, size_t nmb, FILE *fp);
int Rfflush(FILE *fp);
int Rfclose(FILE *fp);
#endif /* HAVE_GSSAPI */

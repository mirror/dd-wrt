/*
 * Copyright (c) 2012, 2013, 2014, 2020, 2021, 2024
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

#include "common.h"

static const char rcsid[] =
"$Id: shmemconfig.c,v 1.49.4.5.6.6.4.2 2024/11/20 22:05:40 karls Exp $";

static size_t
linkedsize(const linkedname_t *head);
/*
 * Returns size used for contents of "head" (if not NULL), and all following.
 */

static void
poolinit(void *pool, const size_t size);
/*
 * inits a memorypool of size "size" using the memoryblock "pool", including
 * initing global alignment variables.
 *
 * Subsequent calls to getmem() will return maximally aligned memory
 * from that pool.
 */

void *
getmem(const size_t size);
/*
 * Returns "size" bytes of memory from the current memorypool,
 * or NULL if there is not enough memory available.
 */

# if 0
static size_t
memleft(void);
/*
 * Returns the number of bytes left in the memorypool.
 */
#endif

static size_t
memleft_no_alignment_luck(void);
/*
 *
 */

typedef enum { COPY = 1, SIZE } configop_t;
static ssize_t
pointer_copyorsize(const configop_t op, struct config *src,
                   const ptrdiff_t srcoffset, struct config *dst,
                   void *mem, const size_t memsize);
/*
 * copies or sizes up all pointers in src based on whether "op" indicates
 * the operation is copy or size.
 *
 * If the op is COPY, "mem" points to a memory area of size "memsize", which
 * must be big enough to hold the contents of all pointers.
 * If "mem" is NULL if the function should allocate the memory via malloc(3)
 * itself.
 *
 * Returns:
 *    If op is COPY or FREE: 0 on success, -1 on failure.
 *    If op is SIZE: size required to hold the contents of all pointers.
 */


/*
 * variable for local memorypool inited by poolinit() and used by getmem().
 */
static void *_nextstart;
static uintptr_t _alignment, _mask;

static size_t _left, _left_no_alignment_luck, _extra_due_alignment;

#define ADDLEN(_len, total)                                                    \
do {                                                                           \
   const size_t len = (_len);                                                  \
                                                                               \
   if ((len) != 0) {                                                           \
      slog(LOG_DEBUG, "%s: adding %lu bytes at line %d",                       \
           function, (unsigned long)((len) + _alignment), __LINE__);           \
                                                                               \
      *(total) += ((len) + _alignment);                                        \
   }                                                                           \
} while (/* CONSTCOND */ 0)


#define DOOFFSET(object, offset)                                               \
do {                                                                           \
   if (object != NULL)                                                         \
      (object) = (void *)((uintptr_t)(object) + (offset));                     \
} while (/* CONSTCOND */ 0)

#define DOCOPY(src, srcoffset, dst, attr, attrsize, memfunc)                   \
do {                                                                           \
   const size_t tocopy = (attrsize);                                           \
                                                                               \
   if (tocopy == 0) {                                                          \
      SASSERTX((src)->attr == NULL);                                           \
      SASSERTX((dst)->attr == NULL);                                           \
   }                                                                           \
   else {                                                                      \
      SASSERTX((src)->attr != NULL);                                           \
                                                                               \
      if (((dst)->attr = memfunc(tocopy)) == NULL) {                           \
         swarn("%s: failed to allocate %lu byte%s of memory at line %d.  "     \
                "Memory left: %lu",                                            \
                function,                                                      \
                (unsigned long)tocopy,                                         \
                tocopy == 1 ? "" : "s",                                        \
                __LINE__,                                                      \
                (unsigned long)memleft_no_alignment_luck());                   \
         return -1;                                                            \
      }                                                                        \
                                                                               \
      DOOFFSET(((src)->attr), srcoffset);                                      \
      memcpy((dst)->attr, (src)->attr, tocopy);                                \
   }                                                                           \
} while (/* CONSTCOND */ 0)

#define DOLINKCOPY(lsrc, srcoffset, ldst, lattr, memfunc)                      \
do {                                                                           \
   linkedname_t *srclink, *dstlink;                                            \
                                                                               \
   SASSERTX(lsrc != NULL);                                                     \
                                                                               \
   if ((srclink = (lsrc)->lattr) == NULL)                                      \
      break;                                                                   \
                                                                               \
   DOOFFSET(srclink, srcoffset);                                               \
   (lsrc)->lattr = srclink; /* update src also. */                             \
                                                                               \
   if (((ldst)->lattr = memfunc(sizeof(*(ldst)->lattr))) == NULL) {            \
      swarnx("%s: failed to allocate %lu byte%s of memory at line %d",         \
             function,                                                         \
             (unsigned long)sizeof(*(ldst)->lattr),                            \
             (unsigned long)sizeof(*(ldst)->lattr) == 1 ? "" : "s",            \
             __LINE__);                                                        \
      return -1;                                                               \
   }                                                                           \
   dstlink = (ldst)->lattr;                                                    \
                                                                               \
   do {                                                                        \
      char *name;                                                              \
                                                                               \
      SASSERTX(srclink->name != NULL);                                         \
      name = srclink->name;                                                    \
                                                                               \
      DOOFFSET(name, srcoffset); /* just for strlen(). */                      \
      DOCOPY(srclink, srcoffset, dstlink, name, strlen(name) + 1, memfunc);    \
                                                                               \
      DOOFFSET(srclink->next, srcoffset);                                      \
      if (srclink->next == NULL)                                               \
         dstlink->next = NULL;                                                 \
      else {                                                                   \
         if ((dstlink->next = memfunc(sizeof(*dstlink->next))) == NULL) {      \
            swarnx("%s: failed to allocate %lu byte%s of memory at line %d",   \
                   function,                                                   \
                   (unsigned long)sizeof(*dstlink->next),                      \
                   (unsigned long)sizeof(*dstlink->next) == 1 ? "" : "s",      \
                   __LINE__);                                                  \
            return -1;                                                         \
         }                                                                     \
      }                                                                        \
                                                                               \
      dstlink = dstlink->next;                                                 \
      srclink = srclink->next;                                                 \
   } while (srclink != NULL);                                                  \
} while (/* CONSTCOND */ 0)

#define DOLOGCOPY(src, srcoffset, dst, attr, memfunc)                          \
do {                                                                           \
   if ((src)->attr.filenoc > 0) {                                              \
      size_t i;                                                                \
                                                                               \
      DOCOPY(src,                                                              \
             srcoffset,                                                        \
             dst,                                                              \
             attr.fnamev,                                                      \
             sizeof(*src->attr.fnamev) * src->attr.filenoc,                    \
             memfunc);                                                         \
                                                                               \
      for (i = 0; i < src->attr.filenoc; ++i)                                  \
         DOCOPY(src,                                                           \
                srcoffset,                                                     \
                dst,                                                           \
                attr.fnamev[i],                                                \
             strlen((char *)((uintptr_t)src->attr.fnamev[i] + srcoffset)) + 1, \
                memfunc);                                                      \
                                                                               \
      DOCOPY(src,                                                              \
             srcoffset,                                                        \
             dst,                                                              \
             attr.filenov,                                                     \
             sizeof(*src->attr.filenov) * src->attr.filenoc,                   \
             memfunc);                                                         \
                                                                               \
      DOCOPY(src,                                                              \
             srcoffset,                                                        \
             dst,                                                              \
             attr.createdv,                                                    \
             sizeof(*src->attr.createdv) * src->attr.filenoc,                  \
             memfunc);                                                         \
   }                                                                           \
} while (/* CONSTCOND */ 0)


size_t
pointer_size(config)
   struct config *config;
{

   return pointer_copyorsize(SIZE, config, 0, NULL, NULL, 0);
}

int
pointer_copy(src, srcoffset, dst, mem, memsize)
   struct config *src;
   const ptrdiff_t srcoffset;
   struct config *dst;
   void *mem;
   const size_t memsize;
{
/*   const char *function = "pointer_copy()"; */

   SASSERTX(dst != NULL);

   return pointer_copyorsize(COPY,
                                   src,
                                   srcoffset,
                                   dst,
                                   mem,
                                   memsize);
}

static ssize_t
pointer_copyorsize(op, src, srcoffset, dst, mem, memsize)
   const configop_t op;
   struct config *src;
   const ptrdiff_t srcoffset;
   struct config *dst;
   void *mem;
   const size_t memsize;
{
   const char *function = "pointer_copyorsize()";
   void *(*memfunc)(size_t size) = (memsize == 0 ? malloc : getmem);
   rule_t *srcrulev[]   = { src->crule, src->hrule, src->srule };
   ssize_t size;
   size_t i;
   int docopy = 0, dofree = 0, dosize = 0;

   switch (op) {
      case COPY:
         docopy = 1;
         break;

      case SIZE:
         dosize = 1;
         break;

      default:
         SERRX(op);
   }

   slog(LOG_DEBUG,
        "%s: docopy = %d, dofree = %d, dosize = %d, "
        "mem = %p, memsize = %lu, offset = %ld",
        function,
        docopy,
        dofree,
        dosize,
        mem,
        (unsigned long)memsize,
        (long)srcoffset);

   /*
    * regardless of whether it's copy or size as we need poolinit() to
    * calculate required alignment/padbytes, or the calculated size will
    * be to low.
    */
   poolinit(mem, memsize);

   /*
    * Then go through all the individual attributes and copy/free/size the
    * pointed to memory.  Better keep our tongue straight in our mouth here,
    * as the local saying goes.
    */

   size = 0;

   if (src->internal.addrc > 0) {
      switch (op) {
         case COPY:
            DOCOPY(src,
                   srcoffset,
                   dst,
                   internal.addrv,
                   sizeof(*src->internal.addrv) * src->internal.addrc,
                   memfunc);
            break;

         case SIZE:
            ADDLEN(sizeof(*src->internal.addrv) * src->internal.addrc, &size);
            break;
      }
   }

   if (src->external.addrc > 0) {
      switch (op) {
         case COPY:
            DOCOPY(src,
                   srcoffset,
                   dst,
                   external.addrv,
                   sizeof(*src->external.addrv) * src->external.addrc,
                   memfunc);
            break;

         case SIZE:
            ADDLEN(sizeof(*src->external.addrv) * src->external.addrc, &size);
            break;

      }
   }

   for (i = 0; i < ELEMENTS(srcrulev); ++i) {
      struct rule_t *dstrule; /* false gcc warning: may be used uninitialized */
      struct rule_t *srcrule = srcrulev[i];

      if (srcrule == NULL)
         continue;

      switch (op) {
         case COPY:
            /*
             * Init stepwise so that the head of the next rule type starts
             * after all rules of previous type.  This is just to ease
             * debugging.
             */
            DOOFFSET(srcrule, srcoffset);

            switch (srcrule->type) {
               case object_crule:
                  DOCOPY(src,
                         srcoffset,
                         dst,
                         crule,
                         sizeof(*src->crule),
                         memfunc);
                  dstrule = dst->crule;
                  break;

#if HAVE_SOCKS_HOSTID
               case object_hrule:
                  DOCOPY(src,
                         srcoffset,
                         dst,
                         hrule,
                         sizeof(*src->hrule),
                         memfunc);
                  dstrule = dst->hrule;
                  break;
#endif /* HAVE_SOCKS_HOSTID */

               case object_srule:
                  DOCOPY(src,
                         srcoffset,
                         dst,
                         srule,
                         sizeof(*src->srule),
                         memfunc);
                  dstrule = dst->srule;
                  break;

               default:
                  SERRX(srcrule->type);
            }

            break;

         case SIZE:
            ADDLEN(sizeof(*srcrule), &size);
            break;
      }

      do {
         if (srcrule->socketoptionc > 0) {
            switch (op) {
               case COPY:
                  DOCOPY(srcrule,
                         srcoffset,
                         dstrule,
                         socketoptionv,
                       sizeof(*srcrule->socketoptionv) * srcrule->socketoptionc,
                         memfunc);
                   break;

               case SIZE:
                  ADDLEN(sizeof(*srcrule->socketoptionv)
                               * srcrule->socketoptionc,
                         &size);
                  break;
            }
         }

#if HAVE_LDAP
         switch (op) {
            case COPY:
               DOLINKCOPY(srcrule,
                          srcoffset,
                          dstrule,
                          state.ldapauthorisation.ldapurl,
                          memfunc);

               DOLINKCOPY(srcrule,
                          srcoffset,
                          dstrule,
                          state.ldapauthorisation.ldapbasedn,
                          memfunc);


               DOLINKCOPY(srcrule,
                          srcoffset,
                          dstrule,
                          state.ldapauthorisation.ldapserver,
                          memfunc);

               DOLINKCOPY(srcrule,
                          srcoffset,
                          dstrule,
                          state.ldapauthentication.ldapurl,
                          memfunc);

               DOLINKCOPY(srcrule,
                          srcoffset,
                          dstrule,
                          state.ldapauthentication.ldapbasedn,
                          memfunc);

               DOLINKCOPY(srcrule,
                          srcoffset,
                          dstrule,
                          state.ldapauthentication.ldapserver,
                          memfunc);

               break;

            case SIZE:
               ADDLEN(linkedsize(srcrule->state.ldapauthorisation.ldapurl),
                      &size);

               ADDLEN(linkedsize(srcrule->state.ldapauthorisation.ldapbasedn),
                      &size);

               ADDLEN(linkedsize(srcrule->state.ldapauthorisation.ldapserver),
                      &size);

               ADDLEN(linkedsize(srcrule->state.ldapauthentication.ldapurl),
                      &size);

               ADDLEN(linkedsize(srcrule->state.ldapauthentication.ldapbasedn),
                      &size);

               ADDLEN(linkedsize(srcrule->state.ldapauthentication.ldapserver),
                      &size);

               break;
         }

#endif /* HAVE_LDAP */

         switch (op) {
            case COPY:
               DOLINKCOPY(srcrule, srcoffset, dstrule, user, memfunc);
               DOLINKCOPY(srcrule, srcoffset, dstrule, group, memfunc);
               break;

            case SIZE:
               ADDLEN(linkedsize(srcrule->user), &size);
               ADDLEN(linkedsize(srcrule->group), &size);
               break;
         }

#if HAVE_LDAP
         switch (op) {
            case COPY:
               DOLINKCOPY(srcrule, srcoffset, dstrule, ldapgroup, memfunc);
               break;

            case SIZE:
               ADDLEN(linkedsize(srcrule->ldapgroup), &size);
               break;
         }
#endif /* HAVE_LDAP */

         switch (op) {
            case COPY:
               if (srcrule->next == NULL)
                  dstrule->next = NULL;
               else
                  DOCOPY(srcrule,
                         srcoffset,
                         dstrule,
                         next,
                         sizeof(*srcrule->next),
                         memfunc);

               srcrule = srcrule->next;
               dstrule = dstrule->next;
               break;

            case SIZE:
               if (srcrule->next != NULL)
                  ADDLEN(sizeof(*srcrule), &size);

               srcrule = srcrule->next;
               break;

         }
      } while (srcrule != NULL);
   }

   if (src->route != NULL) {
      route_t *dstroute; /* false gcc warning: may be used uninitialized */
      route_t *srcroute;

      switch (op) {
         case COPY:
            DOCOPY(src, srcoffset, dst, route, sizeof(*src->route), memfunc);
            dstroute = dst->route;
            break;

         case SIZE:
            ADDLEN(sizeof(*srcroute), &size);
            break;
      }

      srcroute = src->route;

      do {
         switch (op) {
            case COPY:
               DOCOPY(srcroute,
                      srcoffset,
                      dstroute,
                      socketoptionv,
                     sizeof(*srcroute->socketoptionv) * srcroute->socketoptionc,
                      memfunc);

               if (srcroute->next == NULL)
                  dstroute->next = NULL;
               else
                  DOCOPY(srcroute,
                         srcoffset,
                         dstroute,
                         next,
                         sizeof(*srcroute->next),
                         memfunc);

               dstroute = dstroute->next;
               srcroute = srcroute->next;
               break;

            case SIZE:
               ADDLEN(sizeof(*srcroute->socketoptionv)
                            * srcroute->socketoptionc,
                      &size);

               if (srcroute->next != NULL)
                  ADDLEN(sizeof(*srcroute), &size);

               srcroute = srcroute->next;
               break;
         }
      } while (srcroute != NULL);
   }

   if (src->monitor != NULL) {
      monitor_t *dstmonitor; /* false gcc warning: may be used uninitialized */
      monitor_t *srcmonitor;

      switch (op) {
         case COPY:
            DOCOPY(src,
                   srcoffset,
                   dst,
                   monitor,
                   sizeof(*src->monitor),
                   memfunc);

            dstmonitor = dst->monitor;
            break;

         case SIZE:
            ADDLEN(sizeof(*srcmonitor), &size);
            break;
      }

      srcmonitor = src->monitor;
      do {
         switch (op) {
            case COPY:
               if (srcmonitor->next == NULL)
                  dstmonitor->next = NULL;
               else
                  DOCOPY(srcmonitor,
                         srcoffset,
                         dstmonitor,
                         next,
                         sizeof(*dstmonitor->next),
                         memfunc);

               dstmonitor = dstmonitor->next;
               srcmonitor = srcmonitor->next;
               break;

            case SIZE:
               if (srcmonitor->next != NULL)
                  ADDLEN(sizeof(*srcmonitor), &size);

               srcmonitor = srcmonitor->next;
               break;
         }
      } while (srcmonitor != NULL);
   }

   if (src->socketoptionc > 0) {
      switch (op) {
         case COPY:
            DOCOPY(src,
                   srcoffset,
                   dst,
                   socketoptionv,
                   sizeof(*src->socketoptionv) * src->socketoptionc,
                   memfunc);
            break;

         case SIZE:
            ADDLEN(sizeof(*src->socketoptionv) * src->socketoptionc,
                   &size);
            break;
      }
   }

   if (docopy) {
      dst->oldshmemv  = sockscf.oldshmemv;  /* process-local. */
      dst->stat       = sockscf.stat;       /* process-local. */
      dst->state      = sockscf.state;      /* process-local. */
      dst->child      = sockscf.child;      /* process-local. */
   }

   switch (op) {
      case COPY:
         DOLOGCOPY(src, srcoffset, dst, errlog, memfunc);
         DOLOGCOPY(src, srcoffset, dst, log, memfunc);
         break;

      case SIZE: {
         logtype_t *logv[] = { &src->errlog, &src->log };

         for (i = 0; i < ELEMENTS(logv); ++i) {
            if (logv[i]->filenoc > 0) {
               size_t ii;

               ADDLEN(sizeof((*logv[i]->fnamev))   * logv[i]->filenoc, &size);
               ADDLEN(sizeof(*(logv[i]->createdv)) * logv[i]->filenoc, &size);
               ADDLEN(sizeof(*(logv[i]->filenov))  * logv[i]->filenoc, &size);

               for (ii = 0; ii < logv[i]->filenoc; ++ii)
                  ADDLEN(strlen(logv[i]->fnamev[ii]) + 1, &size);
            }
         }
         break;
      }
   }

   slog(LOG_DEBUG, "%s: extra bytes used due to alignment issues: %lu",
        function, (unsigned long)_extra_due_alignment);

   if (op == COPY) {
      if (memleft_no_alignment_luck() == 0)
         slog(LOG_DEBUG,
              "%s: all allocated bytes used, as expected", function);
      else
         swarnx("%s: %lu byte%s of memory left in pool after copying to dst",
                function,
                (unsigned long)memleft_no_alignment_luck(),
                memleft_no_alignment_luck() == 1 ? "" : "s");
   }

   if (dosize)
      return size;
   else
      return 0;
}

size_t
compareconfigs(a, b)
   const struct config *a;
   const struct config *b;
{
#define CHECKRESULT(rc, attr, size)                                            \
do {                                                                           \
   if ((rc) != 0) {                                                            \
      swarnx("%s: %lu byte%s compare on line %d says attribute \"%s\" is not " \
             "identical in both config objects",                               \
             function,                                                         \
             (unsigned long)(size),                                            \
             size == 1 ? "" : "s",                                             \
             __LINE__,                                                         \
             #attr);                                                           \
                                                                               \
             return 0;                                                         \
   }                                                                           \
                                                                               \
   compared += size;                                                           \
} while (/* CONSTCOND */ 0)

#define PTRCHECK(a, b, attr)                                                   \
do {                                                                           \
   if ((a->attr == NULL && b->attr != NULL)                                    \
   ||  (a->attr != NULL && b->attr == NULL)) {                                 \
      swarnx("%s: \"%s\"-attribute in objects are not the same.  "             \
             "In \"%s\" \"%s\" is %s NULL pointer, while in \"%s\" it %s",     \
             function,                                                         \
             #attr,                                                            \
             #a,                                                               \
             #attr,                                                            \
             a->attr == NULL ? "a"  : "is not a",                              \
             #b,                                                               \
             b->attr == NULL ? "is" : "is not");                               \
                                                                               \
             return 0;                                                         \
   }                                                                           \
} while (/* CONSTCOND */ 0)

#define EQCHECK_PTR(a, b, attr, size)                                          \
do {                                                                           \
   int rc;                                                                     \
                                                                               \
   PTRCHECK((a), (b), attr);                                                   \
   if ((a)->attr != NULL) {                                                    \
      rc = memcmp((a)->attr, (b)->attr, (size));                               \
      CHECKRESULT(rc, attr, size);                                             \
   }                                                                           \
} while (/* CONSTCOND */ 0)

#define EQCHECK(a, b, attr)                                                    \
do {                                                                           \
   size_t size;                                                                \
   int rc;                                                                     \
                                                                               \
   size = sizeof(a->attr);                                                     \
   rc   = memcmp(&a->attr, &b->attr, size);                                    \
                                                                               \
   CHECKRESULT(rc, attr, size);                                                \
                                                                               \
} while (/* CONSTCOND */ 0)

#define LINKCHECK(lsrc, ldst, lattr)                                           \
do {                                                                           \
   PTRCHECK(lsrc, ldst, lattr);                                                \
   if (lsrc->lattr != NULL) {                                                  \
      linkedname_t *srclink = lsrc->lattr;                                     \
      linkedname_t *dstlink = ldst->lattr;                                     \
                                                                               \
      do {                                                                     \
         EQCHECK_PTR(srclink,                                                  \
                     dstlink,                                                  \
                     name,                                                     \
                     strlen(srclink->name) + 1);                               \
         PTRCHECK(srclink, dstlink, next);                                     \
                                                                               \
         srclink = srclink->next;                                              \
         dstlink = dstlink->next;                                              \
      } while (srclink != NULL);                                               \
   }                                                                           \
} while (/* CONSTCOND */ 0)

   const char *function = "compareconfigs()";
   const rule_t *arulev[] = { a->crule,   a->hrule,   a->srule },
                *brulev[] = { b->crule,   b->hrule,   b->srule };
   const logtype_t *alogv[] = { &a->errlog, &a->log },
                   *blogv[] = { &b->errlog, &b->log };
   size_t i, compared = 0;

   /*
    * Check all pointers and structs, as that's where an error is likely
    * to occur.  Also check non-pointers where easy to do, though it should
    * be almost impossible for an error to exists there due to the initial
    * struct assignment between 'a' and 'b'.
    */

   EQCHECK(a, b, initial);

   EQCHECK_PTR(a,
               b,
               internal.addrv,
               sizeof(*a->internal.addrv) * a->internal.addrc);

   EQCHECK_PTR(a,
               b,
               external.addrv,
               sizeof(*a->external.addrv) * a->external.addrc);
   EQCHECK(a, b, cpu);

   PTRCHECK(a, b, crule);
   PTRCHECK(a, b, hrule);
   PTRCHECK(a, b, srule);
   for (i = 0; i < ELEMENTS(arulev); ++i) {
      const rule_t *arule = arulev[i], *brule = brulev[i];
      rule_t tmprulea, tmpruleb;

      if (arulev[i] == NULL)
         continue;

      SASSERTX(arule != NULL);
      SASSERTX(brule != NULL);

      do {
         EQCHECK_PTR(arule,
                     brule,
                     socketoptionv,
                     sizeof(*arule->socketoptionv) * arule->socketoptionc);

         EQCHECK(arule, brule, src);
         EQCHECK(arule, brule, dst);
         EQCHECK(arule, brule, rdr_from);
         EQCHECK(arule, brule, rdr_to);
         EQCHECK(arule, brule, hostidoption_isset);
#if HAVE_SOCKS_HOSTID
         EQCHECK(arule, brule, hostid);
         EQCHECK(arule, brule, hostindex);
#endif /* HAVE_SOCKS_HOSTID */

         EQCHECK(arule, brule, log);
         EQCHECK(arule, brule, number);
         EQCHECK(arule, brule, linenumber);

         /*
          * mostly memory, but a few pointers too; just copy their address.
          */

         tmprulea = *arule;
         tmpruleb = *brule;

#if HAVE_LDAP

         tmprulea.state.ldapauthorisation.ldapurl
         = tmpruleb.state.ldapauthorisation.ldapurl;

         tmprulea.state.ldapauthorisation.ldapbasedn
         = tmpruleb.state.ldapauthorisation.ldapbasedn;

         tmprulea.state.ldapauthorisation.ldapserver
         = tmpruleb.state.ldapauthorisation.ldapserver;

         tmprulea.state.ldapauthentication.ldapurl
         = tmpruleb.state.ldapauthentication.ldapurl;

         tmprulea.state.ldapauthentication.ldapbasedn 
         = tmpruleb.state.ldapauthentication.ldapbasedn;

         tmprulea.state.ldapauthentication.ldapserver 
         = tmpruleb.state.ldapauthentication.ldapserver;

#endif /* HAVE_LDAP */

         EQCHECK((&tmprulea), (&tmpruleb), state);

#if HAVE_LDAP

         LINKCHECK(arule, brule, state.ldapauthorisation.ldapurl);
         LINKCHECK(arule, brule, state.ldapauthorisation.ldapbasedn);
         LINKCHECK(arule, brule, state.ldapauthorisation.ldapserver);
         LINKCHECK(arule, brule, state.ldapauthentication.ldapurl);
         LINKCHECK(arule, brule, state.ldapauthentication.ldapbasedn);
         LINKCHECK(arule, brule, state.ldapauthentication.ldapserver);


#endif /* HAVE_LDAP */

         EQCHECK(arule, brule, timeout);
         LINKCHECK(arule, brule, user);
         LINKCHECK(arule, brule, group);

#if HAVE_LDAP

         LINKCHECK(arule, brule, ldapgroup);
         EQCHECK(arule, brule, ldapsettingsfromuser);

#endif /* HAVE_LDAP */

#if HAVE_LIBWRAP

         EQCHECK(arule, brule, libwrap);

#endif /* HAVE_LIBWRAP */

         EQCHECK(arule, brule, bw_shmid);
         EQCHECK(arule, brule, mstats_shmid);
         EQCHECK(arule, brule, ss_shmid);

         PTRCHECK(arule, brule, next);
         arule = arule->next;
         brule = brule->next;
      } while (arule != NULL);
   }

   EQCHECK(a, b, routeoptions);
   PTRCHECK(a, b, route);
   if (a->route != NULL) {
      route_t *aroute = a->route,
              *broute = b->route;

      do {
         EQCHECK(aroute, broute, number);
         EQCHECK(aroute, broute, state);

         EQCHECK(aroute, broute, socketoptionc);
         EQCHECK_PTR(aroute,
                     broute,
                     socketoptionv,
                     sizeof(*aroute->socketoptionv) * aroute->socketoptionc);

         EQCHECK(aroute, broute, src);
         EQCHECK(aroute, broute, dst);
         EQCHECK(aroute, broute, gw);
         EQCHECK(aroute, broute, rdr_from);

         PTRCHECK(aroute, broute, next);
         aroute = aroute->next;
         broute = broute->next;
      } while (aroute != NULL);
   }

   if (a->monitor != NULL) {
      monitor_t *amon = a->monitor, *bmon = b->monitor;

      do {
         EQCHECK(amon, bmon, number);
         EQCHECK(amon, bmon, mstats_shmid);
         EQCHECK(amon, bmon, src);
         EQCHECK(amon, bmon, dst);

#if HAVE_SOCKS_HOSTID
         EQCHECK(amon, bmon, hostid);
         EQCHECK(amon, bmon, hostindex);
#endif /* HAVE_SOCKS_HOSTID */

         PTRCHECK(amon, bmon, next);
         amon = amon->next;
         bmon = bmon->next;
      } while (amon != NULL);
   }


   EQCHECK(a, b, socketoptionc);
   EQCHECK_PTR(a,
               b,
               socketoptionv,
               sizeof(*a->socketoptionv) * a->socketoptionc);

   EQCHECK(a, b, compat);
   EQCHECK(a, b, extension);

   for (i = 0; i < ELEMENTS(alogv); ++i) {
      const logtype_t *alog = alogv[i], *blog = blogv[i];

      EQCHECK(alog, blog, type);

      PTRCHECK(alog, blog, filenov);
      PTRCHECK(alog, blog, createdv);

      if (alog->fnamev != NULL) {
         size_t ii;

         for (ii = 0; ii < alog->filenoc; ++ii)
            EQCHECK_PTR(alog,
                        blog,
                        fnamev[ii],
                        strlen(alog->fnamev[ii]) + 1);

         SASSERTX(alog->filenov != NULL);
         EQCHECK_PTR(alog,
                     blog,
                     filenov,
                     sizeof(*alog->filenov) * alog->filenoc);

         SASSERTX(alog->createdv != NULL);
         EQCHECK_PTR(alog,
                     blog,
                     createdv,
                     sizeof(*alog->createdv) * alog->filenoc);
      }

      PTRCHECK(alog, blog, fnamev);

      EQCHECK(alog, blog, filenoc);
      EQCHECK(alog, blog, facility);
      EQCHECK(alog, blog, facilityname);
   }

   EQCHECK(a, b, srchost);

#if !HAVE_PRIVILEGES
   EQCHECK(a, b, uid);
#endif /* !HAVE_PRIVILEGES */

   EQCHECK(a, b, cmethodv);
   EQCHECK(a, b, cmethodc);

   EQCHECK(a, b, smethodv);
   EQCHECK(a, b, smethodc);

   EQCHECK(a, b, udpconnectdst);

   /* hosts_{allow,deny}_old; set at startup only. */

   return compared;
}

static size_t
linkedsize(list)
   const linkedname_t *list;
{
   const char *function = "linkedsize()";
   size_t size = 0;

   if (list == NULL)
      return 0;

   size += sizeof(*list); /* first link it's up to caller to align. */
   ADDLEN(strlen(list->name) + 1 /* NUL */, &size);
   list = list->next;

   while (list != NULL) {
      ADDLEN(sizeof(*list), &size);
      ADDLEN(strlen(list->name) + 1 /* NUL */, &size);
      list = list->next;
   }

   return size;
}

static void
poolinit(pool, size)
   void *pool;
   const size_t size;
{
   const char *function = "poolinit()";

   /*
    * try to figure out what the strictest *pointer* alignment is so that
    * getmem() can return properly aligned data.
    */
   const void *_voidv[2];
   _mask       = ~(((uintptr_t)&_voidv[1] - (uintptr_t)&_voidv[0]) - 1);
   _alignment  = ((uintptr_t)&_voidv[1] - (uintptr_t)&_voidv[0]),

    _nextstart = pool;
   _left       = _left_no_alignment_luck = size;

   slog(LOG_DEBUG,
        "%s: memorypoolsize is %lu, starting at address %p.  "
        "Alignment is on %lu-byte boundary",
        function, (unsigned long)size, pool, (unsigned long)_alignment);
}

void *
getmem(size)
   const size_t size;
{
   const char *function = "getmem()";
   void *start;
   size_t size_with_padding;


   /*
    * technically we could do with adding (_alignment - 1 bytes) and instead
    * a check for testing what the non-aligned address happens to be, since
    * it could happen to be aligned.  That would prevent us from easily
    * checking that the amount of memory allocated and the memory we calculated
    * as the required amount is the same however, so ignore that minor
    * optimization.
    */
   start = (void *)(((uintptr_t)(_nextstart) + _alignment) & _mask);
   size_with_padding = ((uintptr_t)start - (uintptr_t)_nextstart) + size;

   slog(LOG_DEBUG, "%s: size %lu (with padding: %lu)",
        function, (unsigned long)size, (unsigned long)size_with_padding);

   if (_left < size_with_padding)
      return NULL;

   _extra_due_alignment    += size_with_padding - size;
   _left_no_alignment_luck -= size + _alignment;

   _left      -= size_with_padding;
   _nextstart  = (void *)((uintptr_t)_nextstart + size_with_padding);

   SASSERTX(((uintptr_t)start % _alignment) == 0);
   return start;
}

void
doconfigtest(void)
{
   const char *function = "doconfigtest()";
   static struct config *shmem;
   size_t size, pointedto, mempointedto;
   struct config *mem;

   slog(LOG_DEBUG,
        "%s: doing test with memory preallocated for pointercontents",
        function);

   pointedto = pointer_size(&sockscf);
   slog(LOG_DEBUG, "%s: size of config: %lu, pointed to size: %lu",
        function, (unsigned long)sizeof(sockscf), (unsigned long)pointedto);

   size = sizeof(sockscf) + pointedto;
   if ((shmem = malloc(size)) == NULL)
      serrx("%s: could not malloc(3) %lu bytes", function, (unsigned long)size);


   /*
    * Shallow copy what we can.
    */
   *shmem = sockscf;

   /*
    * Then do a deep copy of the rest.
    */
   if (pointer_copy(&sockscf,
                    0x0,
                    shmem,
                    (char *)shmem + sizeof(sockscf),
                    size          - sizeof(sockscf)) == 0)
      slog(LOG_DEBUG, "%s: pointer_copy() successful", function);
   else
      serr("%s: pointer_copy() failed", function);

   if ((size = compareconfigs(&sockscf, shmem)) == 0)
      swarnx("%s: created config not identical to original config", function);
   else
      slog(LOG_DEBUG,
           "%s: created config identical to original, %lu bytes compared",
           function, (unsigned long)size);

   /*
    * Try to copy again, this time from memory we copied to.
    */

   slog(LOG_DEBUG, "%s: doing test #2 with memory preallocated for pointers",
        function);

   mempointedto = pointer_size(shmem);

   if (mempointedto != pointedto) {
      swarnx("%s: memory pointed to by shmem is %lu, but expected %lu",
             function, (unsigned long)mempointedto, (unsigned long)pointedto);

      free(shmem);
      return;
   }

   size = sizeof(sockscf) + mempointedto;
   if ((mem = malloc(size)) == NULL)
      serrx("%s: could not malloc(3) %lu bytes", function, (unsigned long)size);


   /*
    * Shallow copy what we can.
    */
   *mem = *shmem;

   /*
    * Then do a deep copy of the rest.
    */
   if (pointer_copy(shmem,
                    0x0,
                    mem,
                    (char *)mem + sizeof(sockscf),
                    size - sizeof(sockscf)) == 0)
      slog(LOG_DEBUG, "%s: pointer_copy() successful", function);
   else
      serr("%s: pointer_copy() failed", function);

   if ((size = compareconfigs(shmem, mem)) == 0)
      swarnx("%s: created config not identical to original config", function);
   else
      slog(LOG_DEBUG,
           "%s: created config is identical to original.  %lu bytes compared",
           function, (unsigned long)size);

   free(shmem);
   free(mem);

   /*
    * test without preallocating memory for pointercontents.
    */

   slog(LOG_DEBUG,
        "%s: doing test *without* memory preallocated for pointercontents",
        function);

   size = sizeof(sockscf);
   if ((shmem = malloc(size)) == NULL)
      serrx("%s: could not malloc(3) %lu bytes", function, (unsigned long)size);


   /*
    * Shallow copy what we can.
    */
   *shmem = sockscf;

   /*
    * Then do a deep copy of the rest.
    */
   if (pointer_copy(&sockscf,
                    0x0,
                    shmem,
                    NULL,
                    0) == 0)
      slog(LOG_DEBUG, "%s: pointer_copy() successful", function);
   else
      serr("%s: pointer_copy() failed", function);

   if ((size = compareconfigs(&sockscf, shmem)) == 0)
      swarnx("%s: created config not identical to original config", function);
   else
      slog(LOG_DEBUG,
           "%s: created config is identical to original.  %lu bytes compared",
           function, (unsigned long)size);

   /* needs to be handled specially. */
   sockd_freelogobject(&shmem->log, 0 /* just test. */);
   sockd_freelogobject(&shmem->errlog, 0 /* just test. */);

   /* free(shmem).  Should free shmem->pointers too, but can not. */
}

#if 0
static size_t
memleft(void)
{
   return _left;
}
#endif

static size_t
memleft_no_alignment_luck(void)
{
   return _left_no_alignment_luck;
}

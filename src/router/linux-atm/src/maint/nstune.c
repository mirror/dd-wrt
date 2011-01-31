/******************************************************************************
 *
 * nstune.c
 *
 * User level utility to tune the NICStAR device driver.
 *
 * Author: Rui Prior
 *
 * (C) INESC 1998
 *
 ******************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/time.h>
#include <linux/atm_nicstar.h>
#include <linux/atm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>


static void usage(const char *name)
{
   fprintf(stderr, "Set buffer level marks: \n");
   fprintf(stderr, "%s itf {s|l|h|i} min init max \n", name);
   fprintf(stderr, "Set buffer level to init mark: \n");
   fprintf(stderr, "%s itf {s|l|h|i} \n", name);
   fprintf(stderr, "\n");
   exit(1);
}


int main(int argc, char **argv)
{
   char *name, *end;
   int itf, min, init, max;
   int s;
   pool_levels pl;
   int btype;
   struct atmif_sioc sioc;

   name = argv[0];
   if (argc != 6 && argc != 3)
      usage(name);
   itf = strtol(argv[1], &end, 0);
   if (end == argv[1] || itf < 0)
      usage(name);
   if (argc == 6)
   {
      min = strtol(argv[3], &end, 0);
      if (end == argv[3] || min <= 0)
         usage(name);
      init = strtol(argv[4], &end, 0);
      if (end == argv[4] || init <= 0)
         usage(name);
      max = strtol(argv[5], &end, 0);
      if (end == argv[5] || max <= 0)
         usage(name);
      if (min >= init || init >= max)
         usage(name);
      switch(*argv[2])
      {
         case 's':
            pl.buftype = NS_BUFTYPE_SMALL;
            break;
         case 'l':
            pl.buftype = NS_BUFTYPE_LARGE;
            break;
         case 'h':
            pl.buftype = NS_BUFTYPE_HUGE;
            break;
         case 'i':
            pl.buftype = NS_BUFTYPE_IOVEC;
            break;
         default:
            usage(name);
      }
   
      sioc.number = itf;
      sioc.arg = &pl;
      sioc.length = sizeof(pl);
      pl.level.min = min;
      pl.level.init = init;
      pl.level.max = max;

      s = socket(PF_ATMPVC, SOCK_DGRAM, 0);
      if (s < 0)
      {
         perror("socket");
         return 2;
      }
      if (ioctl(s, NS_SETBUFLEV, &sioc) < 0)
      {
         perror("ioctl NS_SETBUFLEV");
         return 3;
      }

   }
   else /* argc == 2 */
   {
      switch(*argv[2])
      {
         case 's':
            btype = NS_BUFTYPE_SMALL;
            break;
         case 'l':
            btype = NS_BUFTYPE_LARGE;
            break;
         case 'h':
            btype = NS_BUFTYPE_HUGE;
            break;
         case 'i':
            btype = NS_BUFTYPE_IOVEC;
            break;
         default:
            usage(name);
      }

      sioc.number = itf;
      sioc.arg = (void *) btype;
      sioc.length = sizeof(void *);

      s = socket(PF_ATMPVC, SOCK_DGRAM, 0);
      if (s < 0)
      {
         perror("socket");
         return 2;
      }
      if (ioctl(s, NS_ADJBUFLEV, &sioc) < 0)
      {
         perror("ioctl NS_ADJBUFLEV");
         return 3;
      }

   }

   return 0;
}

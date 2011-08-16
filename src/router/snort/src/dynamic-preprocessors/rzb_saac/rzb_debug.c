#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "rzb_debug.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

void prettyprint(const unsigned char *data, unsigned int size) {
   unsigned int i;
   const unsigned char *dataptr = data;
   unsigned char asciigraph[17];

   memset(asciigraph, '\x00', 17);

#ifdef PACKETDUMPSIZE
   size = (size > PACKETDUMPSIZE) ? PACKETDUMPSIZE : size;
#endif

   for(i=0; i < size; i++, dataptr++) {
      printf("%02x ", *dataptr);
      asciigraph[i % 16] = (isgraph(*dataptr) || (*dataptr == ' ')) ? *dataptr : '.';

      if(i % 16 == 15) {
         printf("%s\n", asciigraph);
         memset(asciigraph, '\x00', 17);
      }
   }

   // Dump any remaining data
   if(i % 16) {
      printf("%*s", (16 - (i%16)) * 3, " ");
      printf("%s\n", asciigraph);
   }
}


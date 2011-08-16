/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
/*
  sfmemcap.c

  These functions wrap the alloc & free functions. They enforce a memory cap using
  the MEMCAP structure.  The MEMCAP structure tracks memory usage.  Each allocation
  has 4 bytes added to it so we can store the allocation size.  This allows us to 
  free a block and accurately track how much memory was recovered.
  
  Marc Norton  
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sfmemcap.h"
#include "util.h"

/*
*   Set max # bytes & init other variables.
*/
void sfmemcap_init( MEMCAP * mc, unsigned nbytes )
{
	mc->memcap = nbytes;
	mc->memused= 0;
	mc->nblocks= 0;
}

/*
*   Create and Init a MEMCAP -  use free to release it
*/
MEMCAP * sfmemcap_new( unsigned nbytes )
{
	 MEMCAP * mc;

	 mc = (MEMCAP*)calloc(1,sizeof(MEMCAP));

         if( mc ) sfmemcap_init( mc, nbytes );
	 
	 return mc;
}

/*
*  Release the memcap structure
*/
void sfmemcap_delete( MEMCAP * p )
{
     if(p)free( p );
}

/*
*  Allocate some memory
*/
void * sfmemcap_alloc( MEMCAP * mc, unsigned nbytes )
{
   long * data;

   //printf("sfmemcap_alloc: %d bytes requested, memcap=%d, used=%d\n",nbytes,mc->memcap,mc->memused);

   nbytes += sizeof(long);


   /* Check if we are limiting memory use */
   if( mc->memcap > 0 )
   {
      /* Check if we've maxed out our memory - if we are tracking memory */
      if( (mc->memused + nbytes) > mc->memcap )
      {
	      return 0;
      }
   }

   //data = (long *) malloc( nbytes );
   data = (long *)SnortAlloc( nbytes );

   if( data == NULL )
   {
        return 0;
   }

   *data++ = (long)nbytes;

   mc->memused += nbytes;
   mc->nblocks++;

   return data;
}

/*
*   Free some memory
*/
void sfmemcap_free( MEMCAP * mc, void * p )
{
   long * q;

   q = (long*)p;
   q--;
   mc->memused -= (unsigned)(*q);
   mc->nblocks--;

   free(q);
}

/*
*   For debugging.
*/
void sfmemcap_showmem( MEMCAP * mc )
{
     fprintf(stderr, "memcap: memcap = %u bytes,",mc->memcap);
     fprintf(stderr, " memused= %u bytes,",mc->memused);
     fprintf(stderr, " nblocks= %d blocks\n",mc->nblocks);
}

/*
*  String Dup Some memory.
*/
char * sfmemcap_strdup( MEMCAP * mc, const char *str )
{
    char *data = NULL;
    int data_size;

    data_size = strlen(str) + 1;
    data = (char *)sfmemcap_alloc(mc, data_size);

    if(data == NULL)
    {
        return  0 ;
    }

    SnortStrncpy(data, str, data_size);

    return data;
}

/*
*  Dup Some memory.
*/
void * sfmemcap_dupmem( MEMCAP * mc, void * src, int n )
{
    void * data = (char *)sfmemcap_alloc( mc, n );
    if(data == NULL)
    {
        return  0;
    }

    memcpy( data, src, n );

    return data;
}

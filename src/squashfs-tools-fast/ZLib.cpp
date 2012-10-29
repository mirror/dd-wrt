/*
 * lzma zlib simplified wrapper
 *
 * Copyright (c) 2005-2006 Oleg I. Vdovikin <oleg@cs.msu.su>
 *
 * This library is free software; you can redistribute 
 * it and/or modify it under the terms of the GNU Lesser 
 * General Public License as published by the Free Software 
 * Foundation; either version 2.1 of the License, or 
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU Lesser General Public License 
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General 
 * Public License along with this library; if not, write to 
 * the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA 02111-1307 USA 
 *
 * 07/10/06 - jc - Added LZMA encoding parameter specification (_LZMA_PARAMS)
 *				   contact: jeremy@bitsum.com
 */

/*
 * default values for encoder/decoder used by wrapper
 */

#include <zlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
/* jc: undef to kill compress2_lzma */
#define _LZMA_PARAMS 

#define ZLIB_LC 3
#define ZLIB_LP 0
#define ZLIB_PB 2
#define ZLIB_FB 128 /* jc: add default fast bytes param */

#ifdef WIN32
#include <initguid.h>
#else
#define INITGUID
#endif

#include "MyWindows.h"
#include "lzmalib/LzmaLib.h"

#define STG_E_SEEKERROR                  ((HRESULT)0x80030019L)
#define STG_E_MEDIUMFULL                 ((HRESULT)0x80030070L)



#ifdef _LZMA_PARAMS



/* jc: new compress2 proxy that allows lzma param specification */
extern "C" int compress2_lzma_test (Bytef *dest,   size_t *destLen,
                                  	const Bytef *source, uLong sourceLen,
                                  	int level, int fb, int lc, int lp, int pb)
{	
	size_t os = LZMA_PROPS_SIZE;
	unsigned char outprops[LZMA_PROPS_SIZE];
    	if(pb<0) pb=ZLIB_PB;
	if(lc<0) lc=ZLIB_LC;
	if(lp<0) lp=ZLIB_LP;
	if(fb<0) fb=ZLIB_FB;

	unsigned int dictSize = 1 << (level + 14);
	unsigned int reduceSize = sourceLen;

	if (dictSize > reduceSize) {
		unsigned i;
		for (i = 15; i <= 30; i++) {
			if (reduceSize <= ((unsigned int) 2 << i)) {
				dictSize = ((unsigned int) 2 << i);
				break;
			}
			if (reduceSize <= ((unsigned int) 3 << i)) {
				dictSize = ((unsigned int) 3 << i);
				break;
			}
		}
	}


	LzmaCompress(dest,destLen,source,sourceLen,outprops,&os,level,dictSize,lc,lp,pb,fb,2);
	
	return Z_OK;
}

#include <malloc.h>
#include <stdio.h>
#include <pthread.h>


unsigned char pbmatrix[3]={0,1,2};
unsigned char lcmatrix[4]={0,1,2,3};
unsigned char lpmatrix[4]={0,1,2,3};

struct MATRIXENTRY
{
int pb;
int lc;
int lp;
};

struct MATRIXENTRY matrix[]={
{2,0,0},
{2,0,1},
{2,0,2},
{2,1,0},
{2,1,2},
{2,2,0},
{2,3,0},
{0,2,0},
{0,2,1},
{0,3,0},
{0,0,0},
{0,0,2},
{1,0,1},
{1,2,0},
{1,3,0}
};



int pbsave = -1;
int lcsave = -1;
int lpsave = -1;



int testlevel;
int testfb;
pthread_spinlock_t	pos_mutex;
Bytef *test1;
const Bytef *testsource;

size_t test2len;
size_t test1len;
size_t testsourcelen;
extern "C" void *brute(void *arg)
{
//	int oldstate;
	size_t test3len = test2len;
//	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
//	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldstate);
	int *count = (int*)arg;
	int testcount=count[0];
int takelcvalue = matrix[testcount].lc;
int takepbvalue = matrix[testcount].pb;
int takelpvalue = matrix[testcount].lp;
Bytef *test2 = (Bytef*)malloc(test2len);
//fprintf(stderr,"try method [pb:%d lc:%d lp:%d fb:%d]\n",takepbvalue,takelcvalue,takelpvalue,testfb);
int ret =  compress2_lzma_test(test2,&test3len,testsource,testsourcelen,testlevel,testfb,takelcvalue,takelpvalue,takepbvalue);
//fprintf(stderr,"test return %d\n",ret);
pthread_spin_lock(&pos_mutex);
if (test3len<test1len)
    {
    test1len = test3len;
    memcpy(test1,test2,test3len);
    pbsave = takepbvalue;
    lcsave = takelcvalue;
    lpsave = takelpvalue;
    }
pthread_spin_unlock(&pos_mutex);
free(test2);
return NULL;
}



extern "C" int compress2_lzma (Bytef *dest,   uLongf *destLen,
                                  	const Bytef *source, uLong sourceLen,
                                  	int level, int fb, int lc, int lp, int pb)
{
int i,a;
pthread_t *thread;
cpu_set_t cpuset[4];
//fprinttf(stderr,"source %d, dest %d\n",sourceLen, *destLen);
test1 = (Bytef*)malloc(*destLen);
test1len = *destLen+*destLen;
test2len = *destLen;
testsource = source;
testfb = fb;
testsourcelen = sourceLen;
testlevel = level;
pthread_spin_init(&pos_mutex,0);

//fprintf(stderr,"PTHR %d\n",PTHREAD_THREADS_MAX);
	if((thread = (pthread_t *)malloc((16) * sizeof(pthread_t))) == NULL)
		fprintf(stderr,"Out of memory allocating thread descriptors\n");
//for (a=0;a<2;a++)
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr,64*1024);

int threadcount=0;
#define MAX_THREADS 8
	int *argument=(int*)malloc(16*4);
for (a=0;a<15/MAX_THREADS;a++)
{
	for(i = 0; i < MAX_THREADS; i++) {
		argument[i]=threadcount++;
//		fprintf(stderr,"start thread %d\n",threadcount);
		if(pthread_create(&thread[i], &attr, brute, &argument[i]) != 0 )
			fprintf(stderr,"Failed to create thread\n");
	}
	for (i=0;i<MAX_THREADS;i++)
	    {
	    pthread_join(thread[i],NULL);
	    }
}

	for(i = 0; i < 15%MAX_THREADS; i++) {
		argument[i]=threadcount++;
//		fprintf(stderr,"start thread %d\n",threadcount);
		if(pthread_create(&thread[i], NULL, brute, &argument[i]) != 0 )
			fprintf(stderr,"Failed to create thread\n");
	}
	for (i=0;i<15%MAX_THREADS;i++)
	    {
	    pthread_join(thread[i],NULL);
	    }
	free(argument);
pthread_attr_destroy(&attr);
    fprintf(stderr,"use method [pb:%d lc:%d lp:%d fb:%d] (len %d)\n",pbsave,lcsave,lpsave,fb,test1len);
    memcpy(dest+4,test1,test1len);
    dest[0]=pbsave;
    dest[1]=lcsave;
    dest[2]=lpsave;
    dest[3]=fb;
    *destLen=test1len+4;
free(thread);
    pthread_spin_destroy(&pos_mutex);
    free(test1);
    return Z_OK;
}

ZEXTERN int ZEXPORT uncompress OF((Bytef *dest,   uLongf *destLen,
                                   const Bytef *source, uLong sourceLen))
{
size_t os = LZMA_PROPS_SIZE;
  unsigned char props[LZMA_PROPS_SIZE];
  props[0] = (unsigned char)((source[0] * 5 + source[2]) * 9 + source[1]);
  unsigned int dictSize = 1<<23;
  int i;
  for (i = 0; i < 4; i++)
    props[1 + i] = (unsigned char)(dictSize >> (8 * i));

  size_t slen = sourceLen-4;
  size_t dlen = *destLen;
  LzmaUncompress(dest, &dlen, &source[4], &slen,&props[0], LZMA_PROPS_SIZE);
	
	return Z_OK;
}


#endif


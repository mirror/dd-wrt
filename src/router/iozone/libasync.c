/* 
 * Library for Posix async read operations with hints.
 * Author: Don Capps
 * Company: Iozone
 * Date: 4/24/1998
 *
 * Two models are supported.  First model is a replacement for read() where the async
 * operations are performed and the requested data is bcopy()-ed back into the users 
 * buffer. The second model is a new version of read() where the caller does not 
 * supply the address of the buffer but instead is returned an address to the
 * location of the data. The second model eliminates a bcopy from the path.
 *
 * To use model #1:
 * 1. Call async_init(&pointer_on_stack,fd,direct_flag);
 *	The fd is the file descriptor for the async operations.
 *	The direct_flag sets VX_DIRECT 
 *
 * 2. Call async_read(gc, fd, ubuffer, offset, size, stride, max, depth)
 *    	Where:
 *	gc ............	is the pointer on the stack
 *	fd ............	is the file descriptor
 *	ubuffer .......	is the address of the user buffer.
 *	offset ........	is the offset in the file to begin reading
 *	size ..........	is the size of the transfer.
 *	stride ........	is the distance, in size units, to space the async reads.
 *	max ...........	is the max size of the file to be read.
 *	depth .........	is the number of async operations to perform.
 *
 * 3. Call end_async(gc) when finished.
 *	Where:
 *	gc ............ is the pointer on the stack.
 *
 * To use model #2:
 * 1. Call async_init(&pointer_on_stack,fd,direct_flag);
 *	The fd is the file descriptor for the async operations.
 *	The direct_flag sets VX_DIRECT 
 * 2. Call async_read(gc, fd, &ubuffer, offset, size, stride, max, depth)
 *    	Where:
 *	gc ............	is the pointer on the stack
 *	fd ............	is the file descriptor
 *	ubuffer .......	is the address of a pointer that will be filled in 
 *                      by the async library.
 *	offset ........	is the offset in the file to begin reading
 *	size ..........	is the size of the transfer.
 *	stride ........	is the distance, in size units, to space the async reads.
 *	max ...........	is the max size of the file to be read.
 *	depth .........	is the number of async operations to perform.
 *
 * 3. Call async_release(gc) when finished with the data that was returned.
 *    This allows the async library to reuse the memory that was filled in
 *    and returned to the user.
 *
 * 4. Call end_async(gc) when finished.
 *	Where:
 *	gc ............ is the pointer on the stack.
 *
 * To use model #1: (WRITES)
 * 1. Call async_init(&pointer_on_stack,fd,direct_flag);
 *	The fd is the file descriptor for the async operations.
 *
 * 2. Call async_write(gc, fd, ubuffer, size, offset, depth)
 *    	Where:
 *	gc ............	is the pointer on the stack
 *	fd ............	is the file descriptor
 *	ubuffer .......	is the address of the user buffer.
 *	size ..........	is the size of the transfer.
 *	offset ........	is the offset in the file to begin reading
 *	depth .........	is the number of async operations to perform.
 *
 * 4. Call end_async(gc) when finished.
 *	Where:
 *	gc ............ is the pointer on the stack.
 *
 * Notes:
 *	The intended use is to replace calls to read() with calls to
 *	async_read() and allow the user to make suggestions on 
 *	what kind of async read-ahead would be nice to have.
 *	The first transfer requested is guarenteed to be complete
 *	before returning to the caller. The async operations will
 *	be started and will also be guarenteed to have completed
 *	if the next call specifies its first request to be one
 *	that was previously performed with an async operation.
 *	
 *	The async_read_no_copy() function allows the async operations
 *	to return the data to the user and not have to perform 
 *	a bcopy of the data back into the user specified buffer 
 *	location. This model is faster but assumes that the user
 *	application has been modified to work with this model.
 *
 * 	The async_write() is intended to enhance the performance of 
 *	initial writes to a file. This is the slowest case in the write
 *	path as it must perform meta-data allocations and wait.
 */

#include <sys/types.h>
#include <aio.h>

#if defined(_LARGEFILE64_SOURCE) && !defined(__LP64__)
#	define aio_error	aio_error64
#	define aio_return	aio_return64
#	define aio_read 	aio_read64
#	define aio_cancel	aio_cancel64
#	define aio_write	aio_write64
#endif

#if defined(solaris) || defined(linux) || defined(SCO_Unixware_gcc) || defined(__NetBSD__)
#else
#include <sys/timers.h>
#endif
#include <sys/errno.h>
#include <unistd.h>
#ifndef bsd4_4
#include <malloc.h>
#endif
#ifdef VXFS
#include <sys/fs/vx_ioctl.h>
#endif

#if defined(OSFV5) || defined(linux)
#include <string.h>
#endif

#if defined(linux)
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#if (defined(solaris) && defined(__LP64__)) || defined(__s390x__) || defined(__FreeBSD__) || defined(__NetBSD__)
/* If we are building for 64-bit Solaris, all functions that return pointers
 * must be declared before they are used; otherwise the compiler will assume
 * that they return ints and the top 32 bits of the pointer will be lost,
 * causing segmentation faults.  The following includes take care of this.
 * It should be safe to add these for all other OSs too, but we're only
 * doing it for Solaris now in case another OS turns out to be a special case.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h> /* For the BSD string functions */
#endif

static void mbcopy(const char *source, char *dest, size_t len);


#if !defined(solaris) && !defined(off64_t) && !defined(_OFF64_T) && !defined(__off64_t_defined) && !defined(SCO_Unixware_gcc)
#	if defined(bsd4_4)
typedef off_t off64_t;
#	else
typedef long long off64_t;
#	endif
#endif
#if defined(OSFV5)
#include <string.h>
#endif


extern long long page_size;
extern int one;
/*
 * Internal cache entrys. Each entry on the global
 * cache, pointed to by async_init(gc) will be of
 * this structure type.
 */
static const char version[] = "Libasync Version $Revision: 3.52 $";
struct cache_ent {
#if defined(_LARGEFILE64_SOURCE) && defined(__CrayX1__)
	aiocb64_t myaiocb;		/* For use in large file mode */
#elif defined(_LARGEFILE64_SOURCE) && !defined(__LP64__)
	struct aiocb64 myaiocb;		/* For use in large file mode */
#else
	struct aiocb myaiocb;
#endif 
	long long fd;				/* File descriptor */
	long long size;				/* Size of the transfer */
	struct cache_ent *forward;		/* link to next element on cache list */
	struct cache_ent *back;			/* link to previous element on the cache list */
	long long direct;			/* flag to indicate if the buffer should be */
						/* de-allocated by library */
	char *real_address;			/* Real address to free */
	
	volatile void *oldbuf;			/* Used for firewall to prevent in flight */
						/* accidents */
	int oldfd;				/* Used for firewall to prevent in flight */
						/* accidents */
	size_t oldsize;				/* Used for firewall to prevent in flight */
						/* accidents */
	off64_t  offset;				
};

/*
 * Head of the cache list
 */
struct cache {
	struct cache_ent *head;		/* Head of cache list */
	struct cache_ent *tail;		/* tail of cache list */
	struct cache_ent *inuse_head;	/* head of in-use list */
	long long count;		/* How many elements on the cache list */
	struct cache_ent *w_head;		/* Head of cache list */
	struct cache_ent *w_tail;		/* tail of cache list */
	long long w_count;		/* How many elements on the write list */
	};

long long max_depth;
extern int errno;
static struct cache_ent *alloc_cache();
static struct cache_ent *incache();

#ifdef HAVE_ANSIC_C
void async_init(struct cache **,int, int);
int async_suspend(struct cache_ent *);
void end_async(struct cache *);
void takeoff_cache(struct cache *, struct cache_ent *);
void del_cache(struct cache *);
void putoninuse(struct cache *,struct cache_ent *);
void takeoffinuse(struct cache *);
struct cache_ent * allocate_write_buffer( struct cache *, long long , long long ,long long, long long, long long, long long, char *, char *);
void async_put_on_write_queue(struct cache *, struct cache_ent *);
void async_write_finish(struct cache *);
void async_wait_for_write(struct cache *);
int async_read(struct cache *, long long , char *, off64_t, long long, long long, off64_t, long long);
struct cache_ent * alloc_cache(struct cache *gc,long long fd,off64_t offset,long long size,long long op);
struct cache_ent * incache(struct cache *, long long, off64_t, long long);
int async_read_no_copy(struct cache *, long long, char **, off64_t, long long, long long, off64_t, long long);
void async_release(struct cache *gc);
size_t async_write(struct cache *,long long, char *, long long, off64_t, long long);
size_t async_write_no_copy(struct cache *gc,long long fd,char *buffer,long long size,long long offset,long long depth,char *free_addr);
#else
void async_init();
void end_async();
int async_suspend();
int async_read();
void async_release();
struct cache_ent *allocate_write_buffer();
size_t async_write();
void async_wait_for_write();
void async_put_on_write_queue();
void async_write_finish();
struct cache_ent * alloc_cache();
#endif

/* On Solaris _LP64 will be defined by <sys/types.h> if we're compiling
 * as a 64-bit binary.  Make sure that __LP64__ gets defined in this case,
 * too -- it should be defined on the compiler command line, but let's
 * not rely on this.
 */
#if defined(_LP64)
#if !defined(__LP64__)
#define __LP64__
#endif
#endif


/***********************************************/
/* Initialization routine to setup the library */
/***********************************************/
#ifdef HAVE_ANSIC_C
void async_init(struct cache **gc,int fd,int flag)
#else
void
async_init(gc,fd,flag)
struct cache **gc;
int fd;
int flag;
#endif
{
#ifdef VXFS
	if(flag)
		ioctl(fd,VX_SETCACHE,VX_DIRECT);
#endif
	if(*gc)
	{
		printf("Warning calling async_init two times ?\n");
		return;
	}
	*gc=(struct cache *)malloc((size_t)sizeof(struct cache));
	if(*gc == 0)
	{
		printf("Malloc failed\n");
		exit(174);
	}
	bzero(*gc,sizeof(struct cache));
#if defined(__AIX__) || defined(SCO_Unixware_gcc)
	max_depth=500;
#else
	max_depth=sysconf(_SC_AIO_MAX);
	if(max_depth < 0)
		max_depth = 500;
#endif
}

/***********************************************/
/* Tear down routine to shutdown the library   */
/***********************************************/
#ifdef HAVE_ANSIC_C
void end_async(struct cache *mygc)
#else
void
end_async(mygc)
struct cache *mygc;
#endif
{
	del_cache(mygc);
	if(mygc && (mygc->w_head !=NULL))
	   async_write_finish(mygc);

	if(mygc != NULL)
	   free((void *)mygc);
	mygc = NULL;
}

/***********************************************/
/* Wait for a request to finish                */
/***********************************************/
#ifdef HAVE_ANSIC_C
int
async_suspend(struct cache_ent *ce)
#else
int
async_suspend(ce)
struct cache_ent *ce;
#endif
{
#ifdef _LARGEFILE64_SOURCE 
#ifdef __LP64__
	const struct aiocb * const cblist[1] = {&ce->myaiocb};
#else
	const struct aiocb64 * const cblist[1] = {&ce->myaiocb};
#endif
#else
	const struct aiocb * const cblist[1] = {&ce->myaiocb};
#endif

#ifdef _LARGEFILE64_SOURCE 
#ifdef __LP64__
	return aio_suspend(cblist, 1, NULL);
#else
	return aio_suspend64(cblist, 1, NULL);
#endif
#else
	return aio_suspend(cblist, 1, NULL);
#endif
}

/*************************************************************************
 * This routine is a generic async reader assist funtion. It takes
 * the same calling parameters as read() but also extends the
 * interface to include:
 * stride ..... For the async reads, what is the distance, in size units, 
 * 		to space the reads. Note: Stride of 0 indicates that
 *		you do not want any read-ahead.
 * max    ..... What is the maximum file offset for this operation.
 * depth  ..... How much read-ahead do you want.
 * 
 * The calls to this will guarentee to complete the read() operation
 * before returning to the caller. The completion may occur in two
 * ways. First the operation may be completed by calling aio_read()
 * and then waiting for it to complete. Second  the operation may be 
 * completed by copying the data from a cache of previously completed 
 * async operations. 
 * In the event the read to be satisfied is not in the cache then a 
 * series of async operations will be scheduled and then the first 
 * async read will be completed. In the event that the read() can be 
 * satisfied from the cache then the data is copied back to the 
 * user buffer and a series of async reads will be initiated.  If a 
 * read is issued and the cache contains data and the read can not 
 * be satisfied from the cache, then the cache is discarded, and 
 * a new cache is constructed.
 * Note: All operations are aio_read(). The series will be issued
 * as asyncs in the order requested. After all are in flight
 * then the code will wait for the manditory first read.
 *************************************************************************/

#ifdef HAVE_ANSIC_C
int async_read(struct cache *gc, long long fd, char *ubuffer, off64_t offset, 
	long long size, long long stride, off64_t max, long long depth)
#else
int 
async_read(gc, fd, ubuffer, offset, size, stride, max, depth)
struct cache *gc;
long long fd;
char *ubuffer;
off64_t offset;
long long size;
long long stride;
off64_t max;
long long depth;
#endif
{
	off64_t a_offset,r_offset;
	long long a_size;
	struct cache_ent *ce,*first_ce=0;
	long long i;
	ssize_t retval=0;
	ssize_t ret;
	long long start = 0;
	long long del_read=0;

	a_offset=offset;
	a_size = size;
	/*
	 * Check to see if it can be completed from the cache
	 */
	if((ce=(struct cache_ent *)incache(gc,fd,offset,size)))
	{
		while((ret=aio_error(&ce->myaiocb))== EINPROGRESS)
		{
			async_suspend(ce);
		}
		if(ret)
		{
			printf("aio_error 1: ret %zd %d\n",ret,errno);
		}
		retval=aio_return(&ce->myaiocb);
		if(retval > 0)
		{
			mbcopy((char *)ce->myaiocb.aio_buf,(char *)ubuffer,(size_t)retval);
		}
#if defined(DEBUG)
		if(retval < ce->myaiocb.aio_nbytes)
		{
			printf("aio_return error1: ret %zd %d\n",retval,errno);
			printf("aio_return error1: fd %d offset %lld buffer %p size %zd Opcode %d\n",
				ce->myaiocb.aio_fildes,
				(long long)ce->myaiocb.aio_offset,
				ce->myaiocb.aio_buf,
				ce->myaiocb.aio_nbytes,
				ce->myaiocb.aio_lio_opcode
				);
		}
#endif
		ce->direct=0;
		takeoff_cache(gc,ce);
	}else
	{
		/*
		 * Clear the cache and issue the first request async()
		 */
		del_cache(gc);
		del_read++;
		first_ce=alloc_cache(gc,fd,offset,size,(long long)LIO_READ);
again:
		ret=aio_read(&first_ce->myaiocb);
		if(ret!=0)
		{
			if(errno==EAGAIN)
				goto again;
#if defined(DEBUG)
			else
				printf("error returned from aio_read(). Ret %zd errno %d\n",ret,errno);
#endif
		}
	}
	if(stride==0)	 /* User does not want read-ahead */
		goto out;
	if(a_offset<0)	/* Before beginning of file */
		goto out;
	if(a_offset+size>max)	/* After end of file */
		goto out;
	if(depth >=(max_depth-1))
		depth=max_depth-1;
	if(depth==0)
		goto out;
	if(gc->count > 1)
		start=depth-1;
	for(i=start;i<depth;i++)	/* Issue read-aheads for the depth specified */
	{
		r_offset=a_offset+((i+1)*(stride*a_size));
		if(r_offset<0)
			continue;
		if(r_offset+size > max)
			continue;
		if((ce=incache(gc,fd,r_offset,a_size)))
			continue;
		ce=alloc_cache(gc,fd,r_offset,a_size,(long long)LIO_READ);
		ret=aio_read(&ce->myaiocb);
		if(ret!=0)
		{
			takeoff_cache(gc,ce);
			break;
		}
	}			
out:
	if(del_read)	/* Wait for the first read to complete */
	{
		while((ret=aio_error(&first_ce->myaiocb))== EINPROGRESS)
		{
			async_suspend(first_ce);
		}
		if(ret)
			printf("aio_error 2: ret %zd %d\n",ret,errno);
		retval=aio_return(&first_ce->myaiocb);
#if defined(DEBUG)
		if(retval < first_ce->myaiocb.aio_nbytes)
		{
			printf("aio_return error2: ret %zd %d\n",retval,errno);
			printf("aio_return error2: fd %d offset %lld buffer %p size %zd Opcode %d\n",
				first_ce->myaiocb.aio_fildes,
				(long long)first_ce->myaiocb.aio_offset,
				first_ce->myaiocb.aio_buf,
				first_ce->myaiocb.aio_nbytes,
				first_ce->myaiocb.aio_lio_opcode
				);
		}
#endif
		if(retval > 0)
		{
			mbcopy((char *)first_ce->myaiocb.aio_buf,(char *)ubuffer,(size_t)retval);
		}
		first_ce->direct=0;
		takeoff_cache(gc,first_ce);
	}
	return((int)retval);	
}

/************************************************************************
 * This routine allocates a cache_entry. It contains the 
 * aiocb block as well as linkage for use in the cache mechanism.
 * The space allocated here will be released after the cache entry
 * has been consumed. The routine takeoff_cache() will be called
 * after the data has been copied to user buffer or when the
 * cache is purged. The routine takeoff_cache() will also release
 * all memory associated with this cache entry.
 ************************************************************************/

#ifdef HAVE_ANSIC_C
struct cache_ent * alloc_cache(struct cache *gc,long long fd,off64_t offset,long long size,long long op)
#else
struct cache_ent *
alloc_cache(gc,fd,offset,size,op)
struct cache *gc;
long long fd,size,op;
off64_t offset;
#endif
{
	struct cache_ent *ce;
	intptr_t temp;
	ce=(struct cache_ent *)malloc((size_t)sizeof(struct cache_ent));
	if(ce == (struct cache_ent *)0)
	{
		printf("Malloc failed\n");
		exit(175);
	}
	bzero(ce,sizeof(struct cache_ent));
	ce->myaiocb.aio_fildes=(int)fd;
	ce->myaiocb.aio_offset=(off64_t)offset;
	ce->real_address = malloc((size_t)(size+page_size));
	if(ce->real_address == NULL)
	{
		printf("Malloc failure in alloc_cache\n");
		exit(1);
	}
/*printf("\nAllocate buffer2 %p Size %lld \n",ce->real_address,size+page_size);*/
	temp = (intptr_t)ce->real_address;
	temp = (temp+(page_size-1)) & ~(page_size-1);
	ce->myaiocb.aio_buf=(volatile void *)temp;
	if(ce->myaiocb.aio_buf == NULL)
	{
		printf("Malloc failed\n");
		exit(176);
	}
	/*bzero(ce->myaiocb.aio_buf,(size_t)size);*/
	ce->myaiocb.aio_reqprio=0;
	ce->myaiocb.aio_nbytes=(size_t)size;
	ce->myaiocb.aio_sigevent.sigev_notify=SIGEV_NONE;
	ce->myaiocb.aio_lio_opcode=(int)op;
	ce->fd=(int)fd;
	ce->forward=0;
	ce->back=gc->tail;
	if(gc->tail)
		gc->tail->forward = ce;
	gc->tail= ce;
	if(!gc->head)
		gc->head=ce;
	gc->count++;
/*	printf("Added to global cache\n");*/
	return(ce);
}

/************************************************************************
 * This routine checks to see if the requested data is in the
 * cache. 
*************************************************************************/
#ifdef HAVE_ANSIC_C
struct cache_ent *
incache(struct cache *gc, long long fd, off64_t offset, long long size)
#else
struct cache_ent *
incache(gc,fd,offset,size)
struct cache *gc;
long long fd,size;
off64_t offset;
#endif
{
	struct cache_ent *move;
	if(gc->head==0)
	{
		return(0);
	}
	move=gc->head;
	while(move)
	{
		if((move->fd == fd) && (move->myaiocb.aio_offset==(off64_t)offset) &&
			((size_t)size==move->myaiocb.aio_nbytes))
			{
				return(move);
			}
		move=move->forward;
	}
	return(0);
}

/************************************************************************
 * This routine removes a specific cache entry from the cache, and
 * releases all memory associated witht the cache entry (if not direct).
*************************************************************************/

void
takeoff_cache(struct cache *gc, struct cache_ent *ce)
{
	struct cache_ent *move;
	long long found;
	move=gc->head;
	if(move==ce) /* Head of list */
	{

		gc->head=ce->forward;
		if(gc->head)
			gc->head->back=0;
		else
			gc->tail = 0;
		if(!ce->direct)
		{
			if(ce->real_address != NULL)
			   free((void *)(ce->real_address));
			ce->real_address = NULL;
			if(ce != NULL)
			   free((void *)ce);
			ce = NULL;
		}
		gc->count--;
		return;
	}
	found=0;
	while(move)
	{
		if(move==ce)
		{
			if(move->forward)
			{
				move->forward->back=move->back;
			}
			if(move->back)
			{
				move->back->forward=move->forward;
			}
			found=1;
			break;
		}
		else
		{
			move=move->forward;
		}
	}
	if(gc->head == ce)
		gc->tail = ce;
	if(!found)
		printf("Internal Error in takeoff cache\n");
	move=gc->head;
	if(!ce->direct)
	{
		if(ce->real_address != NULL)
		   free((void *)(ce->real_address));
		ce->real_address = NULL;
		if(ce != NULL)
		   free((void *)ce);
		ce = NULL;
	}
	gc->count--;
}

/************************************************************************
 * This routine is used to purge the entire cache. This is called when
 * the cache contains data but the incomming read was not able to 
 * be satisfied from the cache. This indicates that the previous
 * async read-ahead was not correct and a new pattern is emerging. 
 ************************************************************************/
#ifdef HAVE_ANSIC_C
void
del_cache(struct cache *gc)
#else
void
del_cache(gc)
struct cache *gc;
#endif
{
	struct cache_ent *ce;
	ssize_t ret;
	ce=gc->head;
	while(1)
	{
		ce=gc->head;
		if(ce==0)
			return;
/*printf("Cancel I/O Offset: %lld\n",ce->myaiocb.aio_offset);*/
		while((ret = aio_cancel(0,&ce->myaiocb))==AIO_NOTCANCELED)
			; 
		while((ret=aio_error(&ce->myaiocb))== EINPROGRESS)
		{
			async_suspend(ce);
		}

		ret = aio_return(&ce->myaiocb);
		if(ret < 0)
			printf("Aio return in del_cache failed. %ld %d\n",ret,errno);
/*printf("Cancel: %ld\n",ret);*/
		ce->direct=0;
		takeoff_cache(gc,ce);	  /* remove from cache */
	}
}

/************************************************************************
 * Like its sister async_read() this function performs async I/O for 
 * all buffers but it differs in that it expects the caller to 
 * request a pointer to the data to be returned instead of handing
 * the function a location to put the data. This will allow the
 * async I/O to be performed and does not require any bcopy to be
 * done to put the data back into the location specified by the caller.
 ************************************************************************/
#ifdef HAVE_ANSIC_C
int
async_read_no_copy(struct cache *gc, long long fd, char **ubuffer, off64_t offset, long long size, long long stride, off64_t max, long long depth)
#else
int
async_read_no_copy(gc, fd, ubuffer, offset, size, stride, max, depth)
struct cache *gc;
long long fd;
char **ubuffer;
off64_t offset;
long long size;
long long stride;
off64_t max;
long long depth;
#endif
{
	off64_t a_offset,r_offset;
	long long a_size;
	struct cache_ent *ce,*first_ce=0;
	long long i;
	ssize_t retval=0;
	ssize_t ret;
	long long del_read=0;
	long long start=0;

	a_offset=offset;
	a_size = size;
	if((a_offset + size) >= max)
	{
		stride = 0LL;
		offset = a_offset = r_offset = 0LL;
	}
	if(stride < 0)
		stride = 0;
	/*
	 * Check to see if it can be completed from the cache
	 */
	if((ce=(struct cache_ent *)incache(gc,fd,offset,size)))
	{
		while((ret=aio_error(&ce->myaiocb))== EINPROGRESS)
		{
			async_suspend(ce);
			ret = 0;
		}
		if(ret)
		{
			printf("aio_error 3: ret %zd %d\n",ret,errno);
			printf("It changed in flight\n");
		}	
#if defined(DEBUG)
		printf("aio_error debug: fd %d offset %lld buffer %p size %zd Opcode %d\n",
				ce->myaiocb.aio_fildes,
				(long long)ce->myaiocb.aio_offset,
				ce->myaiocb.aio_buf,
				ce->myaiocb.aio_nbytes,
				ce->myaiocb.aio_lio_opcode
				);
#endif
		retval=aio_return(&ce->myaiocb);
		if(retval >= 0)
		{
			*ubuffer= (char *)ce->myaiocb.aio_buf;
		}
		if(ce->myaiocb.aio_buf == NULL)
		{
			printf("Stuffing a null in users buffer.\n");
			exit(1);
		}
		else
			*ubuffer= (char *)ce->myaiocb.aio_buf;
#if defined(DEBUG)
/* Sometimes this returns a zero, on a perfectly good offset and size !!! Needs work !!!*/
		if(retval < ce->myaiocb.aio_nbytes)
		{
			printf("aio_return error4: ret %zd %d\n",retval,errno);
			printf("aio_return error4: fd %d offset %lld buffer %p Nbytes %zd Opcode %d\n",
				ce->myaiocb.aio_fildes,
				(long long)ce->myaiocb.aio_offset,
				ce->myaiocb.aio_buf,
				ce->myaiocb.aio_nbytes,
				ce->myaiocb.aio_lio_opcode
				);
		}
#endif
		ce->direct=1;
		takeoff_cache(gc,ce); /* do not delete buffer*/
		putoninuse(gc,ce);
	}else
	{
		/*
		 * Clear the cache and issue the first request async()
		 */
		del_cache(gc);
		del_read++;
/*	printf("\nIssue first read Offset %lld\n",offset);*/
		first_ce=alloc_cache(gc,fd,offset,size,(long long)LIO_READ); /* allocate buffer */
again2:
		first_ce->oldbuf=first_ce->myaiocb.aio_buf;
		first_ce->oldfd=first_ce->myaiocb.aio_fildes;
		first_ce->oldsize=first_ce->myaiocb.aio_nbytes;
		first_ce->offset =first_ce->myaiocb.aio_offset;
		ret=aio_read(&first_ce->myaiocb);
		if(ret!=0)
		{
			if(errno==EAGAIN)
				goto again2;
#if defined(DEBUG)
			else
				printf("error returned from aio_read(). Ret %zd errno %d\n",ret,errno);
#endif
		}
	}
	if(stride==0)	 /* User does not want read-ahead */
		goto out;
	if(a_offset<0)	/* Before beginning of file */
	{
		a_offset = r_offset = 0LL;
		goto out;
	}
	if(a_offset+size >= max)	/* After end of file */
	{
		offset = a_offset = r_offset = 0LL;
		stride = 0;
		first_ce->myaiocb.aio_offset=(off64_t)offset;
		goto out;
		/*goto again2;*/
	}
	if(depth >=(max_depth-1))
		depth=max_depth-1;
	if(depth==0)
	{
		offset = a_offset = r_offset = 0LL;
		goto out;
	}
/*printf("\n Depth %lld ",depth);*/
	if(gc->count > 1)
		start=depth-1;
here:
	for(i=start;i<depth;i++)	/* Issue read-aheads for the depth specified */
	{
		/* stride = 0;	 Hack to make stride read work */
		r_offset = a_offset+((i+1)*(stride*a_size));
		if(r_offset < 0)
		{
			a_offset = r_offset = 0LL;
			continue;
		}
		if((r_offset+a_size) >= max)
		{
			offset = a_offset = r_offset = 0LL;
			depth--;
/*	printf("Reduce depth %lld\n",depth);*/
			if(depth < 1)	/* reduce depth first to see if it can stay below eof */
				depth = 1;
			if((depth == 1) && (stride > 0))
				stride--;  /* Reduce stride, as depth has been depleted */
			if(stride < 0)
				stride = 0;
			goto here;
		}
		if((ce=incache(gc,fd,r_offset,a_size)))
		{
			continue;
		}
/*	printf("\nIra offset %lld CD %lld",r_offset,i);*/
		ce=alloc_cache(gc,fd,r_offset,a_size,(long long)LIO_READ);
		ce->oldbuf=ce->myaiocb.aio_buf;
		ce->oldfd=ce->myaiocb.aio_fildes;
		ce->offset=ce->myaiocb.aio_offset;
		ce->oldsize=ce->myaiocb.aio_nbytes;

		ret=aio_read(&ce->myaiocb);
		if(ret!=0)
		{
			takeoff_cache(gc,ce);
			break;
		}
	}			
out:
	if(del_read)	/* Wait for the first read to complete */
	{
		while((ret=aio_error(&first_ce->myaiocb))== EINPROGRESS)
		{
			async_suspend(first_ce);
			ret = 0;
		}
/*printf("Wait for first read to complete\n");*/
#if defined(DEBUG)
		if(ret)
			printf("aio_error 4: ret %zd %d\n",ret,errno);
		if(first_ce->oldbuf != first_ce->myaiocb.aio_buf ||
			first_ce->oldfd != first_ce->myaiocb.aio_fildes ||
			first_ce->oldsize != first_ce->myaiocb.aio_nbytes) 
			   printf("It changed in flight2\n");
#endif
		retval=aio_return(&first_ce->myaiocb);
/*
		if(retval > 0)
			printf("First read complete\n");
*/
#if defined(DEBUG)
		if(retval == 0)
		{
			printf("Aio_return came back with a zero \n");
		}
#endif

#if defined(DEBUG)
		if(retval < first_ce->myaiocb.aio_nbytes)
		{
			printf("aio_return error5: ret %zd %d\n",retval,errno);
			printf("aio_return error5: fd %d offset %lld buffer %p size %zd Opcode %d\n",
				first_ce->myaiocb.aio_fildes,
				(long long)first_ce->myaiocb.aio_offset,
				first_ce->myaiocb.aio_buf,
				first_ce->myaiocb.aio_nbytes,
				first_ce->myaiocb.aio_lio_opcode
				);
		}
#endif
		if(retval >= 0)
		{
			*ubuffer= (char *)first_ce->myaiocb.aio_buf;
		}
		if(first_ce->myaiocb.aio_buf == NULL)
		{
			printf("Stuffing2 error\n");
			exit(1);
		}
		else
			*ubuffer= (char *)first_ce->myaiocb.aio_buf;

		first_ce->direct=1;	 /* do not delete the buffer */
		takeoff_cache(gc,first_ce);
		putoninuse(gc,first_ce);
	}
/*printf(" RT ");*/
	return((int)retval);	
}

/************************************************************************
 * The caller is now finished with the data that was provided so
 * the library is now free to return the memory to the pool for later
 * reuse.
 ************************************************************************/
#ifdef HAVE_ANSIC_C
void async_release(struct cache *gc)
#else
void
async_release(gc)
struct cache *gc;
#endif
{
	takeoffinuse(gc);
}


/************************************************************************
 * Put the buffer on the inuse list. When the user is finished with 
 * the buffer it will call back into async_release and the items on the 
 * inuse list will be deallocated.
 ************************************************************************/
#ifdef HAVE_ANSIC_C
void
putoninuse(struct cache *gc,struct cache_ent *entry)
#else
void
putoninuse(gc,entry)
struct cache *gc;
struct cache_ent *entry;
#endif
{
	if(gc->inuse_head)
		entry->forward=gc->inuse_head;
	else
		entry->forward=0;
	gc->inuse_head=entry;
}

/************************************************************************
 * This is called when the application is finished with the data that
 * was provided. The memory may now be returned to the pool.
 ************************************************************************/
#ifdef HAVE_ANSIC_C
void
takeoffinuse(struct cache *gc)
#else
void
takeoffinuse(gc)
struct cache *gc;
#endif
{
	struct cache_ent *ce;
	if(gc->inuse_head==0)
		printf("Takeoffinuse error\n");
	ce=gc->inuse_head;
	gc->inuse_head=gc->inuse_head->forward;
	
	if(gc->inuse_head !=0)
		printf("Error in take off inuse\n");
	if(ce->real_address != NULL)
	   free((void*)(ce->real_address));
	ce->real_address = NULL;
	if(ce != NULL)
	   free(ce);
	ce = NULL;
}

/*************************************************************************
 * This routine is a generic async writer assist funtion. It takes
 * the same calling parameters as write() but also extends the
 * interface to include:
 * 
 * offset ..... offset in the file.
 * depth  ..... How much read-ahead do you want.
 * 
 *************************************************************************/
#ifdef HAVE_ANSIC_C
size_t
async_write(struct cache *gc,long long fd,char *buffer,long long size,off64_t offset,long long depth)
#else
size_t
async_write(gc,fd,buffer,size,offset,depth)
struct cache *gc;
long long fd,size;
char *buffer;
off64_t offset;
long long depth;
#endif
{
	struct cache_ent *ce;
	size_t ret;
	ce=allocate_write_buffer(gc,fd,offset,size,(long long)LIO_WRITE,depth,0LL,(char *)0,(char *)0);
	ce->direct=0;	 /* not direct. Lib supplies buffer and must free it */
	mbcopy(buffer,(char *)(ce->myaiocb.aio_buf),(size_t)size);
	async_put_on_write_queue(gc,ce);
	/*
	printf("asw: fd %d offset %lld, size %zd\n",ce->myaiocb.aio_fildes,
		ce->myaiocb.aio_offset,
		ce->myaiocb.aio_nbytes);
	*/	

again:
	ret=aio_write(&ce->myaiocb);
	if(ret==-1)
	{
		if(errno==EAGAIN)
		{
			async_wait_for_write(gc);
printf("Again 1\n");
			goto again;
		}
		if(errno==0)
		{
			/* Compensate for bug in async library */
			async_wait_for_write(gc);
printf("Again 2\n");
			goto again;
		}
		else
		{
			printf("Error in aio_write: ret %zd errno %d count %lld\n",ret,errno,gc->w_count);
			/*
			printf("aio_write_no_copy: fd %d buffer %x offset %lld size %zd\n",
				ce->myaiocb.aio_fildes,
				ce->myaiocb.aio_buf,
				ce->myaiocb.aio_offset,
				ce->myaiocb.aio_nbytes);
			*/
			exit(177);
		}
	} 
	return((ssize_t)size);
}

/*************************************************************************
 * Allocate a write aiocb and write buffer of the size specified. Also 
 * put some extra buffer padding so that VX_DIRECT can do its job when
 * needed.
 *************************************************************************/

#ifdef HAVE_ANSIC_C
struct cache_ent *
allocate_write_buffer( struct cache *gc, long long fd, long long offset, long long size,long long op, 
	long long w_depth, long long direct, char *buffer, char *free_addr)
#else
struct cache_ent *
allocate_write_buffer(gc,fd,offset,size,op,w_depth,direct,buffer,free_addr)
struct cache *gc;
long long fd,size,op;
off64_t offset;
long long w_depth;
long long direct;
char *buffer,*free_addr;
#endif
{
	struct cache_ent *ce;
	intptr_t temp;
	if(fd==0LL)
	{
		printf("Setting up write buffer insane\n");
		exit(178);
	}
	if(gc->w_count > w_depth)
		async_wait_for_write(gc);
	ce=(struct cache_ent *)malloc((size_t)sizeof(struct cache_ent));
	if(ce == (struct cache_ent *)0)
	{
		printf("Malloc failed 1\n");
		exit(179);
	}
	bzero(ce,sizeof(struct cache_ent));
	ce->myaiocb.aio_fildes=(int)fd;
	ce->myaiocb.aio_offset=(off_t)offset;
	if(!direct)
	{
		ce->real_address = malloc((size_t)(size+page_size));
		if(ce->real_address == (void *)NULL)
		{
			printf("Malloc failure in alloc_write_buffer\n");
			exit(1);
		}
		temp = (intptr_t)ce->real_address;
		temp = (temp+(page_size-1)) & ~(page_size-1);
		ce->myaiocb.aio_buf=(volatile void *)temp;
	}
	else
	{
		ce->myaiocb.aio_buf=(volatile void *)buffer;
		ce->real_address=(char *)free_addr;
	}
	if(ce->myaiocb.aio_buf == 0)
	{
		printf("Malloc failed 2\n");
		exit(180);
	}
	ce->myaiocb.aio_reqprio=0;
	ce->myaiocb.aio_nbytes=(size_t)size;
	ce->myaiocb.aio_sigevent.sigev_notify=SIGEV_NONE;
	ce->myaiocb.aio_lio_opcode=(int)op;
	ce->fd=(int)fd;
	return(ce);
}

/*************************************************************************
 * Put it on the outbound queue.
 *************************************************************************/

#ifdef HAVE_ANSIC_C
void
async_put_on_write_queue(struct cache *gc,struct cache_ent *ce)
#else
void
async_put_on_write_queue(gc,ce)
struct cache *gc;
struct cache_ent *ce;
#endif
{
	ce->forward=0;
	ce->back=gc->w_tail;
	if(gc->w_tail)
		gc->w_tail->forward = ce;
	gc->w_tail= ce;
	if(!gc->w_head)
		gc->w_head=ce;
	gc->w_count++;
	return;
}

/*************************************************************************
 * Cleanup all outstanding writes
 *************************************************************************/
#ifdef HAVE_AHSIC_C
void
async_write_finish(struct cache *gc)
#else
void
async_write_finish(gc)
struct cache *gc;
#endif
{
	while(gc->w_head)
	{
		async_wait_for_write(gc);
	}
}

/*************************************************************************
 * Wait for an I/O to finish
 *************************************************************************/

#ifdef HAVE_ANSIC_C
void
async_wait_for_write(struct cache *gc)
#else
void
async_wait_for_write(gc)
struct cache *gc;
#endif
{
	struct cache_ent *ce;
	size_t ret;
	int retval;
	if(gc->w_head==0)
		return;
	ce=gc->w_head;
        if(ce == NULL)
		return;
	gc->w_head=ce->forward;
	gc->w_count--;
	ce->forward=NULL;
	if(ce==gc->w_tail)
		gc->w_tail=0;
	/*printf("Wait for buffer %x  offset %lld  size %zd to finish\n",
		ce->myaiocb.aio_buf,
		ce->myaiocb.aio_offset,
		ce->myaiocb.aio_nbytes);
	printf("write count %lld \n",gc->w_count);
	*/
	while((ret=aio_error(&ce->myaiocb))== EINPROGRESS)
	{
		async_suspend(ce);
	}

#if defined(DEBUG)
	if(ret)
	{
		printf("aio_error 5: ret %zd %d\n",ret,errno);
		printf("fd %d offset %lld size %zd\n",
			ce->myaiocb.aio_fildes,
			(long long)ce->myaiocb.aio_offset,
			ce->myaiocb.aio_nbytes);
		exit(181);
	}
#endif
	retval=aio_return(&ce->myaiocb);
#if defined(DEBUG)
	if(retval < 0)
	{
		printf("aio_return return error: %d\n",errno);
	}
#endif

	if(!ce->direct)
	{
		if(ce->real_address != NULL)
		   free((void *)(ce->real_address)); /* Causes crash. */
		ce->real_address=NULL;
		if(ce != NULL)
		   free((void *)ce);
		ce=NULL;
	}

}

/*************************************************************************
 * This routine is a generic async writer assist funtion. It takes
 * the same calling parameters as write() but also extends the
 * interface to include:
 * 
 * offset ..... offset in the file.
 * depth  ..... How much read-ahead do you want.
 * free_addr .. address of memory to free after write is completed.
 * 
 *************************************************************************/
#ifdef HAVE_ANSIC_C
size_t
async_write_no_copy(struct cache *gc,long long fd,char *buffer,long long size,long long offset,long long depth,char *free_addr)
#else
size_t
async_write_no_copy(gc,fd,buffer,size,offset,depth,free_addr)
struct cache *gc;
long long fd,size;
char *buffer;
long long offset;
long long depth;
char *free_addr;
#endif
{
	struct cache_ent *ce;
	size_t ret;
	long long direct = 1;
	ce=allocate_write_buffer(gc,fd,offset,size,(long long)LIO_WRITE,depth,direct,buffer,free_addr);
	ce->direct=0;	/* have library de-allocate the buffer */
	async_put_on_write_queue(gc,ce);
	/*
	printf("awnc: fd %d offset %lld, size %zd\n",ce->myaiocb.aio_fildes,
		ce->myaiocb.aio_offset,
		ce->myaiocb.aio_nbytes);
	*/

again2:
	ret=aio_write(&ce->myaiocb);
	if(ret==-1)
	{
		if(errno==EAGAIN)
		{
			async_wait_for_write(gc);
			printf("Again2 1\n");
			goto again2;
		}
		if(errno==0)
		{
			/* Compensate for bug in async library */
			async_wait_for_write(gc);
			printf("Again2 2\n");
			goto again2;
		}
		else
		{
#if defined(DEBUG)
			printf("Error in aio_write: ret %zd errno %d\n",ret,errno);
			printf("aio_write_no_copy: fd %d buffer %p offset %lld size %zd\n",
				ce->myaiocb.aio_fildes,
				ce->myaiocb.aio_buf,
				(long long)ce->myaiocb.aio_offset,
				ce->myaiocb.aio_nbytes);
			exit(182);
#endif
		}
	} 
	else	
	{
		return((ssize_t)size);
	}
	return((ssize_t)size);
}

void mbcopy(source, dest, len)
const char *source;
char *dest;
size_t len;
{
	int i;
	for(i=0;i<len;i++)
		*dest++=*source++;
}


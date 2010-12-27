/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef LINUX

#ifndef _lint

/** \file
   Operating System access implementation
*/

#include "dsl_cpe_control.h"
#include "dsl_cpe_os.h"

#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT

#ifndef PRJ_NAME_PREFIX
#define PRJ_NAME_PREFIX
#endif

#define SYS_NAME_PREFIX  PRJ_NAME_PREFIX"/tmp"
#define SYS_PIPE_PREFIX  SYS_NAME_PREFIX"/pipe"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_OS

#endif /* #ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

static struct termios stored_stdin_settings;
#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT
static struct termios stored_stdout_settings;
#endif /* INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT
/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_Open(const DSL_char_t *pName)
{
   return open(pName, O_RDWR, 0644);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_Close(const DSL_int_t fd)
{
   return close(fd);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_Write(const DSL_int_t fd, const DSL_void_t *pData, const DSL_uint32_t nSize)
{
   return write(fd, pData, nSize);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_Read(const DSL_int_t fd, DSL_void_t *pData, const DSL_uint32_t nSize)
{
   return read(fd, (char *)pData, nSize);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_Ioctl(const DSL_int_t fd, const DSL_uint32_t cmd, DSL_int_t param)
{
   return ioctl(fd, cmd, param);
}

/**
   Wait for a device wake up
*/
DSL_int_t DSL_CPE_Select(
   const DSL_int_t max_fd,
   const DSL_CPE_fd_set_t *read_fd_in,
   DSL_CPE_fd_set_t *read_fd_out,
   const DSL_uint32_t timeout_msec)
{
   DSL_CPE_fd_set_t tmp;
   struct timeval tv;

   tv.tv_sec = timeout_msec / 1000;
   tv.tv_usec = (timeout_msec % 1000) * 1000;

   if(read_fd_in)
   {
      if(read_fd_out)
      {
         if (timeout_msec == (DSL_uint32_t)-1)
         {
            if (read_fd_out != read_fd_in)
            {
               memcpy(read_fd_out, read_fd_in, sizeof(DSL_CPE_fd_set_t));
            }
            return select(max_fd, read_fd_out, NULL, NULL, NULL);
         }
         else
         {
            if (read_fd_out != read_fd_in)
            {
               memcpy(read_fd_out, read_fd_in, sizeof(DSL_CPE_fd_set_t));
            }
            return select(max_fd, read_fd_out, NULL, NULL, &tv);
         }
      }
      else
      {
         if (timeout_msec == (DSL_uint32_t)-1)
         {
            memcpy(&tmp, read_fd_in, sizeof(DSL_CPE_fd_set_t));
            return select(max_fd, &tmp, NULL, NULL, NULL);
         }
         else
         {
            memcpy(&tmp, read_fd_in, sizeof(DSL_CPE_fd_set_t));
            return select(max_fd, &tmp, NULL, NULL, &tv);
         }
      }
   }
   else
   {
      DSL_CPE_MSecSleep(timeout_msec);
   }

   return 0;
}

#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT
/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_GetChar(DSL_void_t)
{
   return getchar();
}
#endif /* #ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT
/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_PutChar(DSL_char_t c, DSL_CPE_File_t *stream)
{
   return putc(c, stream);
}
#endif /* #ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_void_t DSL_CPE_FD_SET(DSL_int_t fd, DSL_CPE_fd_set_t *set)
{
   FD_SET(fd, set);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_void_t DSL_CPE_FD_CLR(DSL_int_t fd, DSL_CPE_fd_set_t *set)
{
   FD_CLR(fd, set);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_void_t DSL_CPE_FD_ZERO(DSL_CPE_fd_set_t *set)
{
   FD_ZERO(set);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_FD_ISSET(DSL_int_t fd, const DSL_CPE_fd_set_t *set)
{
#ifdef DSL_CPE_SIMULATOR
   DSL_uint32_t i;

   for (i = 0; i < (__FD_SETSIZE / __NFDBITS); i++)
   {
      if (FD_ISSET(fd, set))
      {
         return DSL_SIM_FD_ISSET(fd);
      }
   }
   return 0;
#else
   return FD_ISSET(fd, set);
#endif
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_Error_t DSL_CPE_PipeCreate(DSL_char_t *pName)
{
   /* Used to generate the pipe path */
   DSL_char_t *pipepath = NULL;
   DSL_Error_t ret = DSL_SUCCESS;

   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "create pipe %s" DSL_CPE_CRLF, pName));

   pipepath = DSL_CPE_Malloc(strlen(SYS_PIPE_PREFIX"/") + strlen(pName) + 1);
   if ( !pipepath )
   {
      return DSL_ERR_MEMORY;
   }

   strcpy(pipepath, SYS_PIPE_PREFIX"/");
   /* try to create directory, ignore error designedly */
   mkdir(pipepath, S_IFDIR | 0777);
   strcat(pipepath, pName);

   /*create a named pipe and check errors*/
   if ((mkfifo(pipepath, 0777) == -1) && (errno != EEXIST))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "mkfifo %s failed (errno=%d)"
         DSL_CPE_CRLF, pipepath, errno));
      /* delete named pipe */
      /*unlink(pipepath);*/
      ret = DSL_ERROR;
   }

   DSL_CPE_Free (pipepath);
   return ret;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_CPE_File_t *DSL_CPE_PipeOpen(DSL_char_t *pName, DSL_boolean_t reading, DSL_boolean_t blocking)
{
   int fd;
   int flags;
   DSL_CPE_File_t * pipefile = DSL_NULL;
   DSL_char_t pipepath[256];

   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "open pipe %s" DSL_CPE_CRLF, pName));

   DSL_CPE_snprintf(pipepath, sizeof(pipepath), SYS_PIPE_PREFIX"/%s", pName);
   /* only open allows the flag "O_NONBLOCK",
      so first open a fd and change it to a DSL_CPE_File_t* with fdopen() */
   if (reading == DSL_TRUE)
   {
      flags = O_RDONLY;
   }
   else
   {
      flags = O_WRONLY;
   }

   if (!blocking)
   {
      flags |= O_NONBLOCK;
   }

   fd = open(pipepath, flags);
   if (fd <= 0)
   {
      /* Attention: The following printout is assigned to WARNING only because
         the evvent handling of standard pipe mechanism will always lead to
         this state if there is no instance that collects event data from an
         existing pipe. This is the normal behavior and should be NOT displayed
         as an error! */
      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, ("open %s failed (errno=%d)" DSL_CPE_CRLF,
         pipepath, errno));
   }
   if (fd > 0)
   {
      if (reading == DSL_TRUE)
      {
         pipefile = fdopen(fd, "r");
      }
      else
      {
         pipefile = fdopen(fd, "w");
      }
      if (pipefile == DSL_NULL)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, ("fdopen %s failed (errno=%d)" DSL_CPE_CRLF,
            pipepath, errno));
      }
   }

   return pipefile;
}


/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_Error_t DSL_CPE_PipeClose(DSL_CPE_File_t *pipefile)
{
   return (fclose(pipefile)==0) ? DSL_SUCCESS : DSL_ERROR;
}


/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int32_t DSL_CPE_LockCreate(
               DSL_CPE_Lock_t *lockId)
{
   if(lockId)
   {
      if (DSL_LOCK_INIT_VALID(lockId) == DSL_FALSE)
      {
         if(sem_init(&lockId->object, 0, 1) == 0)
         {
            lockId->bValid = DSL_TRUE;

            return DSL_SUCCESS;
         }
      }
   }
   
   return DSL_ERROR;
}


/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int32_t DSL_CPE_LockDelete(
               DSL_CPE_Lock_t *lockId)
{
   /* delete semaphore */
   if(lockId)
   {
      if (DSL_LOCK_INIT_VALID(lockId) == DSL_TRUE)
      {
         if (sem_destroy(&lockId->object) == 0)
         {
            lockId->bValid = DSL_FALSE;

            return DSL_SUCCESS;
         }
      }
   }
 
   return DSL_ERROR;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int32_t DSL_CPE_LockGet(
               DSL_CPE_Lock_t *lockId)
{
   if(lockId)
   {
      if (DSL_LOCK_INIT_VALID(lockId) == DSL_TRUE)
      {
         if (sem_wait(&lockId->object) == 0)
         {
            return DSL_SUCCESS;
         }
      }
   }

   return DSL_ERROR;
}

/**
   LINUX Application - Get the Lock with timeout.

\par Implementation

\param
   lockId   Provides the pointer to the Lock Object.

\param
   timeout_ms  Timeout value [ms]
               - 0: no wait
               - -1: wait forever
               - any other value: waiting for specified amount of milliseconds
\param
   pRetCode    Points to the return code variable. [O]
               - If the pointer is NULL the return code will be ignored, else
                 the corresponding return code will be set
               - For timeout the return code is set to 1.

\return
   DSL_SUCCESS on success.
   DSL_ERROR   on error or timeout.

\note
   To detect timeouts provide the return code varibale, in case of timeout
   the return code is set to 1.
*/
DSL_int32_t DSL_CPE_LockTimedGet(
               DSL_CPE_Lock_t *lockId,
               DSL_uint32_t timeout_ms,
               DSL_int32_t  *pRetCode)
{
   struct timespec t;
   int ret;

   if(lockId)
   {
      if (DSL_LOCK_INIT_VALID(lockId) == DSL_TRUE)
      {
         if(timeout_ms == 0xFFFFFFFF)
         {
            return DSL_CPE_LockGet(lockId);
         }

         if(timeout_ms == 0)
         {
               /* just try to get the semaphore without waiting, if not available return to calling thread */
            ret = sem_trywait(&lockId->object);
            if(ret != 0)
            {
               if (pRetCode) *pRetCode = 0;
               return DSL_ERROR;
            }
         }
         else
         {
            clock_gettime(CLOCK_REALTIME, &t);
      
            t.tv_sec +=  (timeout_ms / 1000);
            t.tv_nsec += (timeout_ms % 1000) * 1000 * 1000;

            ret = sem_timedwait(&lockId->object, &t);
         }

         if(ret == 0)
         {
            if (pRetCode) *pRetCode = 0;
            return DSL_SUCCESS;
         }

         switch(errno)
         {
            case EINTR:
            if (pRetCode) *pRetCode = 0;
            break;

            case EDEADLK:
            if (pRetCode) *pRetCode = 0;
            break;

            case EINVAL:
            if (pRetCode) *pRetCode = 0;
            break;

            case ETIMEDOUT:
            if (pRetCode) *pRetCode = 1;
            break;
      
            default:
            if (pRetCode) *pRetCode = 0;
            break;
         }
      }
   }

   return DSL_ERROR;   
}


/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int32_t DSL_CPE_LockSet(
               DSL_CPE_Lock_t *lockId)
{
   if(lockId)
   {
      if (DSL_LOCK_INIT_VALID(lockId) == DSL_TRUE)
      {
         if (sem_post(&lockId->object) == 0)
         {
            return DSL_SUCCESS;
         }
      }
   }

   return DSL_ERROR;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_void_t DSL_CPE_Sleep(DSL_uint32_t nSeconds)
{
   sleep(nSeconds);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_void_t DSL_CPE_MSecSleep(DSL_uint32_t nMs)
{
   usleep(nMs * 1000);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_void_t *DSL_CPE_Malloc(DSL_uint32_t size)
{
   DSL_void_t *memblock;

   memblock = (DSL_void_t*) malloc((size_t)size);

   return (memblock);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
void DSL_CPE_Free(DSL_void_t *memblock)
{
   free(memblock);
}

/**
   Open a file.
*/
DSL_CPE_File_t *DSL_CPE_FOpen(const DSL_char_t *name,  const DSL_char_t *mode)
{
   return fopen(name, mode);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_CPE_File_t *DSL_CPE_FMemOpen (DSL_char_t *buf, const DSL_uint32_t size, const DSL_char_t *mode)
{
   return fmemopen(buf, size, mode);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_FClose(DSL_CPE_File_t *fd)
{
   if(fd != 0)
      return fclose(fd);

   return -1;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_FRead(DSL_void_t *buf, DSL_uint32_t size,  DSL_uint32_t count, DSL_CPE_File_t *stream)
{
   return fread(buf, size,  count, stream);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_FWrite(const DSL_void_t *buf, DSL_uint32_t size, DSL_uint32_t count, DSL_CPE_File_t *stream)
{
   return fwrite(buf, size, count, stream);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_FFlush(DSL_CPE_File_t *fd)
{
   return fflush(fd);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_Feof(DSL_CPE_File_t *stream)
{
   return feof(stream);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_char_t *DSL_CPE_FGets(DSL_char_t *str, DSL_int_t n, DSL_CPE_File_t *stream)
{
   return fgets(str, n, stream);
}

/**
  get file status
*/
DSL_int_t DSL_CPE_FStat(DSL_char_t *str, DSL_CPE_stat_t *st)
{
   return stat(str, st);
}

static DSL_int32_t DSL_CPE_UserThreadStartup(
                              DSL_CPE_ThreadCtrl_t *pThrCntrl);


/* ============================================================================
   IFX Linux adaptation - Application Thread handling
   ========================================================================= */

/**
   LINUX Application - Thread stub function. The stub function will be called
   before calling the user defined thread routine. This gives
   us the possibility to add checks etc.

\par Implementation
   Before the stub function enters the user thread routin the following setup will
   be done:
   - make the kernel thread to a daemon
   - asign the parent to the init process (avoid termination if the parent thread dies).
   - setup thread name, and signal handling (if required).
   After this the user thread routine will be entered.

\param 
   pThrCntrl   Thread information data

\return
   - DSL_SUCCESS on success
   - DSL_ERROR on error
*/
static DSL_int32_t DSL_CPE_UserThreadStartup(
                              DSL_CPE_ThreadCtrl_t *pThrCntrl)
{
   DSL_int32_t retVal     = DSL_ERROR;

   if(pThrCntrl)
   {
      if (!pThrCntrl->pThrFct)
      {
         DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "User Thread startup <%s>, missing THR function" DSL_CPE_CRLF, 
              pThrCntrl->thrParams.pName));

         return DSL_ERROR;
      }

      DSL_CCA_DEBUG( DSL_CCA_DBG_MSG,
         (DSL_CPE_PREFIX "User Thread Startup <%s>, TID %d (PID %d) - ENTER" DSL_CPE_CRLF, 
           pThrCntrl->thrParams.pName, pthread_self(), getpid()));

      pThrCntrl->thrParams.bRunning = 1;
      retVal = pThrCntrl->pThrFct(&pThrCntrl->thrParams);
      pThrCntrl->thrParams.bRunning = 0;

      DSL_CCA_DEBUG( DSL_CCA_DBG_MSG,
         (DSL_CPE_PREFIX "User Thread Startup <%s>, TID %d (PID %d) - EXIT" DSL_CPE_CRLF, 
           pThrCntrl->thrParams.pName, pthread_self(), getpid()));
   }
   else
   {
         DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "User Thread startup <%s>, missing control object" DSL_CPE_CRLF));
   }

   return retVal;
}

/**
   LINUX Application - Creates a new thread / task.

\par Implementation
   - Allocate and setup the internal thread control structure.
   - setup the LINUX specific thread parameter (see "init_completion").
   - start the LINUX Kernel thread with the internal stub function (see "kernel_thread")

\param
   pThrCntrl         Pointer to thread control structure. This structure has to 
                     be allocated outside and will be initialized.
\param
   pName             specifies the 8-char thread / task name.
\param
   pThreadFunction   specifies the user entry function of the thread / task.
\param
   nStackSize        specifies the size of the thread stack - not used.
\param
   nPriority         specifies the thread priority, 0 will be ignored
\param
   nArg1             first argument passed to thread / task entry function.
\param
   nArg2             second argument passed to thread / task entry function.

\return
   - DSL_SUCCESS thread was successful started.
   - DSL_ERROR thread was not started
*/
DSL_int32_t DSL_CPE_ThreadInit(
               DSL_CPE_ThreadCtrl_t *pThrCntrl,
               DSL_char_t     *pName,
               DSL_CPE_ThreadFunction_t pThreadFunction,
               DSL_uint32_t   nStackSize,
               DSL_uint32_t   nPriority,
               DSL_uint32_t   nArg1,
               DSL_uint32_t   nArg2)
{
   DSL_int32_t          retVal=0;
   pthread_t            tid;
   pthread_attr_t       attr;

   if(pThrCntrl)
   {
      if (DSL_THREAD_INIT_VALID(pThrCntrl) == DSL_FALSE)
      {
         pthread_attr_init(&attr);
         /*pthread_attr_setstacksize (&attr, nStackSize);*/

         memset(pThrCntrl, 0x00, sizeof(DSL_CPE_ThreadCtrl_t));

         /* set thread function arguments */
         strncpy(pThrCntrl->thrParams.pName, pName, DSL_CPE_THREAD_NAME_LEN);
         pThrCntrl->thrParams.pName[DSL_CPE_THREAD_NAME_LEN-1] = 0;
         pThrCntrl->nPriority = nPriority;
         pThrCntrl->thrParams.nArg1 = nArg1;
         pThrCntrl->thrParams.nArg2 = nArg2;

         /* set thread control settings */
         pThrCntrl->pThrFct = pThreadFunction;

         /*
            create thread with configured attributes
            we call first our own routine for further checks and setup etc.
         */
         retVal = pthread_create (
                     &tid, &attr,
                     (DSL_USER_THREAD_StartRoutine)DSL_CPE_UserThreadStartup,
                     (DSL_void_t*)pThrCntrl);
         if (retVal)
         {
            DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
               (DSL_CPE_PREFIX "User Thread create <%s> - pthread_create = %d" DSL_CPE_CRLF, 
                 (pName ? (pName) : "noname"), errno ));

            return DSL_ERROR;;
         }

         pThrCntrl->tid = tid;
      
         pThrCntrl->bValid = DSL_TRUE;
      
         return DSL_SUCCESS;
      }
      else
      {
         DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "ThreadInit, object already valid" DSL_CPE_CRLF));
      }
   }
   else
   {
      DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX "ThreadInit, missing object" DSL_CPE_CRLF));
   }

   return DSL_ERROR;;
}


/**
   LINUX Application - Shutdown and terminate a given thread.
   Therefore the thread delete functions triggers the user thread function 
   to shutdown. In case of not responce (timeout) the thread will be canceled.

\par Implementation
   - force a shutdown via the shutdown flag.
   - wait for completion (see "wait_for_completion").
   - free previous allocated internal data.

\param
   pThrCntrl   Thread control struct.
\param
   waitTime_ms Time [ms] to wait for "self-shutdown" of the user thread.

\return
   - DSL_SUCCESS thread was successful deleted - thread control struct is freed.
   - DSL_ERROR thread was not deleted
*/
DSL_int32_t DSL_CPE_ThreadDelete(
               DSL_CPE_ThreadCtrl_t *pThrCntrl,
               DSL_uint32_t       waitTime_ms)
{
   DSL_uint32_t   waitCnt = 1;

   if(pThrCntrl)
   {
      if (DSL_THREAD_INIT_VALID(pThrCntrl) == DSL_TRUE)
      {
         if (pThrCntrl->thrParams.bRunning == 1)
         {
            /* trigger user thread routine to shutdown */
            pThrCntrl->thrParams.bShutDown = 1;

            if (waitTime_ms != DSL_THREAD_DELETE_WAIT_FOREVER)
            {
               waitCnt = waitTime_ms / DSL_THREAD_DOWN_WAIT_POLL_MS;
            }

            while (waitCnt && (pThrCntrl->thrParams.bRunning == 1) )
            {
               DSL_CPE_MSecSleep(DSL_THREAD_DOWN_WAIT_POLL_MS);

               if (waitTime_ms != DSL_THREAD_DELETE_WAIT_FOREVER)
                  waitCnt--;
            }
         }
         else
         {
            DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
               ("IFXOS WRN - User Thread Delete <%s> - not running" DSL_CPE_CRLF, 
                 pThrCntrl->thrParams.pName));
         }

         if (pThrCntrl->thrParams.bRunning == 1)
         {
            DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
               ("IFXOS WRN - User Thread Delete <%s> TID %d - kill, no shutdown responce" DSL_CPE_CRLF, 
                 pThrCntrl->thrParams.pName, pThrCntrl->tid));

            /** still running --> kill */
            switch(pthread_cancel(pThrCntrl->tid))
            {
               case 0:
                  pThrCntrl->thrParams.bRunning = 0;
                  break;

               case ESRCH:
                  /* just information that task already exited by itself */
                  DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
                     ("IFXOS WRN - User Thread Delete <%s> TID %d - not found (already exited)" DSL_CPE_CRLF, 
                       pThrCntrl->thrParams.pName, pThrCntrl->tid));
                  pThrCntrl->thrParams.bRunning = 0;
                  break;

               default:
                  DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
                     (DSL_CPE_PREFIX "User Thread Delete <%s> TID %d - unknown (mem loss)" DSL_CPE_CRLF, 
                       pThrCntrl->thrParams.pName, pThrCntrl->tid));
            
                  pThrCntrl->bValid = DSL_FALSE;
            
                  return DSL_ERROR;
            }
         }

         pThrCntrl->bValid = DSL_FALSE;

         return DSL_SUCCESS;
      }
      else
      {
         DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "ThreadDelete, invalid object" DSL_CPE_CRLF));
      }
   }
   else
   {
      DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX "ThreadDelete, missing object" DSL_CPE_CRLF));
   }

   return DSL_ERROR;

}


/**
   LINUX Application - Shutdown a given thread.
   Therefore the thread delete functions triggers the user thread function 
   to shutdown.

\par Implementation
   - force a shutdown via the shutdown flag.
   - wait for completion (see "wait_for_completion").
   - free previous allocated internal data.

\param
   pThrCntrl   Thread control struct.
\param
   waitTime_ms Time [ms] to wait for "self-shutdown" of the user thread.

\return
   - DSL_SUCCESS successful shutdown - thread control struct is freed.
   - DSL_ERROR  no success, thread struct still exists.
*/
DSL_int32_t DSL_CPE_ThreadShutdown(
               DSL_CPE_ThreadCtrl_t *pThrCntrl,
               DSL_uint32_t       waitTime_ms)
{
   DSL_uint32_t   waitCnt = 1;

   if(pThrCntrl)
   {
      if (DSL_THREAD_INIT_VALID(pThrCntrl) == DSL_TRUE)
      {
         if (pThrCntrl->thrParams.bRunning == 1)
         {
            /* trigger user thread routine to shutdown */
            pThrCntrl->thrParams.bShutDown = 1;

            if (waitTime_ms != DSL_THREAD_DELETE_WAIT_FOREVER)
            {
               waitCnt = waitTime_ms / DSL_THREAD_DOWN_WAIT_POLL_MS;
            }

            while (waitCnt && (pThrCntrl->thrParams.bRunning == 1) )
            {
               DSL_CPE_MSecSleep(DSL_THREAD_DOWN_WAIT_POLL_MS);

               if (waitTime_ms != DSL_THREAD_DELETE_WAIT_FOREVER)
                  waitCnt--;
            }
         }
         else
         {
            DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
               ("IFXOS WRN - User Thread Shutdown <%s> - not running" DSL_CPE_CRLF, 
                 pThrCntrl->thrParams.pName));
         }

         if (pThrCntrl->thrParams.bRunning != 0)
         {
            DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
               (DSL_CPE_PREFIX "User Thread Shutdown <%s> - no responce" DSL_CPE_CRLF, 
                 pThrCntrl->thrParams.pName));

            pThrCntrl->bValid = DSL_FALSE;

            return DSL_ERROR;
         }

         pThrCntrl->bValid = DSL_FALSE;

         return DSL_SUCCESS;
      }
      else
      {
         DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "Thread Shutdown, invalid object" DSL_CPE_CRLF));
      }
   }
   else
   {
      DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX "Thread Shutdown, missing object" DSL_CPE_CRLF));
   }

   return DSL_ERROR;
}

/**
   LINUX Application - Modify own thread priority.

\todo
   Under discussion how to handle the priority!

\param
   newPriority - new thread priority.
                 Possible Values are:
                 - DSL_THREAD_PRIO_IDLE
                 - DSL_THREAD_PRIO_LOWEST
                 - DSL_THREAD_PRIO_LOW
                 - DSL_THREAD_PRIO_NORMAL
                 - DSL_THREAD_PRIO_HIGH
                 - DSL_THREAD_PRIO_HIGHEST
                 - DSL_THREAD_PRIO_TIME_CRITICAL
\attention
   The intention for the priority "TIME_CRITICAL" is for use within 
   driver space.

\return
   - DSL_SUCCESS priority changed.
   - DSL_ERROR priority not changed.
*/
DSL_int32_t DSL_CPE_ThreadPriorityModify(
               DSL_uint32_t       newPriority)
{
   struct sched_param param;
   int ret=0, policy=0;
   
   memset(&param, 0x00, sizeof(param));

   ret = pthread_getschedparam (pthread_self(), &policy, &param);
   if(ret == 0)
   {
      param.sched_priority = +newPriority;
      
      /* fix the scheduler to FIFO to be able to increase the priority */
      policy = SCHED_FIFO;

      switch(policy)
      {
         case SCHED_OTHER:
            DSL_CCA_DEBUG( DSL_CCA_DBG_MSG,
               (DSL_CPE_PREFIX "Thread, using SCHED_OTHER (regular, non-realtime scheduling)" DSL_CPE_CRLF));
            break;
        
         case SCHED_RR:
            DSL_CCA_DEBUG( DSL_CCA_DBG_MSG,
               (DSL_CPE_PREFIX "Thread, using SCHED_RR (realtime, round-robin)" DSL_CPE_CRLF));
            break;
         
         case SCHED_FIFO:
            DSL_CCA_DEBUG( DSL_CCA_DBG_MSG,
               (DSL_CPE_PREFIX "Thread, using SCHED_FIFO (realtime, first-in first-out)" DSL_CPE_CRLF));
            break;
         
         default:
            DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
               (DSL_CPE_PREFIX "Thread, priority not defined %d" DSL_CPE_CRLF, policy));
            break;
      }

      /* setting the new priority */
      ret = pthread_setschedparam (pthread_self(), policy, &param);
      if(ret == 0)
      {            
         DSL_CCA_DEBUG( DSL_CCA_DBG_MSG,
            (DSL_CPE_PREFIX "Thread, Set new priority %d, policy %d" DSL_CPE_CRLF, 
              param.sched_priority, policy));

         return DSL_SUCCESS;
      }
      else
      {
         DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "Thread, Set of new priority %d failed (pid %d)" DSL_CPE_CRLF, 
              param.sched_priority, (int)pthread_self()));
      }
   }
   else
   {
      DSL_CCA_DEBUG( DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX "Thread, Get of priority failed (pid %d)" DSL_CPE_CRLF, 
           (int)pthread_self()));
   }

   switch(ret)
   {
      case ENOSYS:
         DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
            (DSL_CPE_PREFIX "Thread, The option _POSIX_THREAD_PRIORITY_SCHEDULING is not defined and the "
             "implementation does not support the function." DSL_CPE_CRLF));
         break;

      case ESRCH:
         DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
            (DSL_CPE_PREFIX "Thread, The value specified by thread does not refer to a existing thread." DSL_CPE_CRLF));
      break;

      case EINVAL:
         DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
            (DSL_CPE_PREFIX "Thread, The value specified by policy or one of the scheduling parameters "
             "associated with the scheduling policy policy is invalid." DSL_CPE_CRLF));
         break;

      case ENOTSUP:
         DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
            (DSL_CPE_PREFIX "Thread, An attempt was made to set the policy or scheduling parameters "
             "to an unsupported value." DSL_CPE_CRLF));
         break;

      case EPERM:
         DSL_CCA_DEBUG( DSL_CCA_DBG_WRN,
            (DSL_CPE_PREFIX "Thread, The caller does not have the appropriate permission to set either "
             "the scheduling parameters or the scheduling policy of the specified thread." DSL_CPE_CRLF));
         break;
   }

   return DSL_ERROR;
}


/**
   Return the own thread / task ID

\return
   Thread ID of the current thread.
*/
DSL_CPE_Thread_t DSL_CPE_ThreadIdGet(void)
{
   return pthread_self();
}

void DSL_CPE_EchoOff(void)
{
   struct termios new_settings;
   tcgetattr(fileno(stdout),&stored_stdout_settings);
   new_settings = stored_stdout_settings;
   new_settings.c_lflag &= (~ECHO);
   tcsetattr(fileno(stdout),TCSANOW,&new_settings);
   return;
}

void DSL_CPE_EchoOn(void)
{
   tcsetattr(fileno(stdout),TCSANOW,&stored_stdout_settings);
   return;
}

DSL_Socket_t DSL_CPE_Accept(
                  DSL_Socket_t    socFd, 
                  DSL_sockaddr_in_t  *pSocAddr)
{
   DSL_int_t addrlen = sizeof (struct sockaddr);

   return (DSL_Socket_t)accept((int)socFd, (struct sockaddr *)pSocAddr, (socklen_t*)&addrlen);
}

DSL_int_t DSL_CPE_Socket(
                  DSL_int_t socType, 
                  DSL_Socket_t *pSocketFd)
{
   if (pSocketFd == DSL_NULL)
   {
      return -1;
   }

   /* arg3 = 0: do not specifiy the protocol */
   if((*pSocketFd = socket(AF_INET, socType, 0)) == -1)
   {
      return -1;
   }

   return 0;
} 

DSL_int_t DSL_CPE_SocketBind(
                  DSL_Socket_t    socFd, 
                  DSL_sockaddr_in_t  *pSocAddr)
{
   DSL_int_t ret;

   if (pSocAddr == DSL_NULL)
   {
      return -1;
   }

   ret = bind(
            (int)socFd,
            (struct sockaddr*)pSocAddr,
            sizeof(struct sockaddr_in));

   if (ret != 0)
   {
      return -1;
   }

   return 0;
}
#endif /* #ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_FPrintf(DSL_CPE_File_t *stream, const DSL_char_t *format, ...)
{
   va_list ap;   /* points to each unnamed arg in turn */
   DSL_int_t nRet = 0;

   va_start(ap, format);   /* set ap pointer to 1st unnamed arg */

   nRet = vfprintf(stream, format, ap);
   fflush(stream);

   va_end(ap);

   return nRet;
}

/*
   [KAv]: IFXOS mapping for this function causes CLI console hung
*/
void DSL_CPE_KeypressSet(void)
{
   struct termios new_settings;

   tcgetattr(fileno(stdin),&stored_stdin_settings);
   new_settings = stored_stdin_settings;

   /* Disable canonical mode */
   new_settings.c_lflag &= ~(ICANON);
   /* set buffer size to 0 byte / timeout 100 ms */
   new_settings.c_cc[VTIME] = 10;
   new_settings.c_cc[VMIN]  = 1;

   tcsetattr(fileno(stdin),TCSANOW,&new_settings);
   return;
}

void DSL_CPE_KeypressReset(void)
{
   tcsetattr(fileno(stdin),TCSANOW,&stored_stdin_settings);
   return;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_Error_t DSL_CPE_SetEnv(const DSL_char_t *sName, const DSL_char_t *sValue)
{
   DSL_Error_t nRet = DSL_ERROR;

   nRet = ((setenv(sName, sValue, 1) == 0) ? DSL_SUCCESS : DSL_ERROR);

   if (nRet == DSL_ERROR)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "Failed to set envvar (%s=%s)!" DSL_CPE_CRLF, sName, sValue));
   }
   else
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Set envvar (%s=%s)"
         DSL_CPE_CRLF, sName, sValue));
   }

   return (nRet);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
#define DSL_CPE_LINUX_CMD_ADD " 2> /dev/null"
DSL_Error_t DSL_CPE_System(const DSL_char_t *sCommand)
{
   DSL_int_t nRet;
   DSL_char_t *sCmd;

   sCmd = DSL_CPE_Malloc(strlen(sCommand) + strlen(DSL_CPE_LINUX_CMD_ADD) + 1);
   if (sCmd == DSL_NULL)
   {
      return DSL_ERR_MEMORY;
   }
   else
   {
      sCmd[0] = 0;
      strcat(sCmd, sCommand);
      strcat(sCmd, DSL_CPE_LINUX_CMD_ADD);
      nRet = system(sCmd);
      DSL_CPE_Free(sCmd);
   }

   return nRet == 0 ? DSL_SUCCESS : DSL_ERROR;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
int DSL_CPE_debug_printf(DSL_char_t *fmt, ...)
{
   va_list ap;   /* points to each unnamed arg in turn */
   DSL_int_t nRet = 0;

   va_start(ap, fmt);   /* set ap pointer to 1st unnamed arg */

   nRet = vprintf(fmt, ap);

   va_end(ap);

   return nRet;
}

/**
   Segmentation fault handler. Will print the current thread id.

   \param val not used
*/
static void DSL_CPE_SegmentationFault(int val)
{
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "Segmentation fault, thread ID is %d, PID %d" DSL_CPE_CRLF,
      (int)DSL_CPE_ThreadIdGet(), getpid()));

   sleep(10000);
}

/**
   Installing of system handler routines.
*/
DSL_void_t DSL_CPE_HandlerInstall(DSL_void_t)
{
   signal(SIGSEGV, DSL_CPE_SegmentationFault);
   /* ignore broken pipes */
   signal(SIGPIPE, SIG_IGN);
}

DSL_uint16_t DSL_CPE_Htons(DSL_uint16_t hVal)
{
   return htons(hVal);
}

DSL_uint32_t DSL_CPE_Htonl(DSL_uint32_t hVal)
{
   return htonl(hVal);
}

#ifdef DSL_DEBUG_TOOL_INTERFACE
DSL_char_t* DSL_CPE_OwnAddrStringGet(DSL_void_t)
{
   DSL_int_t nFd;
   struct ifreq ifr;
   DSL_char_t *pString = DSL_NULL;

   /* obtain ip-address automatically */
   nFd = socket(AF_INET, SOCK_DGRAM, 0);
   ifr.ifr_addr.sa_family = AF_INET;
   strncpy(ifr.ifr_name, DSL_DEBUG_TOOL_INTERFACE_DEFAULT_IFACE, IFNAMSIZ-1);
   DSL_CPE_Ioctl(nFd, SIOCGIFADDR, (DSL_int_t) &ifr);
   close(nFd);

   pString =
      DSL_CPE_Malloc( strlen(inet_ntoa(
      ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)) + 1);

   strcpy (pString, inet_ntoa(
     ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

   return pString;
}

#endif /* DSL_DEBUG_TOOL_INTERFACE*/

#endif /* #ifndef _lint*/

#endif /* LINUX */

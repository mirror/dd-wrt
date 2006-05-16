/******************************************************************************/
/* File: mutualexclusion.cpp                                                  */
/******************************************************************************/
/* This source code file is the source file for the ThreadBase class - this   */
/* class is used to provide basic thread functionality within a C++ class.    */
/******************************************************************************/
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/*                                                                            */
/*    Version 1.01 - Support for Solaris, many thanks to Andreas Almroth for  */
/*                   his effort.                                              */
/*                                                                            */
/*    Version 1.10 - Linux supports standard POSIX Mutual Exclusion types and */
/*                   now reverting to those types instead. No change to Linux */
/*                   compile as the corresponding macros were equal anyway.   */
/*                 - Solaris support now provided by previous change.         */
/*                 - Support for MacOS X.1.?, many thanks to Desmond Schmidt. */
/******************************************************************************/

/******************************************************************************/
/* Platform specific code.                                                    */
/*                                                                            */
/* This section allows for platform specific code.  Library function calls    */
/* which are not supported on certain platforms are replaced with MACROS that */
/* translate to the APIs for the given platform.                              */
/******************************************************************************/
/* MacOS X.1.?                                                                */
/* - The pthread_mutexattr_settype() function does not exist so is defined to */
/*   a NULL statement.                                                        */
/* - Similarly, the MutEx type macros are not defined, since we do not use    */
/*   them on MacOS X.1, it doesn't matter what we define them to be.          */
/******************************************************************************/
#ifdef PLATFORM_macosx_1

#define pthread_mutexattr_settype(p,q)

#define PTHREAD_MUTEX_NORMAL            1

#define PTHREAD_MUTEX_RECURSIVE         1

#define PTHREAD_MUTEX_ERRORCHECK        1

#warning "MacOS X.1 compile, Only fastMutEx type Mutual Exclusions are defined - all other types will revert to this type"

#endif

/******************************************************************************/
/* Full use of Mutual Exclusion Types.                                        */
/******************************************************************************/
#define _GNU_SOURCE

/******************************************************************************/
/* Standard C Includes.                                                       */
/******************************************************************************/
#include <errno.h>

/******************************************************************************/
/* Include library header file.                                                 */
/******************************************************************************/
#include "pthreadcc.h"

/******************************************************************************/
/* MutualExclusion Constructor.                                               */
/*                                                                            */
/* We must create the internal MutEx of the right type.  Based on the setup   */
/* parameter mtType, we set the mutexTheMutEx variable to be initialised with */
/* the appropriate type parameters.                                           */
/******************************************************************************/
MutualExclusion::MutualExclusion(MutExType mtType)
{
    pthread_mutexattr_t     mattrDetails;

    pthread_mutexattr_init(&mattrDetails);
    switch (mtType)
    {
        case fastMutEx:         pthread_mutexattr_settype(&mattrDetails, PTHREAD_MUTEX_NORMAL);
                                break;
        case recursiveMutEx:    pthread_mutexattr_settype(&mattrDetails, PTHREAD_MUTEX_RECURSIVE);
                                break;
        case errorcheckMutEx:	pthread_mutexattr_settype(&mattrDetails, PTHREAD_MUTEX_ERRORCHECK);
                                break;
    }
    pthread_mutex_init(&mutexTheMutex, &mattrDetails);
    pthread_mutexattr_destroy(&mattrDetails);
}

/******************************************************************************/
/* MutualExclusion Destructor.                                                */
/*                                                                            */
/* The class cannot be destroyed until the internal MutEx is also destroyed.  */
/* We do this by calling pthread_mutex_destroy() until it returns a success   */
/* code.  If the MutEx is currently locked then we attempt to lock the MutEx  */
/* to wait for it to become free, we then immediately Unlock the mutex and    */
/* try to destroy it again.                                                   */
/******************************************************************************/
MutualExclusion::~MutualExclusion()
{
    while (pthread_mutex_destroy(&mutexTheMutex) == EBUSY)
    {
        Lock();
        Unlock();
    }
}

/******************************************************************************/
/* void Lock()                                                                */
/*                                                                            */
/* This method attempts to lock the MutEx and returns true if successful, or  */
/* throws an exception on error.  If the MutEx is of an error checking type   */
/* and the MutEx is alreadylocked by the calling thread, a MutexWouldDeadlock */
/* exception is thrown.  The lock is done by calling the pthread_mutex_lock() */
/* function.  If 0 is returned, the MutEx is now locked and we can return, if */
/* EDEADLK is returned then we cannot lock the error checking MutEx without   */
/* causing a deadlock and we throw the exception.  Any other return values    */
/* signifies either an unknown error type or that the MutEx has not been      */
/* properly initialised, which CANNOT happen within this class, an Unknown    */
/* MutEx exception is thrown.                                                 */
/******************************************************************************/
void
MutualExclusion::Lock()
{
    switch (pthread_mutex_lock(&mutexTheMutex))
    {
        case 0:         return;
        case EDEADLK:   throw ThreadException(ThreadException::errMutexWouldDeadlock);
        default:        throw ThreadException(ThreadException::errMutexUnknown);
    }
}

/******************************************************************************/
/* void UnLock()                                                              */
/*                                                                            */
/* This method attempts to unlock the MutEx and returns if successful, or     */
/* throws an exception on error.  If the MutEx is of an error checking type   */
/* and the calling thread did not lock the MutEx, a MutExNotOwned exception   */
/* is thrown.  This is done by calling the pthread_mutex_unlock() function.   */
/* If 0 is returned, the MutEx is now unlocked and we can return, if EPERM is */
/* returned then we cannot unlock the error checking MutEx and we throw the   */
/* exception.  Any other return values signifies either an unknown error type */
/* or that the MutEx has not been properly initialised, which CANNOT happen   */
/* within this class, an Unknown MutEx exception is thrown.                   */
/******************************************************************************/
void
MutualExclusion::Unlock()
{
    switch (pthread_mutex_unlock(&mutexTheMutex))
    {
        case 0:         return;
        case EPERM:     throw ThreadException(ThreadException::errMutexNotOwned);
        default:        throw ThreadException(ThreadException::errMutexUnknown);
    }
}

/******************************************************************************/
/* bool TryLock()                                                             */
/*                                                                            */
/* This method attempts to lock the MutEx and returns true if successful and  */
/* false if the MutEx is already locked.  This is done by calling the         */
/* pthread_mutex_trylock() function.  If 0 is returned, the MutEx is now      */
/* locked and we can return true, if EBUSY is returned then we cannot lock    */
/* the MutEx and we return false.  Any other return values signifies either   */
/* an unknown error type or that the MutEx has not been properly initialised, */
/* which CANNOT happen within this class so we report a major error and       */
/* terminate the program.                                                     */
/******************************************************************************/
bool
MutualExclusion::TryLock()
{
    switch (pthread_mutex_trylock(&mutexTheMutex))
    {
        case 0:         return true;
        case EBUSY:     return false;
        default:        throw ThreadException(ThreadException::errMutexUnknown);
    }
}

/******************************************************************************/
/* End of File: mutualexclusion.cpp                                           */
/******************************************************************************/

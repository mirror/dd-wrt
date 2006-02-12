/******************************************************************************/
/* File: condition.cpp                                                        */
/******************************************************************************/
/* This source code file is the source file for the ThreadBase class - this   */
/* class is used to provide basic thread functionality within a C++ class.    */
/******************************************************************************/
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/*                                                                            */
/*    Version 1.01 - Support for Solaris, many thanks to Andreas Almroth for  */
/*                   his effort, now include base <errno.h> instead of linux  */
/*                   only version.                                            */
/*                                                                            */
/*    Version 1.10 - Support for MacOS X.1.?, many thanks to Desmond Schmidt. */
/******************************************************************************/

/******************************************************************************/
/* Platform specific code.                                                    */
/*                                                                            */
/* This section allows for platform specific code.  Library function calls    */
/* which are not supported on certain platforms are replaced with MACROS that */
/* translate to the APIs for the given platform.                              */
/******************************************************************************/
/* MacOS X.1.?                                                                */
/* - The pthread_condattr_init() function does not exist so is defined to a   */
/*   NULL statement.                                                          */
/* - The pthread_cond_init() function must be called with the attributes set  */
/*   to NULL.                                                                 */
/* - The pthread_condattr_destroy() function also does not exist and is also  */
/*   defined to a NULL statement.                                             */
/******************************************************************************/
#ifdef PLATFORM_macosx_1

#define pthread_condattr_init(p)

#define pthread_cond_init(p, q)         pthread_cond_init(p,NULL)

#define pthread_condattr_destroy(p)

#warning "MacOS X.1 compile, please ignore unused variable warning in condition.cpp:63"

#endif

/******************************************************************************/
/* Standard C Includes.                                                       */
/******************************************************************************/
#include <errno.h>

/******************************************************************************/
/* Include library header file.                                               */
/******************************************************************************/
#include "pthreadcc.h"

/******************************************************************************/
/* Condition Constructor.                                                     */
/*                                                                            */
/* We must create the internal MutEx of the right type.  Based on the setup   */
/* parameter mtType, we set the mutexTheMutEx variable to be initialised with */
/* the appropriate type parameters.                                           */
/******************************************************************************/
Condition::Condition()
{
    pthread_condattr_t     cattrDetails;

    pthread_condattr_init(&cattrDetails);
    pthread_cond_init(&condTheCondition, &cattrDetails);
    pthread_condattr_destroy(&cattrDetails);
}

/******************************************************************************/
/* Condition Destructor.                                                      */
/*                                                                            */
/* The class cannot be destroyed until the internal MutEx is also destroyed.  */
/* We do this by calling pthread_mutex_destroy() until it returns a success   */
/* code.  If the MutEx is currently locked then we attempt to lock the MutEx  */
/* to wait for it to become free, we then immediately Unlock the mutex and    */
/* try to destroy it again.                                                   */
/******************************************************************************/
Condition::~Condition()
{
    while (pthread_cond_destroy(&condTheCondition) == EBUSY)
    {
        Broadcast();
    }
}

/******************************************************************************/
/* void Wait()                                                                */
/*                                                                            */
/* This method blocks until the condition is either signalled or broadcast by */
/* another thread.  The pthread_cond_wait() call first atomically unlocks the */
/* the mutual exclusion, waits for the condition to be signalled and then     */
/* locks the mutual exclusion again.  This function call never returns an     */
/* error code.                                                                */
/******************************************************************************/
void
Condition::Wait()
{
    pthread_cond_wait(&condTheCondition, &(mutexCondLock.mutexTheMutex));
}

/******************************************************************************/
/* void Signal()                                                              */
/*                                                                            */
/* This method signals the condition such that one of the 0..n threads        */
/* waiting on that condition will be unblocked.  It is not specified which of */
/* the number of waits will be unblocked.  The pthread_cond_signal() function */
/* never returns an error code.                                               */
/******************************************************************************/
void
Condition::Signal()
{
    pthread_cond_signal(&condTheCondition);
}

/******************************************************************************/
/* void Broadcast()                                                           */
/*                                                                            */
/* This method signals the condition such that all of the 0..n threads        */
/* waiting on that condition will be unblocked.  The pthread_cond_broadcast() */
/* function never returns an error code.                                      */
/******************************************************************************/
void
Condition::Broadcast()
{
    pthread_cond_broadcast(&condTheCondition);
}

/******************************************************************************/
/* End of File: condition.cpp                                                 */
/******************************************************************************/

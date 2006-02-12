/******************************************************************************/
/* File: semaphore.cpp                                                        */
/******************************************************************************/
/* This source code file is the source file for the Semaphore class - this    */
/* class wraps the Linux Semaphore functionality into an object.              */
/******************************************************************************/
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/******************************************************************************/

/******************************************************************************/
/* Standard C Includes.                                                       */
/******************************************************************************/
#include <unistd.h>

/******************************************************************************/
/* Include semaphore library and error information.                           */
/******************************************************************************/
#include <semaphore.h>
#include <errno.h>

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "pthreadcc.h"

/******************************************************************************/
/* Semaphore Constructor.                                                     */
/*                                                                            */
/* We must create the internal Semaphore with the provided initial value.  If */
/* the creation fails because the provided initial value is too high, a Max.  */
/* Semaphore Count exception is thrown, if the creation fails for another     */
/* reason, an Unknown Semaphore Error exception is thrown.                    */
/******************************************************************************/
Semaphore::Semaphore(unsigned int uiInitialValue)
{
    if (sem_init(&semTheSemaphore, 0, uiInitialValue))
    {
        switch (errno)
        {
            case EINVAL:    throw ThreadException(ThreadException::errMaxSemCount);
            default:        throw ThreadException(ThreadException::errSemUnknown);
        }
    }
}

/******************************************************************************/
/* Semaphore Destructor.                                                      */
/*                                                                            */
/* The class cannot be destroyed until the internal Semaphore is also         */
/* destroyed.  We do this by calling sem_destroy() until it returns a success */
/* code.  If threads are waiting on the Semaphore, we repeatedly call Post()  */
/* to unblock those threads until their are no waiting threads and we can     */
/* destrot the semaphore.                                                     */
/******************************************************************************/
Semaphore::~Semaphore()
{
    while (sem_destroy(&semTheSemaphore) == EBUSY)
    {
        Post();
    }
}

/******************************************************************************/
/* void Wait()                                                                */
/*                                                                            */
/* This method waits for the semaphore.  If the current semaphore count is    */
/* positive, it decrements the count and returns.  If the current count is    */
/* zero, the call blocks until the count changes, it then decrements the      */
/* count and returns.  If the call blocks, another thread must call Post()    */
/* for this call to unblock.                                                  */
/******************************************************************************/
void
Semaphore::Wait()
{
    sem_wait(&semTheSemaphore);
}

/******************************************************************************/
/* void Post()                                                                */
/*                                                                            */
/* This method signals the semaphore by incrementing the current semaphore    */
/* count.  If this procedure would increment the count above the maximum      */
/* supported value, the sem_post() call fails and errno is set to ERANGE.  In */
/* this case we throw a Max. Semaphore Count exception, if neither of these   */
/* cases occurs we throw an Unknwon Semaphore Error exception.                */
/******************************************************************************/
void
Semaphore::Post()
{
    if (sem_post(&semTheSemaphore))
    {
        switch (errno)
        {
            case ERANGE:    throw ThreadException(ThreadException::errMaxSemCount);
            default:        throw ThreadException(ThreadException::errSemUnknown);
        }
    }
}

/******************************************************************************/
/* bool TryWait()                                                             */
/*                                                                            */
/* This method attempts to wait for the semaphore.  If the current semaphore  */
/* count is positive, it decrements the count and returns, just like Wait().  */
/* If the current count is zero, sem_trywait() fails and errno is set to      */
/* EAGAIN, the call returns false to signify that the wait would block.  If   */
/* neither of these cases occurs we have an unknown error and terminate.      */
/******************************************************************************/
bool
Semaphore::TryWait()
{
    if (sem_trywait(&semTheSemaphore))
    {
        switch (errno)
        {
            case EAGAIN:    return false;
            default:        throw ThreadException(ThreadException::errSemUnknown);
        }
    }
    return true;
}

/******************************************************************************/
/* int GetValue()                                                             */
/*                                                                            */
/* This method returns the current count value of the Semaphore.  We do this  */
/* by calling sem_getvalue(), this function always returns zero so we do not  */
/* need to check the return code.  We return the value returned by this       */
/* function call.                                                             */
/******************************************************************************/
int
Semaphore::GetValue()
{
    int iResult;

    sem_getvalue(&semTheSemaphore, &iResult);
    return iResult;
}

/******************************************************************************/
/* End of File: semaphore.cpp                                                 */
/******************************************************************************/

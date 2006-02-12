/******************************************************************************/
/* File: threadbase.cpp                                                       */
/******************************************************************************/
/* This source code file is the source file for the ThreadBase class - this   */
/* class is used to provide basic thread functionality within a C++ class.    */
/******************************************************************************/
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/******************************************************************************/

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "pthreadcc.h"

/******************************************************************************/
/* void LaunchThread()                                                        */
/*                                                                            */
/* This function is called to create and begin executing the thread that is   */
/* maintained by this class.  We call pthread_create to create and begin      */
/* executing the thread.  The thread parameter is this so that the static     */
/* method can resolve the original calling class to run the appropriate code. */
/* The function throws a ThreadLaunch exception if the thread could not be    */
/* created and run.                                                           */
/******************************************************************************/
void
ThreadBase::LaunchThread()
{
    if (pthread_create(&pthreadThreadDetails, NULL, &(ThreadBase::ThreadMain), this))
    {
        throw ThreadException(ThreadException::errThreadLaunch);
    }
}

/******************************************************************************/
/* void WaitThread()                                                          */
/*                                                                            */
/* This member method causes the calling thread to block until the thread     */
/* maintained by this class is no longer running.  This method should NOT be  */
/* called from within the class maintained thread.  First we lock the MutEx   */
/* within the Thread Finished Condition to protect our access to the          */
/* bThreadRunning flag.  While the flag is set we wait for the condition to   */
/* be signalled, this will unlock the MutEx to allow the SignalThreadDead()   */
/* method to reset the bThreadRunning flag.  Once the condition is signalled, */
/* the Wait() call will reacquire the MutEx.  If the flag is still not set, we*/
/* can loop and wait again.  Once the flag is reset, the thread is no longer  */
/* running so we unlock the MutEx and return to allow processing to continue. */
/******************************************************************************/
void
ThreadBase::WaitThread()
{
    condThreadFinished.LockMutEx();
    while (bThreadRunning)
	{
        condThreadFinished.Wait();
    }
    condThreadFinished.UnlockMutEx();
}

/******************************************************************************/
/* void * ThreadMain(void *pvOwner)                                           */
/*                                                                            */
/* This static function is executed as the actual thread being launched.  The */
/* instance of the class is passed as pvOwner as a (void *) and must be cast  */
/* to (ThreadBase *) before it can be used.  We then call the Initialise(),   */
/* Execute() and CleanUp() methods on the class to execute the thread, if the */
/* first two of these functions return a non-NULL result, then there is a     */
/* failure and we must terminate the thread.  When terminating, normally or   */
/* prematurely, we must first call CleanUp() to free any resources before     */
/* signalling that the thread has terminated.  We do this by setting the      */
/* thread running flag to false and signalling the thread finished Condition. */
/* This is done within a lock/unlock of the signal to prevent any possible    */
/* race conditions from occuring.                                             */
/******************************************************************************/
void *
ThreadBase::ThreadMain(void *pvOwner)
{
    ThreadBase  *pcOwner = (ThreadBase *)pvOwner;
    void        *pvResult;

    pcOwner->bThreadRunning = true;

    if ((pvResult = pcOwner->Initialise()) == NULL)
    {
        pvResult = pcOwner->Execute();
    }

    pcOwner->CleanUp();
    pcOwner->SignalThreadDead();

    return pvResult;
}

/******************************************************************************/
/* void SignalThreadDead()                                                    */
/*                                                                            */
/* This private member method is called by the ThreadMain function to signal  */
/* that the thread operated by the class is dead, allowing the destructor to  */
/* unblock and delete resources.  First we lock the MutEx within the Thread   */
/* Finished Condition to protect our access to bThreadRunning.  We then reset */
/* the bThreadRunning flag to signify the thread is no longer running, signal */
/* the condition to unblock any threads waiting on the condition (within the  */
/* class destructor) and finally unlock the mutex to allow the the waiting    */
/* call to commence executing.                                                */
/******************************************************************************/
void
ThreadBase::SignalThreadDead()
{
    condThreadFinished.LockMutEx();
    bThreadRunning = false;
    condThreadFinished.Signal();
    condThreadFinished.UnlockMutEx();
}

/******************************************************************************/
/* End of File: threadbase.cpp                                                */
/******************************************************************************/

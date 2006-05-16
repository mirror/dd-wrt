/******************************************************************************/
/* File: pthreadcc.h                                                          */
/******************************************************************************/
/* This source code file is a header file for a series of C++ classes that    */
/* encapsulate the pthread libraries and other concurrent programming tools.  */
/* The C++ classes wrap the C function calls to provide the same functionality*/
/* to object oriented programmers.                                            */
/******************************************************************************/
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/******************************************************************************/

/******************************************************************************/
/* Check to see if already included.                                          */
/******************************************************************************/
#ifndef PTHREADCC_H
#define PTHREADCC_H

/******************************************************************************/
/* Include definitions required to define classes.                            */
/******************************************************************************/
#include <pthread.h>
#include <semaphore.h>

/******************************************************************************/
/* class ThreadException.                                                     */
/*                                                                            */
/* This class is used to store the exception details when an error occurs.    */
/* When any class method call in the pthreadcc library fails, it throws an    */
/* instance of ThreadException, this can be caught and queried for the actual */
/* error code.  The exception can be cast to either the error code or a char  */
/* pointer for a textual description of the error.                            */
/******************************************************************************/
class ThreadException
{
    public:
        enum ErrorCodes { errMutexUnknown, errMutexWouldDeadlock, errMutexNotOwned,
                          errSemUnknown, errMaxSemCount, errThreadLaunch,
                          errUnknown };

                        ThreadException(ErrorCodes ecType = errUnknown);
                        ThreadException(const ThreadException &cOriginal);
                        ~ThreadException();

        operator        const ErrorCodes() { return ecException; }
        operator        const char*() { return pcExceptionString; }

    private:
        ErrorCodes      ecException;
        char            *pcExceptionString;

        void            SetString(const char *pcErrString);
};

/******************************************************************************/
/* class MutualExclusion.                                                     */
/*                                                                            */
/* This class encapsulates the basic MutEx functionality into a C++ class.    */
/* The constructor is used to create the MutEx of a certain type, the default */
/* type being fastMutEx.  The destructor will destroy the MutEx.  The Lock()  */
/* and Unlock() methods do as expected while TryLock() will lock the MutEx if */
/* possible but will not block if locking is not possible.  Details of these  */
/* member methods are:                                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* MutEx Type |                     |                     |                   */
/*---------   |      fastMutEx      |   recursiveMutEx    |  errorcheckMutEx  */
/* Method |   |                     |                     |                   */
/*----------------------------------------------------------------------------*/
/* Lock()     | Block until MutEx   | Block until MutEx   | Block until MutEx */
/*            | is free.  If MutEx  | is free.  If Mutex  | is free. If Mutex */
/*            | is owned by calling | is owned by calling | is owned by the   */
/*            | thread, Deadlock    | thread, return true | calling thread,   */
/*            | occurs.             | and increment count | throw a MutEx     */
/*            |                     | of number of locks. | Would Deadlock    */
/*            |                     |                     | exception.        */
/*----------------------------------------------------------------------------*/
/* Unlock()   | Unlock the MutEx.   | Decrement count of  | If the MutEx is   */
/*            |                     | locks, if 0, unlock | currently locked  */
/*            |                     | the MutEx.          | by the calling    */
/*            |                     |                     | thread then       */
/*            |                     |                     | unlock the MutEx, */
/*            |                     |                     | otherwise throw a */
/*            |                     |                     | MutEx Not Owned   */
/*            |                     |                     | exception.        */
/*----------------------------------------------------------------------------*/
/* TryLock()  | Try to lock the     | Same functionality  | Try to lock the   */
/*            | MutEx, return true  | as Lock()           | MutEx, return true*/
/*            | if successful, or   |                     | if successful, or */
/*            | false if MutEx is   |                     | false if MutEx is */
/*            | already locked.     |                     | already locked.   */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* NOTE: The Condition Class is a friend of the MutualExclusion class since   */
/* it needs access to the private mutexTheMutex member variable.              */
/******************************************************************************/
class MutualExclusion
{
    friend class Condition;

    public:
        enum MutExType          { fastMutEx, recursiveMutEx, errorcheckMutEx };

                                MutualExclusion(MutExType mtType = fastMutEx);
        virtual                 ~MutualExclusion();

        void                    Lock();
        void                    Unlock();

        bool                    TryLock();

    private:
        pthread_mutex_t         mutexTheMutex;
};

/******************************************************************************/
/* class Semaphore.                                                           */
/*                                                                            */
/* This class encapsulates the basic Semaphore functionality into a C++ class.*/
/* The constructor is used to create the Semaphore with the provided initial  */
/* value, the default initial value is zero.  The destructor will destroy the */
/* Semaphore.  The member methods are:                                        */
/*                                                                            */
/* Wait()     : Atomic operation - if the semaphore count value is positive,  */
/*              decrement the count value and return, otherwise block until   */
/*              this is possible.                                             */
/* Post()     : Atomic operation - increment the semaphore count value and    */
/*              return.  If the count value would overflow, throws a Mex.     */
/*              Semaphore Count exception.                                    */
/* TryWait()  : Try to do a Wait() - if the count is positive, function as    */
/*              for Wait() and return true, otherwise return false to signify */
/*              that the wait failed and would require the process to block.  */
/* GetValue() : Return the current count value of the Semaphore.              */
/* Lock()     : Alternative name for Wait() method.                           */
/* Unlock()   : Alternative name for Post() method.                           */
/* TryLock()  : Alternative name for TryWait() method.                        */
/******************************************************************************/
class Semaphore
{
    public:
                                Semaphore(unsigned int uiInitialValue = 0);
                                ~Semaphore();

        void                    Wait();
        void                    Post();
        bool                    TryWait();

        int                     GetValue();

        void                    Lock()    { Wait(); };
        void                    Unlock()  { Post(); };
        bool                    TryLock() { return TryWait(); };

    private:
        sem_t                   semTheSemaphore;
};

/******************************************************************************/
/* class Condition.                                                           */
/*                                                                            */
/* This class encapsulates the basic Condition functionality into a C++ class.*/
/* The constructor is used to create the Condition and related MutEx.  The    */
/* destructor will destroy both the Condition and MutEx.  Member methods are: */
/*                                                                            */
/* Wait()        : Waits for the condition to be signalled or broadcast by    */
/*                 another thread.  Atomic operation - Unlocks mutexCondLock, */
/*                 blocks until the signal occurs, locks mutexCondLock.       */
/*                 The MutEx must be locked before calling Wait() and unlocked*/
/*                 afterwards.                                                */
/* Signal()      : Signals the condition, one of the threads waiting on the   */
/*                 condition will be unblocked, which one is unspecified.  If */
/*                 no threads are waiting, nothing happens.                   */
/* Broadcast()   : Signals the condition, all of the threads waiting on the   */
/*                 condition will be unblocked, which one is unspecified.  If */
/*                 no threads are waiting, nothing happens.                   */
/* LockMutEx()   : Locks the mutual exclusion associated with the condition.  */
/*                 Since MutEx is of type fastMutEx, lock will not fail and   */
/*                 so there is no return value.                               */
/* UnlockMutEx() : Unlocks the mutual exclusion associated with the condition.*/
/*                 Since MutEx is of type fastMutEx, unlock will not fail and */
/*                 so there is no return value.                               */
/******************************************************************************/
class Condition
{
    public:
                                Condition();
                                ~Condition();

        void                    Wait();
        void                    Signal();
        void                    Broadcast();

        void                    LockMutEx()   { mutexCondLock.Lock(); };
        void                    UnlockMutEx() { mutexCondLock.Unlock(); };

    private:
        pthread_cond_t          condTheCondition;
        MutualExclusion         mutexCondLock;
};

/******************************************************************************/
/* class ThreadBase.                                                          */
/*                                                                            */
/* This class encapsulates the basic pthread functionality into a C++ class.  */
/* The public method LaunchThread() is called to start executing the thread,  */
/* this will cause a seperate thread to run which executes the three virtual  */
/* private methods Initialise(), Execute() and CleanUp() in order. When these */
/* methods complete, the thread terminates.                                   */
/* It is expected that the programmer will inherit from this base class and   */
/* overload the three virtual functions to implement thread functionality as  */
/* required.                                                                  */
/* Other public member methods include ThreadRunning() which returns whether  */
/* or not the class thread is currently running and WaitThread(), which will  */
/* block until the thread has completed running.                              */
/* The destructor for the class will block until the thread has terminated,   */
/* this is done by calling WaitThread(), the SignalThreadDead() private member*/
/* method will reset the ThreadRunning flag and unblock any threads waiting   */
/* on the thread to finish.                                                   */
/******************************************************************************/
/* NOTES:                                                                     */
/*     - An inherited class must always call the base class constructor to    */
/*     ensure that the necessary member variables are correctly set.          */
/*     - Due to virtual destructor calls causing the inherited destructor to  */
/*     be called first and the virtual lookup table being destroyed, this     */
/*     means that virtual methods automatically default to the base class.    */
/*     In real terms, this means that if the destructor on an inherited class */
/*     is called before the thread is finished, then any vitual calls to      */
/*     Initialise(), Execute() and CleanUp() will no longer function.  If the */
/*     thread was in the middle of Execute(), then the base class CleanUp()   */
/*     will be called instead of the inherited CleanUp().  The only fix is to */
/*     call WaitThread() from within any inherited class destructor.          */
/******************************************************************************/
class ThreadBase
{
    public:
                                ThreadBase() : condThreadFinished() { bThreadRunning = false; }
        virtual                 ~ThreadBase() { WaitThread(); }

        void                    LaunchThread();

        bool                    ThreadRunning() { return bThreadRunning; }
        void                    WaitThread();

    protected:
        virtual void *          Initialise() { return NULL; }
        virtual void *          Execute() { return NULL; }
        virtual void            CleanUp() { }

    private:
        pthread_t               pthreadThreadDetails;
        Condition               condThreadFinished;
        bool                    bThreadRunning;

        static void *           ThreadMain(void *pvOwner);
        void                    SignalThreadDead();

};

#endif

/******************************************************************************/
/* End of File: pthreadcc.h                                                   */
/******************************************************************************/

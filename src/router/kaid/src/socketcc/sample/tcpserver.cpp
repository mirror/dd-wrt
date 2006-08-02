/******************************************************************************/
/* File: tcpserver.cpp                                                        */
/******************************************************************************/
/* This source code file contains implementations for the following classes:  */
/*  - TCPServerThread                                                         */
/*  - TCPServer                                                               */
/* Together these classes implement a TCP Server, the functionality of the    */
/* server is provided by inheriting a new class from TCPServerThread.         */
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2002-3                                */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.32 - First inclusion into sample code source base.            */
/******************************************************************************/

/******************************************************************************/
/* Standard C Includes.                                                       */
/******************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "tcpserver.h"

/******************************************************************************/
/* TCPServer Class.                                                           */
/******************************************************************************/
/* Constructor.                                                               */
/*                                                                            */
/* First we create the Listening Socket using the provided parameters.  Once  */
/* the listening socket has been created, we call StartServer() with the      */
/* provided parameters to launch the actual TCP Server.                       */
/******************************************************************************/
TCPServer::TCPServer(int iPortNumber, int iBacklog, int iMinSleepThreads,
                     int iMaxSleepThreads, int iThreadInc,
                     TCPServerThread *pcInitialThread, bool bUseIPv6)
{
    pcListeningSocket = new TCPServerSocket(iPortNumber, iBacklog, bUseIPv6);
    StartServer(pcInitialThread, iMinSleepThreads, iMaxSleepThreads, iThreadInc);
}

/******************************************************************************/
/* Constructor.                                                               */
/*                                                                            */
/* The second constructor allows specifying an IPAddress for the listening    */
/* socket to bind to.  The implementation is as for first constructor.        */
/******************************************************************************/
TCPServer::TCPServer(IPAddress &cServerAddress, int iPortNumber, int iBacklog,
                     int iMinSleepThreads, int iMaxSleepThreads, int iThreadInc,
                     TCPServerThread *pcInitialThread)
{
    pcListeningSocket = new TCPServerSocket(cServerAddress, iPortNumber, iBacklog);
    StartServer(pcInitialThread, iMinSleepThreads, iMaxSleepThreads, iThreadInc);
}

/******************************************************************************/
/* Destructor.                                                                */
/*                                                                            */
/* Terminates the TCP Server and the Garbage Collector Thread.  First all the */
/* TCP Server Threads are signalled to be deactivated.  The Listening Socket  */
/* is then destroyed, this will commence the chain reaction whereby the server*/
/* threads will commence to shutdown their execution.  Finally we set the     */
/* Garbage Collector terminate flag and signal the garbage collector thread.  */
/* This thread will then terminate when the Thread Index counter drops to     */
/* zero (when all registered ThreadBase classes are deleted).  The final two  */
/* operations are performed within the MuTex of the WakeThread condition.     */
/* Finally we wait for the thread to complete by calling WaitThread(), this   */
/* ensures that the class member variables are still accessible to the        */
/* garbage collector thread while it is terminating.                          */
/******************************************************************************/
TCPServer::~TCPServer()
{
    TCPServerThread::DeActivateServer();

    delete pcListeningSocket;

    condWakeThread.LockMutEx();
    bTerminate = true;
    condWakeThread.Signal();
    condWakeThread.UnlockMutEx();

    WaitThread();
}

/******************************************************************************/
/* void StartServer(TCPServerThread *pcInitialThread, int iMinSleepThreads,   */
/*                  int iMaxSleepThreads, int iThreadInc)                     */
/*                                                                            */
/* This method starts the TCPServer after the listening socket has been       */
/* created.  The bTerminate flag is set to false to allow the garbage         */
/* collector thread to run, iThreadCount is set to zero as no threads are     */
/* currently registered with the garbage collector and pcThreadToDelete is    */
/* set to NULL to signify that no thread has been scheduled for deletion.     */
/* We then call LaunchThread() to start the garbage collection thread before  */
/* setting the configuration parameters for the instances of TCPServerThread. */
/* Finally, if an TCPServerThread instance is provided, we launch the thread  */
/* causing the TCP Server to start servicing potential clients, if the        */
/* pointer is NULL, we create a default instance of TCPServerThread.          */
/******************************************************************************/
void
TCPServer::StartServer(TCPServerThread *pcInitialThread, int iMinSleepThreads,
                       int iMaxSleepThreads, int iThreadInc)
{
    bTerminate = false;
    iThreadCount = 0;
    pcThreadToDelete = NULL;

    LaunchThread();

    TCPServerThread::Configure(iMinSleepThreads, iMaxSleepThreads, iThreadInc,
                               pcListeningSocket, this);

    ((pcInitialThread == NULL)?(new TCPServerThread):(pcInitialThread))->LaunchThread();
}

/******************************************************************************/
/* void IncrementThreadCount()                                                */
/*                                                                            */
/* This method is called to increment the index count of the threads that are */
/* still to be deleted by the garbage collector thread.  The WakeUp Conditions*/
/* Mutual Exclusion is locked to protect access to the iThreadCount shared    */
/* variable, the count is incremented and the Mutual Exclusion is unlocked.   */
/******************************************************************************/
void
TCPServer::IncrementThreadCount()
{
    condWakeThread.LockMutEx();
    syslog(LOG_INFO, "Thread created - Active Threads(%d)", ++iThreadCount);
    condWakeThread.UnlockMutEx();
}

/******************************************************************************/
/* void RegisterThreadForDeletion(ThreadBase *pcOldThread)                    */
/*                                                                            */
/* This method is called to tell the garbage collector thread to delete the   */
/* provided ThreadBase instance.  First the WakeUp Conditions Mutual Exclusion*/
/* is locked, this ensures that we wait for any other ThreadBase deletion to  */
/* complete and for the Mutual Exclusion to be unlocked before proceeding.    */
/* Once the lock is acquired, pcThreadToDelete is set to point to the thread  */
/* to be deleted, the WakeUp condition is signalled and the Mutual Exclusion  */
/* is unlocked so that the garbage collector thread can wake up and reacquire */
/* the lock.                                                                  */
/******************************************************************************/
void
TCPServer::RegisterThreadForDeletion(ThreadBase *pcOldThread)
{
    condWakeThread.LockMutEx();
    while (pcThreadToDelete != NULL)
    {
        condWakeThread.UnlockMutEx();
        condWakeThread.LockMutEx();
    }
    pcThreadToDelete = pcOldThread;
    condWakeThread.Signal();
    condWakeThread.UnlockMutEx();
}

/******************************************************************************/
/* void *Execute()                                                            */
/*                                                                            */
/* This virtual member method is automatically executed as a seperate thread  */
/* within TCP Server.  The Garbage Collector Thread runs continuously until   */
/* both the terminate flag has been set and the thread index count is zero.   */
/* Within the loop we wait for the WakeUp Condition to be signalled.  When    */
/* signalled to wake up, the thread checks to see if any ThreadBase instance  */
/* has been registered for deletion.  If so, the class instance is deleted    */
/* (this automatically waits for the thread represented by the class to       */
/* actually terminate), sets pcThreadToDelete to NULL, and decrements the     */
/* index counter of the number of threads that remain to be deleted by the    */
/* Garbage Collector thread.  If the Garbage Collector has not been scheduled */
/* to terminate, or more threads remain to be terminated, then the loop       */
/* continues - otherwise the method ends and the secondary thread terminates. */
/******************************************************************************/
void *
TCPServer::Execute()
{
    condWakeThread.LockMutEx();
    while ((iThreadCount) || !(bTerminate))
    {
        condWakeThread.Wait();

        if (pcThreadToDelete != NULL)
        {
            delete pcThreadToDelete;
            pcThreadToDelete = NULL;
            syslog(LOG_INFO, "Thread deleted - Active Threads (%d)", --iThreadCount);
        }
    }
    condWakeThread.UnlockMutEx();
    return NULL;
}

/******************************************************************************/
/* TCPServerThread Class.                                                     */
/******************************************************************************/
/* Static member variables of TCPServerThread.                                */
/* mutexProtect:            Used to protect access to shared variables in the */
/*                          KillSurplusThreads() and CreateNewThreads()       */
/*                          methods.                                          */
/* mutexAccept:             Used to allow only one thread to call the socket  */
/*                          AcceptClient() method at a time, avoids the       */
/*                          thundering herd problem.                          */
/* iSleepingThreads:        A count of the number of threads currently blocked*/
/*                          waiting for a client to connect to the server.    */
/* iMinimumSleepingThreads: The minimum allowed number of threads waiting for */
/*                          a client to connect to the server.  If the number */
/*                          of sleeping threads falls below this value, new   */
/*                          threads have to be created.                       */
/* iMaximumSleepingThreads: The maximum allowed number of threads waiting for */
/*                          a client to connect to the server.  If the number */
/*                          of sleeping threads climbs above this value, then */
/*                          threads have to be destroyed.                     */
/* iNewThreadCount:         The number of new threads to create when the value*/
/*                          of iSleepingThreads falls below the threshold     */
/*                          value of iMinimumSleepingThreads.                 */
/* pidListeningThread:      The process ID of the thread that currently owns  */
/*                          the Accept mutual exclusion and is blocked while  */
/*                          waiting for a client to connect.  Used so deliver */
/*                          a SIGIO signal to the appropriate thread.         */
/* bServerActive:           A flag which is reset when de-activating the      */
/*                          server.                                           */
/* pcListeningSocket:       A pointer to the TCPServerSocket instance which   */
/*                          has been set-up to accept incomming socket calls. */
/* pcGarbageCollector:      A pointer to the owning TCPServer instance which  */
/*                          is also used to manage resource destruction when  */
/*                          threads terminate.                                */
/******************************************************************************/
MutualExclusion     TCPServerThread::mutexProtect;
MutualExclusion     TCPServerThread::mutexAccept;
int                 TCPServerThread::iSleepingThreads = 0;
int                 TCPServerThread::iMinimumSleepingThreads = 0;
int                 TCPServerThread::iMaximumSleepingThreads = 0;
int                 TCPServerThread::iNewThreadCount = 0;
bool                TCPServerThread::bServerActive = true;
TCPServerSocket     *TCPServerThread::pcListeningSocket = NULL;
TCPServer           *TCPServerThread::pcGarbageCollector = NULL;

/******************************************************************************/
/* void Configure(int iMinSleepingThreads, int iMaxSleepingThreads,           */
/*                int iNewThreadIncrement, TCPServerSocket *pcServerSocket,   */
/*                GarbageCollector *pcGarbage)                                */
/*                                                                            */
/* This method is used to configure the global parameters for all instances   */
/* of TCPServerThread, the static member variables are set to the provided    */
/* values.  This method must be called before any instance of TCPServerThread */
/* is created.                                                                */
/******************************************************************************/
void
TCPServerThread::Configure(int iMinSleepingThreads, int iMaxSleepingThreads,
                           int iNewThreadIncrement, TCPServerSocket *pcServerSocket,
                           TCPServer *pcOwner)
{
    iMinimumSleepingThreads = iMinSleepingThreads;
    iMaximumSleepingThreads = iMaxSleepingThreads;
    iNewThreadCount = iNewThreadIncrement;
    pcListeningSocket = pcServerSocket;
    pcGarbageCollector = pcOwner;
    bServerActive = true;
}

/******************************************************************************/
/* static void DeActivateServer()                                             */
/*                                                                            */
/* This method is called to deactivate the server, the ServerActive flag is   */
/* reset so no new connections are attempted, any threads entering this stage */
/* will detect the value of the ServerActive flag and kill themselves.        */
/******************************************************************************/
void
TCPServerThread::DeActivateServer()
{
    bServerActive = false;
}

/******************************************************************************/
/* void * Initialise()                                                        */
/*                                                                            */
/* To initialise the TCPServerThread, the thread index counter on the garbage */
/* collector is incremented, ensuring that the garbage collector thread will  */
/* wait for this thread to be terminated and deleted before terminating       */
/* itself.                                                                    */
/******************************************************************************/
void *
TCPServerThread::Initialise()
{
    pcGarbageCollector->IncrementThreadCount();
    return NULL;
}

/******************************************************************************/
/* void *Execute()                                                            */
/*                                                                            */
/* This virtual member method is automatically executed as a seperate thread  */
/* within TCPServerThread.  The TCPServerThread runs continuously until a     */
/* particular event occurs that causes the thread to die.  Within this loop   */
/* there are five basic tasks which occur in order, these tasks are:          */
/*                                                                            */
/* 1) Kill any surplus threads - the KillSurplusThreads() method is called.   */
/*    The current thread is sleeping (not servicing a client), if there are   */
/*    too many sleeping threads, the current thread must be terminated.  If   */
/*    this is the case, a TooManyThreads KillThreadException is thrown by the */
/*    KillSurplusThreads() method.  The KillSurplusThreads() method correctly */
/*    maintains the value of the SleepingThreads counter.                     */
/*                                                                            */
/* 2) Wait for a client connection - the AcceptClient() method is called.     */
/*    Block the thread until a connection is accepted to a client socket, a   */
/*    pointer to the newly created socket is returned.  If the server has     */
/*    been de-activated, the AcceptClient() method will fail and throw a      */
/*    ServerDown KillThreadAxception.  This can be caught to begin the thread */
/*    destruction procedure.                                                  */
/*                                                                            */
/* 3) Create any new threads - the CreateNewThreads() method is called.  The  */
/*    current thread has accepted a connection and is no longer sleeping, it  */
/*    is now servicing a client.  If there are not enough sleeping threads to */
/*    handle potential future clients, some new threads must be created.  The */
/*    CreateExtraThreads() method correctly maintains the value of the        */
/*    SleepingThreads counter.                                                */
/*                                                                            */
/* 4) Service the newly connected client - the ServiceClient() method is      */
/*    called.  This virtual method will handle any client/server exchanges    */
/*    that are required, the communications should take place on the socket   */
/*    passed as a parameter and will terminate when the client/server         */
/*    interaction is complete.  The method may throw one of two exceptions if */
/*    an error occurs.  A ClientError exception will cause the socket to be   */
/*    closed and the thread to loop and wait for the next connection to occur */
/*    while a FatalError exception will cause the thread to terminate itself  */
/*    due to a major error that cannot be rectified.                          */
/*                                                                            */
/* 5) Close the client socket - done by deleting the socket pointer returned  */
/*    by the AcceptClient() method.                                           */
/*                                                                            */
/* These tasks are performed within a try block that is itself within the     */
/* infinite loop.  Exceptions are caught and appropriate responses taken,     */
/* certain exceptions will cause the method to terminate and therefore begin  */
/* the termination of the thread.                                             */
/******************************************************************************/
void *
TCPServerThread::Execute()
{
    for (;;)
    {
        TCPSocket   *pcClientSocket = NULL;
        try
        {
            KillSurplusThreads();
            pcClientSocket = AcceptClient();
            CreateExtraThreads();
            ServiceClient(pcClientSocket);
            delete pcClientSocket;
        }
        catch (KillThreadExceptions &excep)
        {
            switch (excep)
            {
                case errFatalError:     delete pcClientSocket;
                case errServerDown:
                case errTooManyThreads: return NULL;
                case errClientError:    delete pcClientSocket;
            }
        }
    }
}

/******************************************************************************/
/* void CleanUp()                                                             */
/*                                                                            */
/* Before terminating the thread, a pointer to the class itself is registered */
/* with the garbage collector for deletion.  The garbage collector thread     */
/* delete this class as soon as the currently running thread is actually      */
/* terminated.                                                                */
/******************************************************************************/
void
TCPServerThread::CleanUp()
{
    pcGarbageCollector->RegisterThreadForDeletion(this);
}

/******************************************************************************/
/* void KillSurplusThreads()                                                  */
/*                                                                            */
/* This method is executed as a critical section, only one of the possibly    */
/* many instances of TCPServerThread can run the CreateExtraThreads() or the  */
/* KillSurplusThreads() method at a time.  This is done via the use of the    */
/* static member variable mutexProtect.  The purpose of this method is to     */
/* destroy the current TCPServerThread instance if the number of sleeping     */
/* threads - those waiting for a connection - gets too high.  This method is  */
/* called when the current thread has completed servicing a connected socket  */
/* and is ready to accept a new connection.  The first task is to unlock the  */
/* SleepingThreads semaphore, this will increment the count of the number of  */
/* sleeping threads including the current thread which is about to sleep.  We */
/* then check the value of the SleepingThreads semaphore - which indicates    */
/* the number of TCPServerThread instances that are waiting for a TCP socket  */
/* connection - to see if it is greater than the specified number of maximum  */
/* sleeping threads.  If this is the case, then the SleepingThreads semaphore */
/* is locked, incrementing the count of sleeping threads (the current thread  */
/* will no longer be sleeping as it is about to be destroyed).  Finally, the  */
/* mutual exclusion is unlocked and a TooManyThreads exception is thrown so   */
/* that the calling code can correctly terminate the thread execution.  If    */
/* the thread is not due to be destroyed then we simply unlock the mutual     */
/* exclusion, allowing other instances of TCPServerThread to enter their      */
/* critical sections.                                                         */
/******************************************************************************/
void
TCPServerThread::KillSurplusThreads()
{
    mutexProtect.Lock();
    iSleepingThreads++;
    if (iSleepingThreads > iMaximumSleepingThreads)
    {
        iSleepingThreads--;
        mutexProtect.Unlock();
        throw errTooManyThreads;
    }
    mutexProtect.Unlock();
}

/******************************************************************************/
/* void CreateExtraThreads()                                                  */
/*                                                                            */
/* This method is executed as a critical section, only one of the possibly    */
/* many instances of TCPServerThread can run the CreateExtraThreads() or the  */
/* KillSurplusThreads() method at a time.  This is done via the use of the    */
/* static member variable mutexProtect.  The purpose of this method is to     */
/* create more TCPServerThread instances if the number of sleeping threads -  */
/* those waiting for a connection - gets too low.  This method is called when */
/* the current thread has accepted a connection and is no longer waiting.     */
/* The first task is to decrement the SleepingThreads counter as the current  */
/* thread will now begin servicing a client and is no longer sleeping.  We    */
/* then check the value of the SleepingThreads counter - which indicates the  */
/* number of TCPServerThread instances that are waiting for a TCP socket      */
/* connection - to see if it is less than the specified number of minimum     */
/* sleeping threads.  If this is the case, then iNewThreadCount instances of  */
/* of TCPServerThread are created and launched, as each thread begins its     */
/* execution, the SleepingThreads semaphore will increment its value.  The    */
/* mutual exclusion is unlocked before the thread returns, allowing other     */
/* instances of TCPServerThread to enter their critical sections.             */
/******************************************************************************/
void
TCPServerThread::CreateExtraThreads()
{
    mutexProtect.Lock();
    iSleepingThreads--;
    if (iSleepingThreads < iMinimumSleepingThreads)
    {
        for (int iCount = 0; iCount < iNewThreadCount; iCount++)
        {
            CreateInstance();
        }
    }
    mutexProtect.Unlock();
}

/******************************************************************************/
/* TCPSocket *AcceptClient()                                                  */
/*                                                                            */
/* This method blocks until the next client connects to the server and returns*/
/* a pointer to a TCPSocket instance that can be used to communicate with the */
/* newly connected client.  This method is executed as a critical section but */
/* using a different Mutual Exclusion variable than the previous two methods  */
/* and can therefore run concurrently with the Kill or Create threads methods */
/* of other instances.  The primary reason for locking around the call to the */
/* socket AcceptClient() method is to avoid the thundering herd problem, only */
/* one thread is actively accepting a connection at a time.  Once the lock is */
/* acquired, if the static ServerActive flag is set we attempt to accept a    */
/* client connection.  We attempt to accept the connection, if successful, we */
/* release the lock on the mutual exclusion and return.  If it fails, a       */
/* SocketException is thrown, indicating a failed connection attempt.  If     */
/* this is the case, or the ServerActive flag is set, we release the Mutual   */
/* Exclusion lock and throw a ServerDown exception.  This will be caught in   */
/* the Execute() method which will then act to terminate the thread.          */
/******************************************************************************/
TCPSocket *
TCPServerThread::AcceptClient()
{
    mutexAccept.Lock();
    if (bServerActive)
    {
        try
        {
            TCPSocket   *pcClientSocket;

            pcClientSocket = pcListeningSocket->AcceptClient();
            mutexAccept.Unlock();
            return pcClientSocket;
        }
        catch (SocketException)
        {
        }
    }
    mutexAccept.Unlock();
    throw errServerDown;
}

/******************************************************************************/
/* void ServiceClient(TCPSocket *pcClientSocket)                              */
/*                                                                            */
/* This virtual member method is automatically called to service the client   */
/* once it has connected to the server.  The default implementation consists  */
/* of a TCP echo server.  The thread blocks until data is received from the   */
/* connected client socket, if the client closes the socket, this will return */
/* an exception.  We then print details about the received data and then echo */
/* the data back to the client.  This loops until the socket is closed.  If a */
/* socket error occurs, details are printed and the FatalError exception is   */
/* thrown so the calling method can correctly terminate.                      */
/******************************************************************************/
void
TCPServerThread::ServiceClient(TCPSocket *pcClientSocket)
{
    int                 iBytesTransferred;
    char                pcBuffer[65535];

    for(;;)
    {
        try
        {
            iBytesTransferred = pcClientSocket->RecvData(pcBuffer, 65535);

            pcBuffer[iBytesTransferred] = 0;

            syslog(LOG_INFO, "Received %d bytes [%s] from, %s:%d", iBytesTransferred,
                   pcBuffer, (pcClientSocket->RemoteIPAddress()).GetAddressString(),
                   pcClientSocket->RemotePortNumber());

            iBytesTransferred = pcClientSocket->SendData(pcBuffer, iBytesTransferred);
        }
        catch (SocketException &excep)
        {
            if (excep != SocketException::errNotConnected)
            {
                syslog(LOG_ERR, "Socket Exception : %s", (const char *) excep);
                throw errFatalError;
            }
            return;
        }
    }
}

/******************************************************************************/
/* End of File: tcpserver.cpp                                                 */
/******************************************************************************/

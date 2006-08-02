/******************************************************************************/
/* File: tcpserver.h                                                          */
/******************************************************************************/
/* This source code file is a header file for the following two classes:      */
/*  - TCPServerThread                                                         */
/*  - TCPServer                                                               */
/* Together these classes implement a TCP Server, the functionality of the    */
/* server is provided by inheriting a new class from TCPServerThread.  To run */
/* the server, create an instance of TCPServer with the necessary parameters  */
/* and a pointer to a newly created instance of TCPServerThread or inherited  */
/* class.  To stop the server, terminate the instance of TCPServer.  All      */
/* TCPServerThread or inherited class instances are automatically deleted.    */
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2002-3                                */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.32 - First inclusion into sample code source base.            */
/******************************************************************************/

/******************************************************************************/
/* Check to see if already included.                                          */
/******************************************************************************/
#ifndef TCPSERVER_H
#define TCPSERVER_H

/******************************************************************************/
/* Include pthreadCC and SocketCC Class Libraries.                            */
/******************************************************************************/
#include <pthreadcc.h>
#include <socketcc.h>

/******************************************************************************/
/* Forward definition of class TCPServerThread.                               */
/******************************************************************************/
class TCPServerThread;

/******************************************************************************/
/* class TCPServer.                                                           */
/*                                                                            */
/* This class implements a TCP Server and manages the garbage collection of   */
/* terminated TCP Server Threads.  Two seperate constructors are provided     */
/* whereby the server TCP Socket can be bound to a specific IP address or all */
/* IP addresses on the host.  The constructors create the listening socket,   */
/* start the execution of the garbage collector thread, and launch the first  */
/* TCPServerThread instance.  The destructor signals the multi-threaded       */
/* server to stop and blocks until all threads are properly destroyed and     */
/* resources released.  The class public methods include:                     */
/*                                                                            */
/* IncrementThreadCount():      Increments the value of ThreadCount, which    */
/*                              indicates the number of ThreadBase instances  */
/*                              that the Garbage Collector must delete before */
/*                              the Garbage Collector thread terminates.      */
/* RegisterThreadForDeletion(): Provides a pointer to a ThreadBase instance   */
/*                              that must be destroyed.  The seperate thread  */
/*                              operated within this class will perform a     */
/*                              delete operation on this value.  As for all   */
/*                              classes inherited from ThreadBase, the delete */
/*                              will block until the represented thread has   */
/*                              terminated.  Once the instance is deleted,    */
/*                              the ThreadCount value is decremented.         */
/******************************************************************************/
class TCPServer : public ThreadBase
{
    private:
        TCPServerSocket     *pcListeningSocket;

        Condition           condWakeThread;

        bool                bTerminate;
        int                 iThreadCount;
        ThreadBase          *pcThreadToDelete;

        void                StartServer(TCPServerThread *pcInitialThread,
                                        int iMinSleepThreads, int iMaxSleepThreads,
                                        int iThreadInc);

        virtual void *      Execute();

    public:
                            TCPServer(int iPortNumber, int iBackLog,
                                      int iMinSleepThreads, int iMaxSleepThreads,
                                      int iThreadInc, TCPServerThread *pcInitialThread = NULL,
                                      bool bUseIPv6 = false);
                            TCPServer(IPAddress &cServerAddress, int iPortNumber,
                                      int iBacklog, int iMinSleepThreads,
                                      int iMaxSleepThreads, int iThreadInc,
                                      TCPServerThread *pcInitialThread = NULL);
                            ~TCPServer();

        void                IncrementThreadCount();
        void                RegisterThreadForDeletion(ThreadBase *pcOldThread);
};

/******************************************************************************/
/* class TCPServerThread.                                                     */
/*                                                                            */
/* This class is also inherited from the ThreadBase class and implements a    */
/* a single thread servicing a single client in a TCP Server.  The thread will*/
/* loop waiting for a client to connect to the server and then servicing the  */
/* client.  The scalable approach allows for the creation of new instances of */
/* TCPServerThread should the number of threads be too low to handle any new  */
/* incomming connections as well as the registration for deletion of threads  */
/* by Garbage Collection should there be too many idle threads.  This class   */
/* provides the following public methods:                                     */
/*                                                                            */
/* Constructor:        A set and forget constructor, once a single instance   */
/*                     is created and the thread launched, the thread will    */
/*                     create any further instances as required.  Class       */
/*                     instance are also automatically deleted when the       */
/*                     thread terminates.                                     */
/* Destructor:         Wait for the thread to terminate, automatically called */
/*                     by the Garbage Collector.  Does not need to be called  */
/*                     via an application.                                    */
/* Configure():        Static Method - call once to specify parameters that   */
/*                     are required to support the TCP Service.               */
/* DeActivateServer(): Static Method - signal the TCP server to stop.  Will   */
/*                     set all active TCPServerThread instances on a path     */
/*                     that will eventually cause all of them to terminate.   */
/*                                                                            */
/* The class can be inherited to provide specific support for a given TCP     */
/* Server application.  This is done by overloading the virtual member method */
/* ServiceClient().  This method can be re-written to provide the required    */
/* functonality.  The default implementation implements an Echo server.  The  */
/* CreateInstance() virtual method also needs to be overloaded to allow the   */
/* automatic creation of other classes of the correct type.                   */
/******************************************************************************/
class TCPServerThread : public ThreadBase
{
    protected:
        static MutualExclusion      mutexProtect, mutexAccept;

        static int                  iSleepingThreads, iMinimumSleepingThreads;
        static int                  iMaximumSleepingThreads, iNewThreadCount;

        static bool                 bServerActive;
        static TCPServerSocket      *pcListeningSocket;
        static TCPServer            *pcGarbageCollector;

        enum KillThreadExceptions { errFatalError, errTooManyThreads, errClientError, errServerDown };

        virtual void *      Initialise();
        virtual void *      Execute();
        virtual void        CleanUp();

        void                KillSurplusThreads();
        void                CreateExtraThreads();
        TCPSocket *         AcceptClient();
        virtual void        ServiceClient(TCPSocket *pcClientSocket);

    public:
                            TCPServerThread() : ThreadBase() {}
        virtual             ~TCPServerThread() {}

        virtual void        CreateInstance() { (new TCPServerThread)->LaunchThread(); }

        static void         Configure(int iMinSleepingThreads, int iMaxSleepingThreads,
                                      int iNewThreadIncrement, TCPServerSocket *pcServerSocket,
                                      TCPServer *pcOwner);
        static void         DeActivateServer();
};

#endif

/******************************************************************************/
/* End of File: tcpserver.h                                                   */
/******************************************************************************/

/******************************************************************************/
/* File: deamon.h                                                             */
/******************************************************************************/
/* This header file contains the definition of the Deamon class. This class   */
/* implements the actual Deamon application by reading the configuration file,*/
/* starting and stopping the server, and handling system signals.             */
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
#ifndef DEAMON_H
#define DEAMON_H

/******************************************************************************/
/* Include definition for TCPServer and Condition classes.                    */
/******************************************************************************/
#include "tcpserver.h"
#include <pthreadcc.h>

/******************************************************************************/
/* class Deamon.                                                              */
/*                                                                            */
/* This class is implements the functionality of the Deamon application.  It  */
/* runs the deamon until a SIGINT or SIGTERM signal is intercepted.  The      */
/* deamon is stopped and restarted when a SIGHUP signal is intercepted.  The  */
/* class reads and parses a configuration file to determine the operating     */
/* parameters for the deamon.  The public interface consists of a constructor,*/
/* a destructor and a single method which runs the daemon.  A description of  */
/* these methods is listed below:                                             */
/*                                                                            */
/* Constructor : Creates the Deamon instance, requires a string filename of   */
/*               the configuration file to use.  The contents of this file    */
/*               can be edited and STGen restarted by sending a SIGHUP signal.*/
/*               This will cause the config file to be re-read when STGen is  */
/*               restarted.  The constructor always succeeds and throws no    */
/*               exceptions.                                                  */
/* Destructor  : Frees any resources allocated in the constructor.            */
/* RunDaemon() : Runs the daemon until a SIGINT or SIGTERM signal is received.*/
/*               The deamon is then terminated and the method call returns.   */
/*               If an error occurs during deamon startup (or on restart in   */
/*               the event of a SIGHUP), a (char *) type exception is thrown, */
/*               and the deamon is no longer running.  The string contains a  */
/*               description of the error which can be logged.                */
/******************************************************************************/
class Deamon
{
    private:
        enum ParseError { errNotComment, errNotInteger, errInvalidParam, errInvalidValue };

        static bool        bKillFlag;
        static bool        bRestartFlag;
        static Condition   condSignal;

        static void SignalHandler(int iSig);

        char        *pcConfigFileName, pcException[1024];
        TCPServer   *pcTheServer;

        bool        ParseLine(char *pcConfigLine, char *pcFieldName, int &iSetting);
        void        ParseConfigFile(int &iServerPortNumber, int &iMinSleepingThreads,
                                    int &iMaxSleepingThreads, bool &bUseIPv6);
        void        Start();
        void        Stop();

    public:
        enum DaemonError { errAbnormalTermination };

                    Deamon(const char *pcConfigFile);
                    ~Deamon();

        void        RunDeamon();
};

#endif

/******************************************************************************/
/* End of File: deamon.h                                                      */
/******************************************************************************/

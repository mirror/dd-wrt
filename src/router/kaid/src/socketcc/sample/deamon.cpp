/******************************************************************************/
/* File: deamon.cpp                                                           */
/******************************************************************************/
/* This source code file contains the implementation of the Deamon class. This*/
/* class implements the actual Deamon application by reading the configuration*/
/* file, starting and stopping the server, and handling system signals.       */
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <syslog.h>

/******************************************************************************/
/* Include socket and pthread library to catch and print exceptions.          */
/******************************************************************************/
#include <socketcc.h>
#include <pthreadcc.h>

/******************************************************************************/
/* Include header files for TCPServer and TCPServerThread classes.            */
/******************************************************************************/
#include "tcpserver.h"

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "deamon.h"

/******************************************************************************/
/* Default values for configuration items.                                    */
/******************************************************************************/
#define PORT_DEFAULT            0
#define MIN_THREADS_DEFAULT     5
#define MAX_THREADS_DEFAULT     10
#define IPV6_DEFAULT            true

/******************************************************************************/
/* Static Variables.                                                          */
/*                                                                            */
/* There is only one instance of STGen, static variables are accessed in the  */
/* signal handler which must be a static method.                              */
/******************************************************************************/
bool        Deamon::bKillFlag = false;
bool        Deamon::bRestartFlag = false;
Condition   Deamon::condSignal;

/******************************************************************************/
/* Private Methods.                                                           */
/******************************************************************************/
/* void SignalHandler(int iSig)                                               */
/*                                                                            */
/* This static method is automatically called when the following signals are  */
/* sent to the ST-Gen application.  We signal the condition variable so that  */
/* primary thread will wake up and handle the signal.                         */
/*                                                                            */
/* Signals Handled:                                                           */
/*    SIGTERM - Terminate ST-Gen, sets bKillFlag.                             */
/*    SIGINT  - Same as SIGTERM.                                              */
/*    SIGHUP  - Restart ST-Gen, sets bRestart.                                */
/******************************************************************************/
void
Deamon::SignalHandler(int iSig)
{
    condSignal.LockMutEx();
    switch (iSig)
    {
        case SIGINT:
        case SIGTERM:   bKillFlag = true; break;
        case SIGHUP:    bRestartFlag = true; break;
    }
    condSignal.UnlockMutEx();
    condSignal.Signal();
}

/******************************************************************************/
/* bool ParseLine(char *pcConfigLine, char *pcFieldName, int &iSetting)       */
/*                                                                            */
/* This method takes as input a string which represents one line from the     */
/* configuration file and processes it.  If the string represents a valid     */
/* setting, then the pcFieldName array is filled with the name of the field   */
/* to be set, iSetting is the integer value to be assigned to that field, and */
/* the function returns true.  If the input string is otherwise valid, it     */
/* contains a comment or a single field name (specifying a default value),    */
/* then the function returns false to indicate that nothing should be done as */
/* a result of processing this input line.  If there is an error in the input */
/* string an exception is thrown.  The possible errors are:                   */
/*  * Second field of string is not a valid integer - Indicates a non-valid   */
/*    value for the setting.                                                  */
/*  * Third field is not a valid comment - Indicates an illegal entry after   */
/*    the field setting.                                                      */
/******************************************************************************/
bool
Deamon::ParseLine(char *pcConfigLine, char *pcFieldName, int &iSetting)
{
    char    *pcString1, *pcString2, *pcString3, *pcEndIntConv;

    if ((pcConfigLine[0] !='#') && (pcConfigLine[0] != '\0'))
    {
        pcString1 = pcConfigLine + strspn(pcConfigLine, " \t");
        pcString2 = pcString1 + strcspn(pcString1, " \t");
        pcString2+= strspn(pcString2, " \t");
        pcString3 = pcString2 + strcspn(pcString2, " \t");
        pcString3+= strspn(pcString3, " \t");

        iSetting = strtol(pcString2, &pcEndIntConv, 10);
        if ((!(isspace(pcEndIntConv[0]))) && (pcEndIntConv[0] != '\0'))
        {
            throw errNotInteger;
        }

        if (pcEndIntConv == pcString2) return false;

        if ((pcString3[0] !='#') && (pcString3[0] != '\0'))
        {
            throw errNotComment;
        }

        strncpy(pcFieldName, pcString1, strcspn(pcString1, " \t"));
        return true;
    }
    return false;
}

/******************************************************************************/
/* void ParseConfigFile(int &iServerPortNumber, int &iMinSleepingThreads,     */
/*                      int &iMaxSleepingThreads, bool &bUseIPv6)             */
/*                                                                            */
/* This function will open and parse the contents of the configuration file   */
/* and assign the configuration values to the four parameters.  A string      */
/* exception is thrown if there is an error reading the file.  We parse the   */
/* file one line at a time, ignoring empty or commented lines using the       */
/* ParseLine() function.  If an error is found, a ParseError is thrown, this  */
/* is then caught and converted to a string description of the error which is */
/* then thrown.                                                               */
/******************************************************************************/
void
Deamon::ParseConfigFile(int &iServerPortNumber, int &iMinSleepingThreads, int &iMaxSleepingThreads, bool &bUseIPv6)
{
    FILE    *pfdConfig;
    char    pcConfigLine[2048], pcFieldName[15];
    int     iLineNumber, iSetting;

    if ((pfdConfig = fopen(pcConfigFileName, "r")) == NULL)
    {
        syslog(LOG_ERR, "ERROR: Unable to open configuration file (%s)", pcConfigFileName);
        throw errAbnormalTermination;
    }

    iLineNumber = 1;

    try
    {
        while (fscanf(pfdConfig, "%[^\n]\n", pcConfigLine) != EOF)
        {
            if (ParseLine(pcConfigLine, pcFieldName, iSetting))
            {
                if (strstr(pcFieldName, "Port") == pcFieldName)
                {
                    iServerPortNumber = iSetting;
                } else if (strstr(pcFieldName, "MinimumThreads") == pcFieldName)
                {
                    iMinSleepingThreads = iSetting;
                } else if (strstr(pcFieldName, "MaximumThreads") == pcFieldName)
                {
                    iMaxSleepingThreads = iSetting;
                } else if (strstr(pcFieldName, "Protocol") == pcFieldName)
                {
                    switch (iSetting)
                    {
                        case 4  : bUseIPv6 = false;
                                  break;
                        case 6  : bUseIPv6 = true;
                                  break;
                        default : throw errInvalidValue;
                    }
                } else
                {
                    throw errInvalidParam;
                }

            }
            iLineNumber++;
        }
        fclose(pfdConfig);
    }
    catch (ParseError &excep)
    {
        fclose(pfdConfig);
        sprintf(pcException, "ERROR: Processing configuration file in line %d (%s) - ",
                iLineNumber, pcConfigLine);
        switch (excep)
        {
            case errNotComment:     strcat(pcException, "Text following field assignment is not a comment");
                                    break;
            case errNotInteger:     strcat(pcException, "Value for field must be an Integer");
                                    break;
            case errInvalidParam:   strcat(pcException, "Invalid field name, valid fields are [Port | MinimumThreads | MaximumThreads | Protocol]");
                                    break;
            case errInvalidValue:   strcat(pcException, "Value of Protocol field must be either 4 or 6");
                                    break;
            default:                strcat(pcException, "Unknown error");
                                    break;
        }
        syslog(LOG_ERR, pcException);
        throw errAbnormalTermination;
    }
}

/******************************************************************************/
/* void Start()                                                               */
/*                                                                            */
/* This function is called to start the deamon.  It begins by setting the     */
/* default configuration values before calling ParseConfigFile() to read and  */
/* set the proper configuration values.  We then create an instance of        */
/* TCPServer with the specified information.  If either ParseConfigFile() or  */
/* the TCPServer constructor fails, an exception is thrown.                   */
/******************************************************************************/
void
Deamon::Start()
{
    int         iServerPortNumber = PORT_DEFAULT;
    int         iMinSleepingThreads = MIN_THREADS_DEFAULT;
    int         iMaxSleepingThreads = MAX_THREADS_DEFAULT;
    bool        bUseIPv6 = IPV6_DEFAULT;

    try
    {
        ParseConfigFile(iServerPortNumber, iMinSleepingThreads, iMaxSleepingThreads, bUseIPv6);

        syslog(LOG_INFO, "Starting the Daemon");
        syslog(LOG_INFO, "Listening on Port %d using IPv%d", iServerPortNumber, (bUseIPv6)?6:4);
        syslog(LOG_INFO, "Minimum sleeping thread pool : %d", iMinSleepingThreads);
        syslog(LOG_INFO, "Maximum sleeping thread pool : %d", iMaxSleepingThreads);

        pcTheServer = new TCPServer(iServerPortNumber, 5, iMinSleepingThreads,
                                    iMaxSleepingThreads,
                                    ((iMaxSleepingThreads - iMinSleepingThreads) >> 1),
                                    new TCPServerThread, bUseIPv6);

        syslog(LOG_INFO, "Daemon Started");
    }
    catch (SocketException &excep)
    {
        syslog(LOG_ERR, "ERROR: Socket Exception (%s) while launching daemon", (const char *) excep);
        throw errAbnormalTermination;
    }
    catch (ThreadException &excep)
    {
        syslog(LOG_ERR, "ERROR: Thread Exception (%s) while launching daemon", (const char *) excep);
        throw errAbnormalTermination;
    }
}

/******************************************************************************/
/* void Stop()                                                                */
/*                                                                            */
/* This method stops the deamon, we delete the passed instance of TCPServer   */
/* that refers to the currently running server.                               */
/******************************************************************************/
void
Deamon::Stop()
{
    delete pcTheServer;
    syslog(LOG_INFO, "Daemon Terminated");
}

/******************************************************************************/
/* Public Methods.                                                            */
/******************************************************************************/
/* Constructor.                                                               */
/*                                                                            */
/* Makes a copy of the supplied configuration file name, sets the pointer to  */
/* TCPServer instance to NULL, installs the signal handler for SIGHUP, SIGINT */
/* and SIGTERM                                                                */
/******************************************************************************/
Deamon::Deamon(const char *pcConfigFile)
{
    pcConfigFileName = strdup(pcConfigFile);
    pcTheServer = NULL;

    signal(SIGINT, &SignalHandler);
    signal(SIGTERM, &SignalHandler);
    signal(SIGHUP, &SignalHandler);
}

/******************************************************************************/
/* Destructor.                                                                */
/*                                                                            */
/* Frees the memory allocated to store the string representing the config.    */
/* file name.                                                                 */
/******************************************************************************/
Deamon::~Deamon()
{
    free(pcConfigFileName);
}

/******************************************************************************/
/* void RunDaemon()                                                           */
/*                                                                            */
/* This method runs the ST-Gen Daemon.  We call Start() to start the deamon.  */
/* We then start a loop waiting for an SIGINT or SIGTERM signal, this will    */
/* set bKillFlag which terminates the loop and finally calls Stop() to stop   */
/* the deamon.  If we receive SIGHUP, bRestartFlag is set so we stop the      */
/* server, start it again and reset the flag.  The method returns when the    */
/* deamon is terminated (via SIGINT or SIGTERM) or if an exception is thrown  */
/* by Start().                                                                */
/******************************************************************************/
void
Deamon::RunDeamon()
{
    Start();

    condSignal.LockMutEx();
    while (!bKillFlag)
    {
        condSignal.Wait();
        if (bRestartFlag)
        {
            Stop();
            Start();
            bRestartFlag = false;
        }
    }
    condSignal.UnlockMutEx();

    Stop();
}

/******************************************************************************/
/* End of File: deamon.cpp                                                     */
/******************************************************************************/

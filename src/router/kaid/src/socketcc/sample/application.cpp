/******************************************************************************/
/* File: application.cpp                                                      */
/******************************************************************************/
/* This source code file is the source file which implements the deamon       */
/* sample application.  Code in this file parses the command line and creates */
/* the Deamon class instance which runs the server.                           */
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
#include <syslog.h>

/******************************************************************************/
/* Include header files for TCPServer and StreamerThread classes.             */
/******************************************************************************/
#include "deamon.h"

/******************************************************************************/
/* Default value for the filename containing the configuration information.   */
/******************************************************************************/
#define CONFIG_FILE             "deamon.config"

/******************************************************************************/
/* int main(int iArgC, char *ppcArgV[])                                       */
/*                                                                            */
/* Deamon server launching program.  After printing some information, we try  */
/* to create an instance of Deamon, if a command line parameter was specified */
/* this is used as the configuration filename otherwise we use the default    */
/* value defined in CONFIG_FILE.  For more than 1 command line argument, we   */
/* abort with a usage error message.  We then try to run the daemon inside a  */
/* a try block, if any errors occur a string exception will be thrown which   */
/* is then printed before the application terminates.                         */
/******************************************************************************/
int
main(int iArgC, char *ppcArgV[])
{
    openlog("my_deamon", LOG_PID, LOG_DAEMON);

    printf("===================================\n");
    printf("          My Echo Deamon.\n");
    printf("===================================\n");
    printf(" Version      : 1.32\n");
    printf(" Developed By : Jason But\n");
    printf(" Copyright    : 2001-2003\n");
    printf("===================================\n");

    Deamon       cDeamon((iArgC == 1)?CONFIG_FILE:(ppcArgV[1]));

    try
    {
        if (iArgC > 2)
        {
            printf("Error: Illegal number of arguments - Usage:\n\tdeamon [config_file]\n");
            printf("===================================\n");
            throw 1;
        }

        cDeamon.RunDeamon();
    }
    catch (...)
    {
        syslog(LOG_CRIT, "ERROR: Abnormal Daemon Termination");
    }
	closelog();
}

/******************************************************************************/
/* End of File: application.cpp                                               */
/******************************************************************************/

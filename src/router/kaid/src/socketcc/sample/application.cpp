#include <stdio.h>
#include <syslog.h>

/******************************************************************************/
/* Include header files for TCPServer and StreamerThread classes.             */
/******************************************************************************/
#include "daemon.h"

/******************************************************************************/
/* Default value for the filename containing the configuration information.   */
/******************************************************************************/
#define CONFIG_FILE             "kaid.conf"

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
    openlog("kaid", LOG_PID, LOG_DAEMON);

    printf("===================================\n");
    printf("          Kai Daemon v0.1\n");
    printf("===================================\n");

    Deamon       cDeamon((iArgC == 1)?CONFIG_FILE:(ppcArgV[1]));

    try
    {
        if (iArgC > 2)
        {
            printf("Error: Illegal number of arguments - Usage:\n\t%s [config_file]\n", ppcArgV[0]);
            printf("===================================\n");
            throw 1;
        }

        cDeamon.RunDeamon();
    }
    catch (...)
    {
        syslog(LOG_CRIT, "ERROR: Abnormal Kai Daemon Termination");
    }
	closelog();
}

/******************************************************************************/
/* End of File: application.cpp                                               */
/******************************************************************************/

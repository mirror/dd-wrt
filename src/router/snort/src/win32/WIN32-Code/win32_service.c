/* $Id$ */
/*
** Copyright (C) 2002 Chris Reid <chris.reid@codecraftconsulants.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 * win32_service.c v1.0 - 20 February 2002
 *
 * Purpose: Lets Snort register as a Win32 Service.  This includes both
 *          an installation an uninstallation aspect.
 *
 * Author:  Chris Reid (chris.reid@codecraftconsulants.com)
 *
 * Notes:   The Snort command-line arguments need to be
 *          saved into the registry when the snort service is
 *          being installed.  They are stored in:
 *              HKLM \ SOFTWARE \ Snort
 *
 * Usage:
 *          snort.exe /SERVICE /INSTALL [regular command-line params]
 *
 *          snort.exe /SERVICE /UNINSTALL
 *
 *          snort.exe /SERVICE /SHOW
 *
 * References
 *          Microsoft has full docs on programming Win32 Services in their
 *          MSDN (Microsoft Developer Network) library.
 *          http://msdn.microsoft.com/
 */

#ifdef ENABLE_WIN32_SERVICE

/*
 * Enable the next line to automatically assign a description to the Service.
 * According to the Microsoft documentation, the call to ChangeServiceConfig2()
 * which sets the description is only available on Windows 2000 or newer.
 *
 *  #define SET_SERVICE_DESCRIPTION
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Windows.h>
#include <Winsvc.h>  /* for Service stuff */
#include <stdio.h>   /* for printf(), etc */
#include <direct.h>  /* for _getcwd()     */

#include <pcap.h>

#include "snort.h"
#include "snort_debug.h"
#include "util.h"

static LPTSTR g_lpszServiceName        = "SnortSvc";
static LPTSTR g_lpszServiceDisplayName = "Snort";
static LPTSTR g_lpszServiceDescription = "The Open Source Network Intrusion Detection System";

static LPTSTR g_lpszRegistryKey        = "SOFTWARE\\Snort";
static LPTSTR g_lpszRegistryCmdFormat  = "CmdLineParam_%03d";
static LPTSTR g_lpszRegistryCountFormat= "CmdLineParamCount";

static SERVICE_STATUS          g_SnortServiceStatus;
static SERVICE_STATUS_HANDLE   g_SnortServiceStatusHandle;

#define MAX_REGISTRY_KEY_LENGTH   255
#define MAX_REGISTRY_DATA_LENGTH  1000


static VOID  SvcDebugOut(LPSTR String, DWORD Status);
static VOID  SvcFormatMessage(LPSTR szString, int iCount);
static VOID  ReadServiceCommandLineParams( int * piArgCounter, char** * pargvDynamic );
static VOID  WINAPI SnortServiceStart (DWORD argc, LPTSTR *argv);
static VOID  WINAPI SnortServiceCtrlHandler (DWORD opcode);
static DWORD SnortServiceInitialization (DWORD argc, LPTSTR *argv, DWORD *specificError);
static VOID  InstallSnortService(int argc, char* argv[]);
static VOID  UninstallSnortService();
static VOID  ShowSnortServiceParams();




/*******************************************************************************
 * (This documentation was taken from Microsoft's own doc's on how to create
 * a Win32 Service.)
 *
 * Writing a Service Program's main Function
 * -----------------------------------------------------------------------------
 *
 * The main function of a service program calls the StartServiceCtrlDispatcher
 * function to connect to the SCM and start the control dispatcher thread. The
 * dispatcher thread loops, waiting for incoming control requests for the
 * services specified in the dispatch table. This thread does not return until
 * there is an error or all of the services in the process have terminated. When
 * all services in a process have terminated, the SCM sends a control request
 * to the dispatcher thread telling it to shut down. The thread can then return
 * from the StartServiceCtrlDispatcher call and the process can terminate.
 *
 * The following example is a service process that supports only one service. It
 * takes two parameters: a string that can contain one formatted output
 * character and a numeric value to be used as the formatted character. The
 * SvcDebugOut function prints informational messages and errors to the debugger.
 * For information on writing the SnortServiceStart and SnortServiceInitialization
 * functions, see Writing a ServiceMain Function. For information on writing the
 * SnortServiceCtrlHandler function, see Writing a Control Handler Function.
 *******************************************************************************/


/* this is the entry point which is called from main() */
int SnortServiceMain(int argc, char* argv[])
{
    int i;
    /*
    SERVICE_TABLE_ENTRY   steDispatchTable[] =
    {
        { g_lpszServiceName, SnortServiceStart },
        { NULL,       NULL                     }
    };
    */

    SERVICE_TABLE_ENTRY   steDispatchTable[2];

    steDispatchTable[0].lpServiceName = g_lpszServiceName;
    steDispatchTable[0].lpServiceProc = SnortServiceStart;
    steDispatchTable[1].lpServiceName = NULL;
    steDispatchTable[1].lpServiceProc = NULL;

    for( i=1; i<argc; i++ )
    {
        if( _stricmp(argv[i],SERVICE_CMDLINE_PARAM) == 0)
        {
            /* Ignore param, because we already know that this is a service
             * simply by the fact that we are already in this function.
             * However, perform a sanity check to ensure that the user
             * didn't just type "snort /SERVICE" without an indicator
             * following.
             */

            if( (i+1) < argc &&
                ( _stricmp(argv[(i+1)], SERVICE_INSTALL_CMDLINE_PARAM)!=0   ||
                  _stricmp(argv[(i+1)], SERVICE_UNINSTALL_CMDLINE_PARAM)!=0 ||
                  _stricmp(argv[(i+1)], SERVICE_SHOW_CMDLINE_PARAM)!=0       ) )
            {
                /* user entered correct command-line parameters, keep looping */
                continue;
            }
        }
        else if( _stricmp(argv[i],SERVICE_INSTALL_CMDLINE_PARAM) == 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, "User wishes to install the Snort service\n"););
            InstallSnortService(argc, argv);
            exit(0);
        }
        else if( _stricmp(argv[i],SERVICE_UNINSTALL_CMDLINE_PARAM) == 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, "User wishes to un-install the Snort service\n"););
            UninstallSnortService();
            exit(0);
        }
        else if( _stricmp(argv[i],SERVICE_SHOW_CMDLINE_PARAM) == 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, "User wishes to show the Snort service command-line parameters\n"););
            ShowSnortServiceParams();
            exit(0);
        }
        else
        {
            break;  /* out of for() */
        }
    }

    /* If we got to this point, then it's time to start up the Win32 Service */
    if (!StartServiceCtrlDispatcher(steDispatchTable))
    {
        char szString[1024];
        memset(szString, sizeof(szString), '\0');
        SvcFormatMessage(szString, sizeof(szString));

        SvcDebugOut(szString, 0);
        SvcDebugOut(" [SNORT_SERVICE] StartServiceCtrlDispatcher error = %d\n", GetLastError());
        FatalError (" [SNORT_SERVICE] StartServiceCtrlDispatcher error = %d\n%s\n", GetLastError(), szString);
    }

    return(0);
}

VOID SvcDebugOut(LPSTR szString, DWORD dwStatus)
{
    CHAR  szBuffer[1024];
    if (strlen(szString) < 1000)
    {
        sprintf(szBuffer, szString, dwStatus);
        OutputDebugStringA(szBuffer);
    }
}

/* Copy the system error message into the buffer provided.
 * The buffer length is indicated in iCount.
 */
VOID SvcFormatMessage(LPSTR szString, int iCount)
{
    LPVOID lpMsgBuf;
    if( szString!=NULL && iCount>0)
    {
        memset(szString, 0, iCount);
        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       GetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
                       (LPTSTR) &lpMsgBuf,
                       0,
                       NULL
                     );

        strncpy(szString, (LPCTSTR) lpMsgBuf, iCount-1);

        szString[iCount-1]=0;

        /* Free the buffer. */
        LocalFree( lpMsgBuf );
        lpMsgBuf = NULL;
    }
}


VOID ReadServiceCommandLineParams( int * piArgCounter, char** * pargvDynamic )
{
    HKEY  hkSnort = NULL;
    long  lRegRC = 0;
    DWORD dwType;
    DWORD dwDataSize;
    BYTE  byData[MAX_REGISTRY_DATA_LENGTH];
    int   i;

    /**********
     * Read the registry entries for Snort command line parameters
     **********/
    lRegRC = RegOpenKeyEx( HKEY_LOCAL_MACHINE,        /* handle to open key      */
                           g_lpszRegistryKey,         /* subkey name             */
                           0,                         /* reserved (must be zero) */
                           KEY_READ,                  /* desired security access */
                           &hkSnort                   /* key handle              */
                         );
    if( lRegRC != ERROR_SUCCESS )
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        FatalError(" [SNORT_SERVICE] Unable to open Snort registry entry. "
                   " Perhaps Snort has not been installed as a service."
                   " %s", szMsg);
    }

    memset(byData, 0, sizeof(byData));
    dwDataSize = sizeof(byData);
    lRegRC = RegQueryValueEx( hkSnort,                      /* handle to key       */
                              g_lpszRegistryCountFormat,    /* value name          */
                              NULL,                         /* reserved            */
                              &dwType,                      /* type buffer         */
                              byData,                       /* data buffer         */
                              &dwDataSize                   /* size of data buffer */
                            );
    if( lRegRC != ERROR_SUCCESS )
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        FatalError(" [SNORT_SERVICE] Unable to read Snort registry entry '%s'."
                   " Perhaps Snort has not been installed as a service."
                   " %s", g_lpszRegistryCountFormat, szMsg);
    }

    (*piArgCounter) = * ((int*)&byData);

    (*pargvDynamic) = SnortAlloc( ((*piArgCounter) + 2) * sizeof(char *) );
    (*pargvDynamic)[0] = SnortStrdup(g_lpszServiceName);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Preparing to use the following command-line arguments:\n"););

    for( i=1; i<=(*piArgCounter); i++ )
    {
        TCHAR szName[MAX_REGISTRY_KEY_LENGTH];
        sprintf(szName, g_lpszRegistryCmdFormat, i);
        memset(byData, 0, sizeof(byData));
        dwDataSize = sizeof(byData);
        lRegRC = RegQueryValueEx( hkSnort,            /* handle to key       */
                                  szName,             /* value name          */
                                  NULL,               /* reserved            */
                                  &dwType,            /* type buffer         */
                                  byData,             /* data buffer         */
                                  &dwDataSize         /* size of data buffer */
                                );
        if( lRegRC != ERROR_SUCCESS )
        {
            TCHAR szMsg[1000];
            SvcFormatMessage(szMsg, sizeof(szMsg));
            FatalError(" [SNORT_SERVICE] Unable to read Snort registry entry '%s'."
                       " Perhaps Snort has not been installed as a service."
                       " %s", szName, szMsg);
        }

        (*pargvDynamic)[i] = SnortStrdup( (char*) byData );
        DEBUG_WRAP(DebugMessage(DEBUG_INIT, "  %s\n", (*pargvDynamic)[i]););
    }
    lRegRC = RegCloseKey( hkSnort );
    if( lRegRC != ERROR_SUCCESS )
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        FatalError(" [SNORT_SERVICE] Unable to close Snort registry entry."
                   " Perhaps Snort has not been installed as a service."
                   " %s", szMsg);
    }
    hkSnort = NULL;
}


/*******************************************************************************
 * (This documentation was taken from Microsoft's own doc's on how to create
 * a Win32 Service.)
 *
 * Writing a ServiceMain Function
 * -----------------------------------------------------------------------------
 *
 * The SnortServiceStart function in the following example is the entry point for
 * the service. SnortServiceStart has access to the command-line arguments, in the
 * way that the main function of a console application does. The first parameter
 * contains the number of arguments being passed to the service. There will
 * always be at least one argument. The second parameter is a pointer to an
 * array of string pointers. The first item in the array always points to the
 * service name.
 *
 * The SnortServiceStart function first fills in the SERVICE_STATUS structure
 * including the control codes that it accepts. Although this service accepts
 * SERVICE_CONTROL_PAUSE and SERVICE_CONTROL_CONTINUE, it does nothing
 * significant when told to pause. The flags SERVICE_ACCEPT_PAUSE_CONTINUE was
 * included for illustration purposes only; if pausing does not add value to
 * your service, do not support it.
 *
 * The SnortServiceStart function then calls the RegisterServiceCtrlHandler
 * function to register SnortService as the service's Handler function and begin
 * initialization. The following sample initialization function,
 * SnortServiceInitialization, is included for illustration purposes; it does not
 * perform any initialization tasks such as creating additional threads. If
 * your service's initialization performs tasks that are expected to take longer
 * than one second, your code must call the SetServiceStatus function
 * periodically to send out wait hints and check points indicating that progress
 * is being made.
 *
 * When initialization has completed successfully, the example calls
 * SetServiceStatus with a status of SERVICE_RUNNING and the service continues
 * with its work. If an error has occurred in initialization, SnortServiceStart
 * reports SERVICE_STOPPED with the SetServiceStatus function and returns.
 *
 * Because this sample service does not complete any real tasks, SnortServiceStart
 * simply returns control to the caller. However, your service should use this
 * thread to complete whatever tasks it was designed to do. If a service does not
 * need a thread to do its work (such as a service that only processes RPC
 * requests), its ServiceMain function should return control to the caller. It is
 * important for the function to return, rather than call the ExitThread
 * function, because returning allows for cleanup of the memory allocated for the
 * arguments.
 *
 * To output debugging information, SnortServiceStart calls SvcDebugOut. The source
 * code for SvcDebugOut is given in Writing a Service Program's main Function.
 *******************************************************************************/

void logmsg(char* msg)
{
    FILE *pFile;
    if( (pFile=fopen("c:\\snortlog.txt", "a")) != NULL )
    {
        if( msg != NULL )
        {
            fprintf(pFile,"%s",msg);
        }
        else
        {
            fprintf(pFile,"%s","Message String is NULL\n");
        }
        fclose(pFile);
        pFile = NULL;
    }
}

void logadapternames( char* interfacenames, char* errorbuf )
{
    char AdaptersName[8192];
    int i;
    memset(AdaptersName, 0x00, sizeof(AdaptersName));
    for( i=0; i<sizeof(AdaptersName); i+=2 )
    {
        AdaptersName[i/2] = interfacenames[i];
    }
    for( i=0; i<sizeof(AdaptersName)-1; i++ )
    {
        if( AdaptersName[i] == 0x00 && AdaptersName[i+1] != 0x00 )
        {
            AdaptersName[i] = '\n';
        }
    }
    logmsg("Errorbuf:");
    logmsg(errorbuf);
    logmsg("\nAdaptersName:");
    logmsg(AdaptersName);
    logmsg("\n");
}

void WINAPI SnortServiceStart (DWORD argc, LPTSTR *argv)
{
    int i;
    int iArgCounter;
    char** argvDynamic = NULL;
    char errorbuf[PCAP_ERRBUF_SIZE];
    char *interfacenames = NULL;

    DWORD dwStatus;
    DWORD dwSpecificError;

    g_SnortServiceStatus.dwServiceType             = SERVICE_WIN32;
    g_SnortServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
    g_SnortServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
    g_SnortServiceStatus.dwWin32ExitCode           = 0;
    g_SnortServiceStatus.dwServiceSpecificExitCode = 0;
    g_SnortServiceStatus.dwCheckPoint              = 0;
    g_SnortServiceStatus.dwWaitHint                = 0;

    g_SnortServiceStatusHandle = RegisterServiceCtrlHandler(g_lpszServiceName, SnortServiceCtrlHandler);

    if (g_SnortServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        SvcDebugOut(" [SNORT_SERVICE] RegisterServiceCtrlHandler failed %d\n", GetLastError());
        FatalError (" [SNORT_SERVICE] RegisterServiceCtrlHandler failed %d\n%s\n", GetLastError(), szMsg);
        return;
    }

    /* Initialization code goes here. */
    dwStatus = SnortServiceInitialization(argc, argv, &dwSpecificError);

    /* Handle error condition */
    if (dwStatus != NO_ERROR)
    {
        g_SnortServiceStatus.dwCurrentState            = SERVICE_STOPPED;
        g_SnortServiceStatus.dwCheckPoint              = 0;
        g_SnortServiceStatus.dwWaitHint                = 0;
        g_SnortServiceStatus.dwWin32ExitCode           = dwStatus;
        g_SnortServiceStatus.dwServiceSpecificExitCode = dwSpecificError;

        SetServiceStatus (g_SnortServiceStatusHandle, &g_SnortServiceStatus);
        return;
    }

    /* Initialization complete - report running status. */
    g_SnortServiceStatus.dwCurrentState       = SERVICE_RUNNING;
    g_SnortServiceStatus.dwCheckPoint         = 0;
    g_SnortServiceStatus.dwWaitHint           = 0;

    if (!SetServiceStatus (g_SnortServiceStatusHandle, &g_SnortServiceStatus))
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        dwStatus = GetLastError();
        SvcDebugOut(" [SNORT_SERVICE] SetServiceStatus error %ld\n",dwStatus);
        FatalError (" [SNORT_SERVICE] SetServiceStatus error %ld\n%s\n",dwStatus,szMsg);
    }

    /* There seems to be a bug in Winpcap, such that snort works fine
     * when it is started from the command line, and the service works
     * fine if a user has run "snort -W" already.  However the service
     * fails to start (on some machines only) if snort hasn't been
     * run manually first.  So here we simulate a user having run
     * "snort -W", then let it go on its merry way.
     */
    memset( errorbuf, '\0', sizeof(errorbuf) );
    interfacenames = pcap_lookupdev(errorbuf);
    logadapternames( interfacenames, errorbuf );

    /* This is where the service does its work. */
    ReadServiceCommandLineParams( &iArgCounter, &argvDynamic );
    SnortMain( iArgCounter+1, argvDynamic );

    /* Cleanup now */
    for( i=0; i<=iArgCounter; i++ )
    {
        free( argvDynamic[i] );
        argvDynamic[i] = NULL;
    }
    free( argvDynamic );
    argvDynamic = NULL;

    SvcDebugOut(" [SNORT_SERVICE] Returning the Main Thread \n",0);

    return;
}

/* Stub initialization function. */
DWORD SnortServiceInitialization(DWORD argc, LPTSTR *argv, DWORD *pdwSpecificError)
{
    argv;
    argc;
    pdwSpecificError;
    return(0);
}



/*******************************************************************************
 * (This documentation was taken from Microsoft's own doc's on how to create
 * a Win32 Service.)
 *
 * Writing a Control Handler Function
 * -----------------------------------------------------------------------------
 *
 * The SnortServiceCtrlHandler function in the following example is the Handler
 * function. When this function is called by the dispatcher thread, it handles
 * the control code passed in the Opcode parameter and then calls the
 * SetServiceStatus function to update the service's status. Every time a
 * Handler function receives a control code, it is appropriate to return status
 * with a call to SetServiceStatus regardless of whether the service acts on
 * the control.
 *
 * When the pause control is received, SnortServiceCtrlHandler simply sets the
 * dwCurrentState field in the SERVICE_STATUS structure to SERVICE_PAUSED.
 * Likewise, when the continue control is received, the state is set to
 * SERVICE_RUNNING. Therefore, SnortServiceCtrlHandler is not a good example of
 * how to handle the pause and continue controls. Because SnortServiceCtrlHandler
 * is a template for a Handler function, code for the pause and continue
 * controls is included for completeness. A service that supports either the
 * pause or continue control should handle these controls in a way that makes
 * sense. Many services support neither the pause or continue control. If the
 * service indicates that it does not support pause or continue with the
 * dwControlsAccepted parameter, then the SCM will not send pause or continue
 * controls to the service's Handler function.
 *
 * To output debugging information, SnortServiceCtrlHandler calls SvcDebugOut. The
 * source code for SvcDebugOut is listed in Writing a Service Program's main
 * Function. Also, note that the g_SnortServiceStatus variable is a global variable
 * and should be initialized as demonstrated in Writing a ServiceMain function.
 *******************************************************************************/

VOID WINAPI SnortServiceCtrlHandler (DWORD dwOpcode)
{
    DWORD dwStatus;

    switch(dwOpcode)
    {
        case SERVICE_CONTROL_PAUSE:
            /* Do whatever it takes to pause here.  */
            snort_conf->run_flags |= RUN_FLAG__PAUSE_SERVICE;

            g_SnortServiceStatus.dwCurrentState = SERVICE_PAUSED;
            break;

        case SERVICE_CONTROL_CONTINUE:
            /* Do whatever it takes to continue here.  */
            snort_conf->run_flags &= ~RUN_FLAG__PAUSE_SERVICE;

            g_SnortServiceStatus.dwCurrentState = SERVICE_RUNNING;
            break;

        case SERVICE_CONTROL_STOP:
            /* Do whatever it takes to stop here. */
            snort_conf->run_flags |= RUN_FLAG__TERMINATE_SERVICE;

            Sleep( PKT_TIMEOUT * 2 );   /* wait for 2x the timeout, just to ensure that things
                                         * the service has processed any last packets
                                         */

            g_SnortServiceStatus.dwWin32ExitCode = 0;
            g_SnortServiceStatus.dwCurrentState  = SERVICE_STOPPED;
            g_SnortServiceStatus.dwCheckPoint    = 0;
            g_SnortServiceStatus.dwWaitHint      = 0;

            if (!SetServiceStatus (g_SnortServiceStatusHandle, &g_SnortServiceStatus))
            {
                dwStatus = GetLastError();
                SvcDebugOut(" [SNORT_SERVICE] SetServiceStatus error %ld\n",dwStatus);
            }

            SvcDebugOut(" [SNORT_SERVICE] Leaving SnortService \n",0);
            return;

        case SERVICE_CONTROL_INTERROGATE:
            /* Fall through to send current status. */
            break;

        default:
            SvcDebugOut(" [SNORT_SERVICE] Unrecognized opcode %ld\n", dwOpcode);
    }

    /* Send current status.  */
    if (!SetServiceStatus (g_SnortServiceStatusHandle,  &g_SnortServiceStatus))
    {
        dwStatus = GetLastError();
        SvcDebugOut(" [SNORT_SERVICE] SetServiceStatus error %ld\n",dwStatus);
    }

    return;
}



/*******************************************************************************
 * (This documentation was taken from Microsoft's own doc's on how to create
 * a Win32 Service.)
 *
 * Installing a Service
 * -----------------------------------------------------------------------------
 *
 * A service configuration program uses the CreateService function to install a
 * service in a SCM database. The application-defined schSCManager handle must
 * have SC_MANAGER_CREATE_SERVICE access to the SCManager object. The following
 * example shows how to install a service.
 *******************************************************************************/

VOID InstallSnortService(int argc, char* argv[])
{
    SC_HANDLE schSCManager, schService;
    char buffer[_MAX_PATH+1];
    LPCTSTR lpszBinaryPathName = NULL;
    HKEY hkSnort = NULL;
    long lRegRC = 0;
    int iArgCounter;
    DWORD dwWriteCounter = 0;
#ifdef SET_SERVICE_DESCRIPTION
    SERVICE_DESCRIPTION sdBuf;
#endif


    printf("\n\n");
    printf(" [SNORT_SERVICE] Attempting to install the Snort service.\n");

    /**********
     * Build up a string which stores the full path to the Snort executable.
     * This takes into account the current working directory, along with a
     * relative path to the Snort executable.
     **********/
    memset( buffer, 0, sizeof(buffer) );
    if( _getcwd( buffer, _MAX_PATH ) == NULL )
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        FatalError(" [SNORT_SERVICE] Unable to determine current working directory. %s", szMsg);
    }

    if( buffer[strlen(buffer) - 1] != '\\' )
    {
        if (strlen(buffer) < _MAX_PATH)
        {
            int len = strlen(buffer);

            buffer[len] = '\\';
            buffer[len + 1] = '\0';
        }
        else
        {
            FatalError(" [SNORT_SERVICE] Unable to create full path to Snort binary.");
        }
    }

    SnortSnprintfAppend(buffer, _MAX_PATH + 1, "%s ", argv[0]);
    SnortSnprintfAppend(buffer, _MAX_PATH + 1, "%s", SERVICE_CMDLINE_PARAM);
    lpszBinaryPathName = buffer;

    printf("\n");
    printf(" [SNORT_SERVICE] The full path to the Snort binary appears to be:\n");
    printf("    %s\n", lpszBinaryPathName);



    /**********
     * Create the registry entries for Snort command line parameters
     **********/
    lRegRC = RegCreateKeyEx( HKEY_LOCAL_MACHINE,        /* handle to open key       */
                             g_lpszRegistryKey,         /* subkey name              */
                             0,                         /* reserved (must be zero)  */
                             NULL,                      /* class string             */
                             REG_OPTION_NON_VOLATILE,   /* special options          */
                             KEY_ALL_ACCESS,            /* desired security access  */
                             NULL,                      /* inheritance              */
                             &hkSnort,                  /* key handle               */
                             NULL                       /* disposition value buffer */
                           );
    if( lRegRC != ERROR_SUCCESS )
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        FatalError(" [SNORT_SERVICE] Unable to create Snort registry entry. %s", szMsg);
    }

    for( iArgCounter=1; iArgCounter<argc; iArgCounter++ )
    {
        /* ignore the Service command line parameters (/SERVICE, /INSTALL, /UNINSTALL)
         * and store all others in the registry
         */
        if( ( _stricmp(argv[iArgCounter],SERVICE_CMDLINE_PARAM)           == 0 )  ||
            ( _stricmp(argv[iArgCounter],SERVICE_INSTALL_CMDLINE_PARAM)   == 0 )  ||
            ( _stricmp(argv[iArgCounter],SERVICE_UNINSTALL_CMDLINE_PARAM) == 0 )   )
        {
            /* ignore it, because it isn't a real Snort command-line parameter */
        }
        else if( strlen(argv[iArgCounter]) > MAX_REGISTRY_DATA_LENGTH )
        {
            FatalError(" [SNORT_SERVICE] A single command line parameter cannot exceed %d characters.", MAX_REGISTRY_DATA_LENGTH);
        }
        else
        {
            char szSubkeyName[30];
            dwWriteCounter++;
            sprintf(szSubkeyName, g_lpszRegistryCmdFormat, dwWriteCounter);
            lRegRC = RegSetValueEx( hkSnort,                       /* handle to key to set value for */
                                    szSubkeyName,                  /* name of the value to set       */
                                    0,                             /* reserved                       */
                                    REG_SZ,                        /* flag for value type            */
                                    (LPBYTE) argv[iArgCounter],    /* address of value data          */
                                    strlen(argv[iArgCounter])      /* size of value data             */
                                  );
            if( lRegRC != ERROR_SUCCESS )
            {
                TCHAR szMsg[1000];
                SvcFormatMessage(szMsg, sizeof(szMsg));
                FatalError(" [SNORT_SERVICE] Unable to write Snort registry entry. %s", szMsg);
            }
        }
    } /* end for() */

    lRegRC = RegSetValueEx( hkSnort,                       /* handle to key to set value for */
                            g_lpszRegistryCountFormat,     /* name of the value to set       */
                            0,                             /* reserved                       */
                            REG_DWORD,                     /* flag for value type            */
                            (LPBYTE) &dwWriteCounter,      /* address of value data          */
                            sizeof(dwWriteCounter)         /* size of value data             */
                          );
    if( lRegRC != ERROR_SUCCESS )
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        FatalError(" [SNORT_SERVICE] Unable to write Snort registry entry. %s", szMsg);
    }

    lRegRC = RegCloseKey( hkSnort );
    if( lRegRC != ERROR_SUCCESS )
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        FatalError(" [SNORT_SERVICE] Unable to close Snort registry entry. %s", szMsg);
    }

    printf("\n");
    printf(" [SNORT_SERVICE] Successfully added registry keys to:\n");
    printf("    \\HKEY_LOCAL_MACHINE\\%s\\\n", g_lpszRegistryKey);


    /**********
     * Add Snort to the Services database
     **********/
    schSCManager = OpenSCManager(NULL,                    /* local machine                        */
                                 NULL,                    /* defaults to SERVICES_ACTIVE_DATABASE */
                                 SC_MANAGER_ALL_ACCESS);  /* full access rights                   */

    if (schSCManager == NULL)
    {
        DWORD dwErr = GetLastError();
        LPCTSTR lpszBasicMessage = "Unable to open a connection to the Services database.";
        TCHAR szMsg[1000];

        SvcFormatMessage(szMsg, sizeof(szMsg));
        switch(dwErr)
        {
        case ERROR_ACCESS_DENIED:
            FatalError(" [SNORT_SERVICE] %s Access is denied. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_DATABASE_DOES_NOT_EXIST:
            FatalError(" [SNORT_SERVICE] %s Services database does not exist. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_INVALID_PARAMETER:
            FatalError(" [SNORT_SERVICE] %s Invalid parameter. %s", lpszBasicMessage, szMsg);
            break;

        default:
            FatalError(" [SNORT_SERVICE] %s Unrecognized error (%d). %s", lpszBasicMessage, dwErr, szMsg);
            break;
        }
    }

    schService = CreateService( schSCManager,              /* SCManager database        */
                                g_lpszServiceName,         /* name of service           */
                                g_lpszServiceDisplayName,  /* service name to display   */
                                SERVICE_ALL_ACCESS,        /* desired access            */
                                SERVICE_WIN32_OWN_PROCESS, /* service type              */
                                SERVICE_DEMAND_START,      /* start type                */
                                SERVICE_ERROR_NORMAL,      /* error control type        */
                                lpszBinaryPathName,        /* service's binary          */
                                NULL,                      /* no load ordering group    */
                                NULL,                      /* no tag identifier         */
                                NULL,                      /* no dependencies           */
                                NULL,                      /* LocalSystem account       */
                                NULL);                     /* no password               */

    if (schService == NULL)
    {
        DWORD dwErr = GetLastError();
        LPCTSTR lpszBasicMessage = "Error while adding the Snort service to the Services database.";
        TCHAR szMsg[1000];

        SvcFormatMessage(szMsg, sizeof(szMsg));
        switch(dwErr)
        {
        case ERROR_ACCESS_DENIED:
            FatalError(" [SNORT_SERVICE] %s Access is denied. %s", lpszBasicMessage, szMsg);
            break;
        case ERROR_CIRCULAR_DEPENDENCY:
            FatalError(" [SNORT_SERVICE] %s Circular dependency. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_DUP_NAME:
            FatalError(" [SNORT_SERVICE] %s The display name (\"%s\") is already in use. %s", lpszBasicMessage
                                                                                            , g_lpszServiceDisplayName
                                                                                            , szMsg);
            break;

        case ERROR_INVALID_HANDLE:
            FatalError(" [SNORT_SERVICE] %s Invalid handle. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_INVALID_NAME:
            FatalError(" [SNORT_SERVICE] %s Invalid service name. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_INVALID_PARAMETER:
            FatalError(" [SNORT_SERVICE] %s Invalid parameter. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_INVALID_SERVICE_ACCOUNT:
            FatalError(" [SNORT_SERVICE] %s Invalid service account. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_SERVICE_EXISTS:
            FatalError(" [SNORT_SERVICE] %s Service already exists. %s", lpszBasicMessage, szMsg);
            break;

        default:
            FatalError(" [SNORT_SERVICE] %s Unrecognized error (%d). %s", lpszBasicMessage, dwErr, szMsg);
            break;
        }
    }

#ifdef SET_SERVICE_DESCRIPTION
    /* Apparently, the call to ChangeServiceConfig2() only works on Windows >= 2000 */
    sdBuf.lpDescription = g_lpszServiceDescription;
    if( !ChangeServiceConfig2(schService,                 /* handle to service      */
                              SERVICE_CONFIG_DESCRIPTION, /* change: description    */
                              &sdBuf) )                   /* value: new description */
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        FatalError(" [SNORT_SERVICE] Unable to add a description to the Snort service. %s", szMsg);
    }
#endif

    printf("\n");
    printf(" [SNORT_SERVICE] Successfully added the Snort service to the Services database.\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}



/*******************************************************************************
 * (This documentation was taken from Microsoft's own doc's on how to create
 * a Win32 Service.)
 *
 * Deleting a Service
 * -----------------------------------------------------------------------------
 *
 * In the following example, a service configuration program uses the
 * OpenService function to get a handle with DELETE access to an installed
 * service object. The program then uses the service object handle in the
 * DeleteService function to remove the service from the SCM database.
 *******************************************************************************/

VOID UninstallSnortService()
{
    SC_HANDLE schSCManager, schService;
    //HKEY hkSnort = NULL;
    long lRegRC = 0;

    printf("\n\n");
    printf(" [SNORT_SERVICE] Attempting to uninstall the Snort service.\n");


    /**********
     * Removing the registry entries for Snort command line parameters
     **********/
    lRegRC = RegDeleteKey( HKEY_LOCAL_MACHINE,  /* handle to open key */
                           g_lpszRegistryKey    /* subkey name        */
                         );
    if( lRegRC != ERROR_SUCCESS )
    {
        TCHAR szMsg[1000];
        SvcFormatMessage(szMsg, sizeof(szMsg));
        printf(" [SNORT_SERVICE] Warning.  Unable to remove root Snort registry entry. %s", szMsg);
    }

    printf("\n");
    printf(" [SNORT_SERVICE] Successfully removed registry keys from:\n");
    printf("    \\HKEY_LOCAL_MACHINE\\%s\\\n", g_lpszRegistryKey);


    /**********
     * Remove Snort from the Services database
     **********/
    schSCManager = OpenSCManager(NULL,                    /* local machine            */
                                 NULL,                    /* ServicesActive database  */
                                 SC_MANAGER_ALL_ACCESS);  /* full access rights       */

    if (schSCManager == NULL)
    {
        DWORD dwErr = GetLastError();
        LPCTSTR lpszBasicMessage = "Unable to open a connection to the Services database.";
        TCHAR szMsg[1000];

        SvcFormatMessage(szMsg, sizeof(szMsg));
        switch(dwErr)
        {
        case ERROR_ACCESS_DENIED:
            FatalError(" [SNORT_SERVICE] %s Access is denied. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_DATABASE_DOES_NOT_EXIST:
            FatalError(" [SNORT_SERVICE] %s Services database does not exist. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_INVALID_PARAMETER:
            FatalError(" [SNORT_SERVICE] %s Invalid parameter. %s", lpszBasicMessage, szMsg);
            break;

        default:
            FatalError(" [SNORT_SERVICE] %s Unrecognized error (%d). %s", lpszBasicMessage, dwErr, szMsg);
            break;
        }
    }

    schService = OpenService(schSCManager,       /* SCManager database       */
                             g_lpszServiceName,  /* name of service          */
                             DELETE);            /* only need DELETE access  */

    if (schService == NULL)
    {
        DWORD dwErr = GetLastError();
        LPCTSTR lpszBasicMessage = "Unable to locate Snort in the Services database.";
        TCHAR szMsg[1000];

        SvcFormatMessage(szMsg, sizeof(szMsg));
        switch(dwErr)
        {
        case ERROR_ACCESS_DENIED:
            FatalError(" [SNORT_SERVICE] %s Access is denied. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_INVALID_HANDLE:
            FatalError(" [SNORT_SERVICE] %s Invalid handle. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_INVALID_NAME:
            FatalError(" [SNORT_SERVICE] %s Invalid name. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_SERVICE_DOES_NOT_EXIST:
            FatalError(" [SNORT_SERVICE] %s Service does not exist. %s", lpszBasicMessage, szMsg);
            break;

        default:
            FatalError(" [SNORT_SERVICE] %s Unrecognized error (%d). %s", lpszBasicMessage, dwErr, szMsg);
            break;
        }
    }

    if (! DeleteService(schService) )
    {
        DWORD dwErr = GetLastError();
        LPCTSTR lpszBasicMessage = "Unable to remove Snort from the Services database.";
        TCHAR szMsg[1000];

        SvcFormatMessage(szMsg, sizeof(szMsg));
        switch(dwErr)
        {
        case ERROR_ACCESS_DENIED:
            FatalError(" [SNORT_SERVICE] %s Access is denied. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_INVALID_HANDLE:
            FatalError(" [SNORT_SERVICE] %s Invalid handle. %s", lpszBasicMessage, szMsg);
            break;

        case ERROR_SERVICE_MARKED_FOR_DELETE:
            FatalError(" [SNORT_SERVICE] %s Service already marked for delete. %s", lpszBasicMessage, szMsg);
            break;

        default:
            FatalError(" [SNORT_SERVICE] %s Unrecognized error (%d). %s", lpszBasicMessage, dwErr, szMsg);
            break;
        }
    }

    printf("\n");
    printf(" [SNORT_SERVICE] Successfully removed the Snort service from the Services database.\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


VOID  ShowSnortServiceParams(void)
{
    int     argc;
    char ** argv;
    int i;

    ReadServiceCommandLineParams( &argc, &argv );

    printf("\n"
           "Snort is currently configured to run as a Windows service using the following\n"
           "command-line parameters:\n\n"
           "    ");

    for( i=1; i<=argc; i++ )
    {
        if( argv[i] != NULL )
        {
            printf(" %s", argv[i]);
            free( argv[i] );
            argv[i] = NULL;
        }
    }

    free( argv );
    argv = NULL;

    printf("\n");
}


#endif  /* ENABLE_WIN32_SERVICE */


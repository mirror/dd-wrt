/*
 * 
 * * Windows Service related function definitions
 * * By Raju Krishnappa(raju_krishnappa@yahoo.com)
 * *
 */  
    
#include <windows.h>
#include <tchar.h>
    
#include <stdio.h>   /* sprintf */
#include <process.h>  /* beginthreadex  */
    
#include <net-snmp/library/winservice.h>
    
    /*
     * 
     * * External global variables used here
     */ 
    
    /*
     * Application Name 
     */ 
    /*
     * This should be decalred by the application, which wants to register as
     * * windows servcie
     */ 
extern LPTSTR   g_szAppName;

    /*
     * 
     * * Declare global variable
     */ 
    
    /*
     * Flag to indicate, whether process is running as Service 
     */ 
    BOOL g_fRunningAsService = FALSE;

    /*
     * Varibale to maintain Current Service status 
     */ 
static SERVICE_STATUS ServiceStatus;

    /*
     * Service Handle 
     */ 
static SERVICE_STATUS_HANDLE hServiceStatus = 0L;

    /*
     * Service Table Entry 
     */ 
    SERVICE_TABLE_ENTRY ServiceTableEntry[] = {
    NULL, ServiceMain, /* Service Main function */ 
NULL, NULL};


    /*
     * Handle to Thread, to implement Pause,Resume and stop funcitonality 
     */ 
static HANDLE   hServiceThread = NULL;  /* Thread Handle */

    /*
     * Holds calling partys Function Entry point, that should started 
     * * when entered to service mode
     */ 
static          INT(*ServiceEntryPoint) (INT Argc, LPTSTR Argv[]) = 0L;

    /*
     * 
     * * To hold Stop Function address, to be called when STOP request
     * * recived from the SCM
     */ 
static          VOID(*StopFunction) () = 0L;

    /*
     * 
     * * To register as Windows Service with SCM(Service Control Manager)
     * * Input - Service Name, Serivce Display Name,Service Description and
     * * Service startup arguments
     */ 
    VOID RegisterService(LPCTSTR lpszServiceName, LPCTSTR lpszServiceDisplayName, LPCTSTR lpszServiceDescription, InputParams * StartUpArg) /* Startup argument to the service */
    
{
    TCHAR szServicePath[MAX_PATH];     /* To hold module File name */
    TCHAR MsgErrorString[MAX_STR_SIZE];        /* Message or Error string */
    TCHAR szServiceCommand[MAX_PATH + 9];      /* Command to execute */
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hService = NULL;
    TCHAR szRegAppLogKey[] =
        "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
    TCHAR szRegKey[512];
    HKEY hKey = NULL;          /* Key to registry entry */
    HKEY hParamKey = NULL;     /* To store startup parameters */
    DWORD dwData;              /* Type of logging supported */
    DWORD i, j;               /* Loop variables */
    GetModuleFileName(NULL, szServicePath, MAX_PATH);
    __try  {
        
            /*
             * Open Service Control Manager handle 
             */ 
            hSCManager =
            OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (hSCManager == NULL)
             {
            DisplayError(_T("Can't open SCM"));
            __leave;
            }
        
            /*
             * Generate the Command to be executed by SCM 
             */ 
            _stprintf(szServiceCommand, "%s %s", szServicePath,
                      _T("-service"));
        
            /*
             * Create the Desired service 
             */ 
            hService = CreateService(hSCManager, lpszServiceName, lpszServiceDisplayName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szServiceCommand, NULL,      /* load-order group */
                                     NULL,      /* group member tag */
                                     NULL,      /* dependencies */
                                     NULL,      /* account */
                                     NULL);     /* password */
        if (hService == NULL)
             {
            
                /*
                 * Generate Error String 
                 */ 
                _stprintf(MsgErrorString, "%s %s",
                          _T("Can't Create Service"),
                          lpszServiceDisplayName);
            DisplayError(MsgErrorString);
            __leave;
            }
        
            /*
             * Create registry entires for EventLog 
             */ 
            /*
             * Create registry Application event log key 
             */ 
            _tcscpy(szRegKey, szRegAppLogKey);
        _tcscat(szRegKey, lpszServiceName);
        
            /*
             * Create registry key 
             */ 
            if (RegCreateKey(HKEY_LOCAL_MACHINE, szRegKey, &hKey) !=
                ERROR_SUCCESS)
             {
            _stprintf(MsgErrorString, "%s %s",
                       _T("Unable to create registry entires"),
                       lpszServiceDisplayName);
            DisplayError(MsgErrorString);
            __leave;
            }
        
            /*
             * Add Event ID message file name to the 'EventMessageFile' subkey 
             */ 
            RegSetValueEx(hKey, "EventMessageFile", 0, REG_EXPAND_SZ, 
                          (CONST BYTE *) szServicePath,
                          _tcslen(szServicePath) + sizeof(TCHAR));
        
            /*
             * Set the supported types flags. 
             */ 
            dwData =
            EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
            EVENTLOG_INFORMATION_TYPE;
        RegSetValueEx(hKey, "TypesSupported", 0, REG_DWORD,
                       (CONST BYTE *) & dwData, sizeof(DWORD));
        
            /*
             * Close Registry key 
             */ 
            RegCloseKey(hKey);
        
            /*
             * Set Service Description String  and save startup parameters if present
             */ 
            if (lpszServiceDescription != NULL || StartUpArg->Argc > 2)
             {
            
                /*
                 * Create Registry Key path 
                 */ 
                _tcscpy(szRegKey,
                        _T("SYSTEM\\CurrentControlSet\\Services\\"));
            _tcscat(szRegKey, g_szAppName);
            hKey = NULL;
            
                /*
                 * Open Registry key 
                 */ 
                if (RegOpenKeyEx
                    (HKEY_LOCAL_MACHINE, szRegKey, 0, KEY_WRITE,
                     /*
                      * Create and Set access 
                      */ 
                     &hKey) != ERROR_SUCCESS)
                 {
                _stprintf(MsgErrorString, "%s %s",
                           _T("Unable to create registry entires"),
                           lpszServiceDisplayName);
                DisplayError(MsgErrorString);
                __leave;
                }
            
                /*
                 * Create description subkey and the set value 
                 */ 
                if (lpszServiceDescription != NULL)
                 {
                if (RegSetValueEx(hKey, "Description", 0, REG_SZ, 
                                   (CONST BYTE *) lpszServiceDescription,
                                   _tcslen(lpszServiceDescription) +
                                   sizeof(TCHAR)) != ERROR_SUCCESS)
                     {
                    _stprintf(MsgErrorString, "%s %s",
                               _T("Unable to create registry entires"),
                               lpszServiceDisplayName);
                    DisplayError(MsgErrorString);
                    __leave;
                    };
                }
            
                /*
                 * Save startup arguments if they are present 
                 */ 
                if (StartUpArg->Argc > 2)
                 {
                
                    /*
                     * Create Subkey parameters 
                     */ 
                    if (RegCreateKeyEx
                        (hKey, "Parameters", 0, NULL,
                         REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                         &hParamKey, NULL) != ERROR_SUCCESS)
                     {
                    _stprintf(MsgErrorString, "%s %s",
                               _T("Unable to create registry entires"),
                               lpszServiceDisplayName);
                    DisplayError(MsgErrorString);
                    __leave;
                    }
                
                    /*
                     * Save parameters 
                     */ 
                    
                    /*
                     * Loop through arguments 
                     */ 
                    for (i = 2, j = 1; i < StartUpArg->Argc; i++, j++)
                     {
                    _stprintf(szRegKey, "%s%d", _T("Param"), j);
                    
                        /*
                         * Create registry key 
                         */ 
                        if (RegSetValueEx
                            (hParamKey, szRegKey, 0, REG_SZ,
                             (CONST BYTE *) StartUpArg->Argv[i],
                             _tcslen(StartUpArg->Argv[i]) +
                             sizeof(TCHAR)) != ERROR_SUCCESS)
                         {
                        _stprintf(MsgErrorString, "%s %s",
                                   _T("Unable to create registry entires"),
                                   lpszServiceDisplayName);
                        DisplayError(MsgErrorString);
                        __leave;
                        };
                    }
                }
            
                /*
                 * Everything is set, delete hKey 
                 */ 
                RegCloseKey(hParamKey);
            RegCloseKey(hKey);
            }
        
            /*
             * Ready to Log messages 
             */ 
            
            /*
             * Successfully registered as service 
             */ 
            _stprintf(MsgErrorString, "%s %s", lpszServiceName,
                      _T("- Successfully registered as Service"));
        
            /*
             * Log message to eventlog 
             */ 
            WriteToEventLog(EVENTLOG_INFORMATION_TYPE, MsgErrorString);
        MessageBox(NULL, MsgErrorString, g_szAppName,
                     MB_ICONINFORMATION);
    }
    __finally  {
        if (hSCManager)
            CloseServiceHandle(hSCManager);
        if (hService)
            CloseServiceHandle(hService);
        if (hKey)
            RegCloseKey(hKey);
        if (hParamKey)
            RegCloseKey(hParamKey);
    }
}


    /*
     * 
     * * Unregister the service with the  Windows SCM 
     * * Input - ServiceName
     * *
     */ 
    VOID UnregisterService(LPCSTR lpszServiceName) 
{
    TCHAR MsgErrorString[MAX_STR_SIZE];        /* Message or Error string */
    SC_HANDLE hSCManager = NULL;      /* SCM handle */
    SC_HANDLE hService = NULL; /* Service Handle */
    SERVICE_STATUS sStatus;
    TCHAR szRegAppLogKey[] =
        "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
    TCHAR szRegKey[512];
    HKEY hKey = NULL;          /* Key to registry entry */
    __try  {
        
            /*
             * Open Service Control Manager 
             */ 
            hSCManager =
            OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (hSCManager == NULL)
             {
            
                /*
                 * Error while opening SCM 
                 */ 
                MessageBox(NULL, _T("Can't open SCM"), g_szAppName,
                            MB_ICONHAND);
            __leave;
            }
        
            /*
             * Open registered service 
             */ 
            hService =
            OpenService(hSCManager, lpszServiceName, SERVICE_ALL_ACCESS);
        if (hService == NULL)
             {
            _stprintf(MsgErrorString, "%s %s", _T("Can't open service"),
                       lpszServiceName);
            MessageBox(NULL, MsgErrorString, g_szAppName, MB_ICONHAND);
            __leave;
            }
        
            /*
             * Query service status 
             */ 
            /*
             * If running stop before deleting 
             */ 
            if (QueryServiceStatus(hService, &sStatus))
             {
            if (sStatus.dwCurrentState == SERVICE_RUNNING
                 || sStatus.dwCurrentState == SERVICE_PAUSED)
                 {
                
                    /*
                     * Shutdown the service 
                     */ 
                    ControlService(hService, SERVICE_CONTROL_STOP,
                                   &sStatus);
                }
            };
        
            /*
             * Delete the service  
             */ 
            if (DeleteService(hService) == FALSE)
             {
            _stprintf(MsgErrorString, "%s %s", _T("Can't delete service"),
                       lpszServiceName);
            MessageBox(NULL, MsgErrorString, g_szAppName, MB_ICONHAND);
            
                /*
                 * Log message to eventlog 
                 */ 
                WriteToEventLog(EVENTLOG_INFORMATION_TYPE, MsgErrorString);
            __leave;
            }
        
            /*
             * Service deleted successfully 
             */ 
            _stprintf(MsgErrorString, "%s %s", lpszServiceName,
                      _T("- Service deleted"));
        
            /*
             * Log message to eventlog 
             */ 
            WriteToEventLog(EVENTLOG_INFORMATION_TYPE, MsgErrorString);
        
            /*
             * Delete registry entires for EventLog 
             */ 
            _tcscpy(szRegKey, szRegAppLogKey);
        _tcscat(szRegKey, lpszServiceName);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szRegKey);
        MessageBox(NULL, MsgErrorString, g_szAppName,
                     MB_ICONINFORMATION);
    }
    
        /*
         * Delete the handles 
         */ 
        __finally  {
        if (hService)
            CloseServiceHandle(hService);
        if (hSCManager)
            CloseServiceHandle(hSCManager);
    }
}


    /*
     * 
     * * To write message to Windows Event log
     * * Input - Event Type, Message string
     * *
     */ 
    VOID WriteToEventLog(WORD wType, LPCTSTR pszFormat,...) 
{
    TCHAR szMessage[512];
    LPTSTR LogStr[1];
    va_list ArgList;
    HANDLE hEventSource = NULL;
    va_start(ArgList, pszFormat);
    _vstprintf(szMessage, pszFormat, ArgList);
    va_end(ArgList);
    LogStr[0] = szMessage;
    hEventSource = RegisterEventSource(NULL, g_szAppName);
    if (hEventSource == NULL)
        return;
    ReportEvent(hEventSource, wType, 0, DISPLAY_MSG, /* To Just output the text to event log */ 
                  NULL, 1, 0, LogStr, NULL);
    DeregisterEventSource(hEventSource);
    if (!g_fRunningAsService)
         {
        
            /*
             * We are running in command mode, output the string 
             */ 
            _putts(szMessage);
        }
}


    /*
     * 
     * * Handle command-line arguments from the user. 
     * *     Serivce related options are:
     * *     -register       - registers the service
     * *     -unregister     - unregisters the service
     * *     -service        - run as serivce
     * *     other command-line arguments are unaltered/ignored.
     * *     They should supplied as first arguments(other wise they will be ignored
     * * Return: Type indicating the option specified
     */ 
    INT ParseCmdLineForServiceOption(int argc, TCHAR * argv[]) 
{
    int            nReturn = RUN_AS_CONSOLE;   /* Defualted to run as console */
    if (argc >= 2)
         {
        
            /*
             * second argument present 
             */ 
            if (lstrcmpi(_T("-register"), argv[1]) == 0)
             {
            nReturn = REGISTER_SERVICE;
            }
        
        else if (lstrcmpi(_T("-unregister"), argv[1]) == 0)
             {
            nReturn = UN_REGISTER_SERVICE;
            }
        
        else if (lstrcmpi(_T("-service"), argv[1]) == 0)
             {
            nReturn = RUN_AS_SERVICE;
            }
        }
    return nReturn;
}


    /*
     * 
     * * To Display an error message describing the last system error
     * * message, along with a title passed as a parameter.
     */ 
    VOID DisplayError(LPCTSTR pszTitle) 
{
    LPVOID pErrorMsg;
    
        /*
         * Build Error String 
         */ 
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR) & pErrorMsg, 0, NULL);
    if (g_fRunningAsService != FALSE)
         {
        WriteToEventLog(EVENTLOG_ERROR_TYPE, pErrorMsg);
        }
    
    else
         {
        MessageBox(NULL, pErrorMsg, pszTitle, MB_ICONHAND);
        }
    LocalFree(pErrorMsg);
}


    /*
     * 
     * *  To update current service status 
     * *  Sends the current service status to the SCM. Also updates
     * *  the global service status structure.
     */ 
static          BOOL
UpdateServiceStatus(DWORD dwStatus, DWORD dwErrorCode,
                    DWORD dwWaitHint) 
{
    BOOL fReturn = FALSE;
    DWORD static   dwCheckpoint = 1;
    DWORD dwControls =
        SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
    if (g_fRunningAsService == FALSE)
        return FALSE;
    ZeroMemory(&ServiceStatus, sizeof(ServiceStatus));
    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = dwStatus;
    ServiceStatus.dwWaitHint = dwWaitHint;
    if (dwErrorCode)
         {
        ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        ServiceStatus.dwServiceSpecificExitCode = dwErrorCode;
        }
    
        /*
         * special cases that depend on the new state 
         */ 
        switch (dwStatus)
         {
    case SERVICE_START_PENDING:
        dwControls = 0;
        break;
    case SERVICE_RUNNING:
    case SERVICE_STOPPED:
        dwCheckpoint = 0;
        break;
        }
    ServiceStatus.dwCheckPoint = dwCheckpoint++;
    ServiceStatus.dwControlsAccepted = dwControls;
    return ReportCurrentServiceStatus();
}


    /*
     * 
     * * Reports current Service status to SCM
     */ 
static          BOOL
ReportCurrentServiceStatus() 
{
    return SetServiceStatus(hServiceStatus, &ServiceStatus);
}


    /*
     * 
     * * The ServiceMain function to start service.
     */ 
    VOID WINAPI ServiceMain(DWORD argc, LPTSTR argv[]) 
{
    SECURITY_ATTRIBUTES SecurityAttributes;
    DWORD dwThreadId;
    
        /*
         * Input Arguments to function startup 
         */ 
        DWORD ArgCount = 0;
    LPTSTR * ArgArray = NULL;
    TCHAR szRegKey[512];
    TCHAR szValue[128];
    DWORD nSize;
    HKEY hParamKey = NULL;     /* To read startup parameters */
    DWORD TotalParams = 0;
    DWORD i;
    InputParams ThreadInputParams;
    
        /*
         * Build the Input parameters to pass to thread 
         */ 
        
        /*
         * SCM sends Service Name as first arg, increment to point
         * * arguments user specified while starting contorl agent
         */ 
        
        /*
         * Read registry parameter 
         */ 
        /*
         * Initialize count to 1 
         */ 
        ArgCount = 1;
    
        /*
         * Create Registry Key path 
         */ 
        _stprintf(szRegKey, "%s%s\\%s",
                  _T("SYSTEM\\CurrentControlSet\\Services\\"), g_szAppName,
                  "Parameters");
    if (RegOpenKeyEx
         (HKEY_LOCAL_MACHINE, szRegKey, 0, KEY_ALL_ACCESS,
          &hParamKey) == ERROR_SUCCESS)
         {
        
            /*
             * Read startup Configuration information 
             */ 
            /*
             * Find number of subkeys inside parameters 
             */ 
            if (RegQueryInfoKey
                 (hParamKey, NULL, NULL, 0, NULL, NULL, NULL, &TotalParams,
                  NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
             {
            if (TotalParams != 0)
                 {
                ArgCount += TotalParams;
                
                    /*
                     * Allocate memory to hold strings 
                     */ 
                    ArgArray =
                    (LPTSTR *) malloc(sizeof(LPTSTR) * ArgCount);
                
                    /*
                     * Copy first argument 
                     */ 
                    ArgArray[0] = _tcsdup(argv[0]);
                for (i = 1; i <= TotalParams; i++)
                     {
                    
                        /*
                         * Create Subkey value name 
                         */ 
                        _stprintf(szRegKey, "%s%d", "Param", i);
                    
                        /*
                         * Set size 
                         */ 
                        nSize = 128;
                    RegQueryValueEx(hParamKey, szRegKey, 0, NULL,
                                     (LPBYTE) & szValue, &nSize);
                    ArgArray[i] = _tcsdup(szValue);
                    }
                }
            }
        RegCloseKey(hParamKey);
        }
    if (ArgCount == 1)
         {
        
            /*
             * No statup agrs are given 
             */ 
            ThreadInputParams.Argc = argc;
        ThreadInputParams.Argv = argv;
        }
    
    else
         {
        ThreadInputParams.Argc = ArgCount;
        ThreadInputParams.Argv = ArgArray;
        }
    
        /*
         * Register Serivce Control Handler 
         */ 
        hServiceStatus =
        RegisterServiceCtrlHandler(g_szAppName, ControlHandler);
    if (hServiceStatus == 0)
         {
        WriteToEventLog(EVENTLOG_ERROR_TYPE,
                         _T("RegisterServiceCtrlHandler failed"));
        return;
        }
    
        /*
         * Update the service status to START_PENDING 
         */ 
        UpdateServiceStatus(SERVICE_START_PENDING, NO_ERROR,
                            SCM_WAIT_INTERVAL);
    
        /*
         * Spin of worker thread, which does majority of the work 
         */ 
        __try  {
        if (SetSimpleSecurityAttributes(&SecurityAttributes) == FALSE)
             {
            WriteToEventLog(EVENTLOG_ERROR_TYPE,
                             _T("Couldn't init security attributes"));
            __leave;
            }
        hServiceThread =
            (void *) _beginthreadex(&SecurityAttributes, 0,
                                    ThreadFunction,
                                    (void *) &ThreadInputParams, 0,
                                    &dwThreadId);
        if (hServiceThread == NULL)
             {
            WriteToEventLog(EVENTLOG_ERROR_TYPE,
                             _T("Couldn't start worker thread"));
            __leave;
            }
        
            /*
             * Set Service Status to Running 
             */ 
            UpdateServiceStatus(SERVICE_RUNNING, NO_ERROR,
                                SCM_WAIT_INTERVAL);
        
            /*
             * Wait for termination event and worker thread to
             * * spin down.
             */ 
            WaitForSingleObject(hServiceThread, INFINITE);
    }
    __finally  {
        
            /*
             * Release resources 
             */ 
            UpdateServiceStatus(SERVICE_STOPPED, NO_ERROR,
                                SCM_WAIT_INTERVAL);
        if (hServiceThread)
            CloseHandle(hServiceThread);
        FreeSecurityAttributes(&SecurityAttributes);
        
            /*
             * Delete allocated argument list 
             */ 
            if (ArgCount > 1 && ArgArray != NULL)
             {
            
                /*
                 * Delete all strings 
                 */ 
                for (i = 0; i < ArgCount; i++)
                 {
                free(ArgArray[i]);
                }
            free(ArgArray);
            }
    }
}


    /*
     * 
     * * Function to start as Windows service
     * * The calling party should specify their entry point as input parameter
     * * Returns TRUE if the Service is started successfully
     */ 
    BOOL RunAsService(INT(*ServiceFunction) (INT, LPTSTR *)) 
{
    
        /*
         * Set the ServiceEntryPoint 
         */ 
        ServiceEntryPoint = ServiceFunction;
    
        /*
         * By default, mark as Running as a service 
         */ 
        g_fRunningAsService = TRUE;
    
        /*
         * Initialize ServiceTableEntry table 
         */ 
        ServiceTableEntry[0].lpServiceName = g_szAppName;       /* Application Name */
    
        /*
         * Call SCM via StartServiceCtrlDispatcher to run as Service 
         * * If the function returns TRUE we are running as Service, 
         */ 
        if (StartServiceCtrlDispatcher(ServiceTableEntry) == FALSE)
         {
        g_fRunningAsService = FALSE;
        
            /*
             * Some other error has occurred. 
             */ 
            WriteToEventLog(EVENTLOG_ERROR_TYPE,
                            _T("Couldn't start service - %s"),
                            g_szAppName);
        }
    return g_fRunningAsService;
}


    /*
     * 
     * * Service control handler function
     * * Responds to SCM commands/requests
     * * The service handles 4 commands
     * * commands - interrogate,pause, continue and stop.
     */ 
    VOID WINAPI ControlHandler(DWORD dwControl) 
{
    switch (dwControl)
         {
    case SERVICE_CONTROL_STOP:
        ProcessServiceStop();  /* To stop the service */
        break;
    case SERVICE_CONTROL_INTERROGATE:
        ProcessServiceInterrogate();   /* Report Current state of the Service */
        break;
    case SERVICE_CONTROL_PAUSE:
        ProcessServicePause(); /* To puase service */
        break;
    case SERVICE_CONTROL_CONTINUE:
        ProcessServiceContinue();      /* To continue Service */
        break;
        }
}


    /*
     * 
     * * To stop the service.  This invokes registered
     * * stop function to stop the service(gracefull exit)
     * * After stopping, Service status is set to STOP in 
     * * main loop
     */ 
    VOID ProcessServiceStop(VOID) 
{
    UpdateServiceStatus(SERVICE_STOP_PENDING, NO_ERROR,
                         SCM_WAIT_INTERVAL);
    
        /*
         * Invoke registered Stop funciton 
         */ 
        if (StopFunction != NULL)
         {
        (*StopFunction) ();
        }
    
    else
         {
        
            /*
             * There is no registered stop function, so terminate the thread 
             */ 
            TerminateThread(hServiceThread, 0);
        }
}


    /*
     * 
     * * Returns the current state of the service to the SCM.
     */ 
    VOID ProcessServiceInterrogate(VOID) 
{
    ReportCurrentServiceStatus();
}


    /*
     * 
     * * To Create a security descriptor with a NULL ACL, which
     * * allows unlimited access. Returns a SECURITY_ATTRIBUTES
     * * structure that contains the security descriptor.
     * * The structure contains a dynamically allocated security
     * * descriptor that must be freed; either manually, or by
     * * calling FreeSecurityAttributes 
     */ 
    BOOL SetSimpleSecurityAttributes(SECURITY_ATTRIBUTES * pSecurityAttr) 
{
    BOOL fReturn = FALSE;
    SECURITY_DESCRIPTOR * pSecurityDesc = NULL;
    
        /*
         * If an invalid address passed as a parameter, return
         * * FALSE right away. 
         */ 
        if (!pSecurityAttr)
        return FALSE;
    pSecurityDesc =
        (SECURITY_DESCRIPTOR *) LocalAlloc(LPTR,
                                           SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (!pSecurityDesc)
        return FALSE;
    fReturn =
        InitializeSecurityDescriptor(pSecurityDesc,
                                     SECURITY_DESCRIPTOR_REVISION);
    if (fReturn != FALSE)
         {
        fReturn =
            SetSecurityDescriptorDacl(pSecurityDesc, TRUE, NULL, FALSE);
        }
    if (fReturn != FALSE)
         {
        pSecurityAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
        pSecurityAttr->lpSecurityDescriptor = pSecurityDesc;
        pSecurityAttr->bInheritHandle = TRUE;
        }
    
    else
         {
        
            /*
             * Couldn't initialize or set security descriptor. 
             */ 
            LocalFree(pSecurityDesc);
        }
    return fReturn;
}


    /*
     * 
     * * This funciton Frees the security descriptor owned by a SECURITY_ATTRIBUTES
     * * structure.
     */ 
    VOID FreeSecurityAttributes(SECURITY_ATTRIBUTES * pSecurityAttr) 
{
    if (pSecurityAttr && pSecurityAttr->lpSecurityDescriptor)
        LocalFree(pSecurityAttr->lpSecurityDescriptor);
}


    /*
     * TheadFunction
     * * This function is spawn as thread.
     * * Invokes registered service function
     * * Returns when called registered function returns
     */ 
    DWORD WINAPI ThreadFunction(LPVOID lpParam) 
{
    
        /*
         * lpParam contains argc and argv, pass to service main function 
         */ 
        
        /*
         * Declare pointer to InputParams 
         */ 
        InputParams * pInputArg;
    pInputArg = (InputParams *) lpParam;
    return (*ServiceEntryPoint) (pInputArg->Argc, pInputArg->Argv);
}


    /*
     * 
     * * To register STOP function with the framework
     * * This function will be inovked when SCM sends
     * * STOP command
     */ 
    VOID RegisterStopFunction(void (*StopFunc) ()) 
{
    StopFunction = StopFunc;
} 

    /*
     * 
     * * To Pause the service whec SCM sends pause command
     * * Invokes PauseThread on worker Thread handle, only
     * * when Service status is Running
     */ 
    VOID ProcessServicePause(VOID) 
{
    if (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
         {
        UpdateServiceStatus(SERVICE_PAUSE_PENDING, NO_ERROR,
                             SCM_WAIT_INTERVAL);
        
            /*
             * Invoke Thread pause on ThreadHandle 
             */ 
            if (SuspendThread(hServiceThread) != -1)
             {
            UpdateServiceStatus(SERVICE_PAUSED, NO_ERROR,
                                 SCM_WAIT_INTERVAL);
            }
        }
}


    /*
     * 
     * * To Continue paused service
     * * Invoke ResumeThread, if thread is paused
     */ 
    VOID ProcessServiceContinue(VOID) 
{
    if (ServiceStatus.dwCurrentState == SERVICE_PAUSED)
         {
        UpdateServiceStatus(SERVICE_CONTINUE_PENDING, NO_ERROR,
                             SCM_WAIT_INTERVAL);
        
            /*
             * Invoke Thread pause on ThreadHandle 
             */ 
            if (ResumeThread(hServiceThread) != -1)
             {
            UpdateServiceStatus(SERVICE_RUNNING, NO_ERROR,
                                 SCM_WAIT_INTERVAL);
            }
        }
}



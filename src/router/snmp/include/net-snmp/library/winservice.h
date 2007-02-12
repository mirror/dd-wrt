#ifndef WINSERVICE_H
#define WINSERVICE_H

    /*
     * 
     * *Windows Service related functions declaration
     * *By Raju Krishanppa(raju_krishnappa@yahoo.com)
     * *
     */ 
    
#ifdef __cplusplus
extern          "C" {
    
#endif  /*  */
    
        /*
         * 
         * * Define Constants for Register, De-register , Run As service or Console mode
         */ 
#define REGISTER_SERVICE 0
#define UN_REGISTER_SERVICE 1
#define RUN_AS_SERVICE 2
#define RUN_AS_CONSOLE 3
    
        /*
         * 
         * * Define Message catalog ID
         * * MessageId: DISPLAY_MSG
         * * MessageText:
         * *  %1.
         */ 
#define DISPLAY_MSG                      0x00000064L
    
        /*
         * 
         * * Hint Value to SCM to wait before sending successive commands to service
         */ 
#define SCM_WAIT_INTERVAL 7000
    
        /*
         * 
         * * Define Generic String Size, to hold Error or Information
         */ 
#define MAX_STR_SIZE  1024
    
        /*
         * Delcare Global variables, which are visible to other modules 
         */ 
    extern BOOL     g_fRunningAsService;
                   
        /*
         * Input parameter structure to thread 
         */            
    typedef struct _InputParams  {
        DWORD Argc;
        LPTSTR * Argv;
    } InputParams;
                   
        /*
         * 
         * * Define Service Related functions
         */            
                   
        /*
         * To register application as windows service with SCM 
         */            
     
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        VOID RegisterService(LPCTSTR lpszServiceName,
                             LPCTSTR lpszServiceDisplayName,
                             LPCTSTR lpszServiceDescription,
                             InputParams * StartUpArg);
                   
        /*
         * To unregister servcie 
         */            
                    VOID UnregisterService(LPCSTR lpszServiceName);
                   
        /*
         * To parse command line for startup option 
         */            
     
        
        
        
        
        
        
                INT ParseCmdLineForServiceOption(INT argc, TCHAR * argv[]);
                   
        /*
         * To write to windows event log 
         */            
     
        
        
        
        
                  VOID WriteToEventLog(WORD wType, LPCTSTR pszFormat, ...);
                   
        /*
         * To display generic windows error 
         */            
                    VOID DisplayError(LPCTSTR pszTitle);
                   
        /*
         * To update windows service status to SCM 
         */            
    static BOOL     UpdateServiceStatus(DWORD dwStatus, DWORD dwErrorCode,
                                        DWORD dwWaitHint);
                   
        /*
         * To Report current service status to SCM 
         */            
    static BOOL     ReportCurrentServiceStatus();
                   
        /*
         * Service Main function,  Which will spawn a thread, and calls the
         * * Service run part
         */            
                    VOID WINAPI ServiceMain(DWORD argc, LPTSTR argv[]);
                   
        /*
         * To start Service 
         */            
     
        
        
        
        
                  BOOL RunAsService(INT(*ServiceFunction) (INT, LPTSTR *));
                   
        /*
         * Call back function to process SCM Requests 
         */            
                    VOID WINAPI ControlHandler(DWORD dwControl);
                   
        /*
         * To Stop the service 
         */            
                    VOID ProcessServiceStop(VOID);
                   
        /*
         * To Pause service 
         */            
                    VOID ProcessServicePause(VOID);
                   
        /*
         * To Continue paused service 
         */            
                    VOID ProcessServiceContinue(VOID);
                   
        /*
         * To send Current Serivce status to SCM when INTERROGATE command is sent 
         */            
                    VOID ProcessServiceInterrogate(VOID);
                   
        /*
         * To allocate and Set security descriptor 
         */            
     
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        BOOL SetSimpleSecurityAttributes(SECURITY_ATTRIBUTES *
                                         pSecurityAttr);
                   
        /*
         * To free Securtiy Descriptor 
         */            
     
        
        
        
        
        
        
        
        
        
        
        
        
          VOID FreeSecurityAttributes(SECURITY_ATTRIBUTES * pSecurityAttr);
                   
        /*
         * TheadFunction - To spawan as thread - Invokes registered service function 
         */            
                    DWORD WINAPI ThreadFunction(LPVOID lpParam);
                   
        /*
         * Service STOP function registration with this framewrok
         * * this function must be invoked before calling RunAsService
         */            
                    VOID RegisterStopFunction(VOID(*StopFunc) ());
                   
#ifdef __cplusplus
}              
#endif  /*  */
#endif  /* WINSERVICE_H */

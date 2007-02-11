/*
 *  winExtDLL Net-SNMP extension
 *  (c) 2006 Alex Burger
 *
 *  Created 9/9/06
 *
 * Purpose:  To load Windows SNMP Service extension DLLs provided with Windows
 *           (such as hostmib.dll).  This allows Net-SNMP to be a replacement 
 *           for the Windows SNMP service.
 *
 * Notes:    This extension requires the PSDK including the Snmp.h header file.
 *           Including Snmp.h will conflict with existing Net-SNMP defines for
 *           ASN_OCTETSTRING etc.  To resolve this, create a copy of Snmp.h in
 *           the PSDK include/ folder called Snmp-winExtDLL.h and change all
 *           occurances of ASN_ to MS_ASN_
 *
 *           This extension requires that the Windows SNMP Service is installed
 *           but set to disabled.  This is required so that the extension DLLs 
 *           are available for loading, and also because this extension and the 
 *           existing Windows extensions use the Windows SNMP API from snmpapi.dll.
 *
 *           This extension is NOT for dynamically loading Net-SNMP extensions.
 */

#include <windows.h>
#include <cstdio>
#include <Snmp-winExtDLL.h>                  // Modified Windows SDK snmp.h.  See Notes above
#include <mgmtapi.h>
#include <string.h>

/*
 * include important headers 
 */
#include <net-snmp/net-snmp-config.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

/*
 * needed by util_funcs.h 
 */
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"

#include "winExtDLL.h"

#define SZBUF_MAX               1024
#define SZBUF_DLLNAME_MAX       254
#define MAX_WINEXT_DLLS         100
#define MAX_KEY_LENGTH          255
#define MAX_VALUE_NAME          16383
#define MAX_WINEXT_TRAP_EVENTS  100

#define DEBUGMSGWINOID(x)     do {if (_DBG_IF_) {__DBGMSGWINOID(x);} }while(0)
#define __DBGMSGWINOID(x)     debugmsg_win_oid x
void debugmsg_win_oid(const char *token, const AsnObjectIdentifier * theoid);

/* Structure to hold name, pointers to functions and MIB tree supported by
 * each Windows SNMP Extension DLL */
typedef struct {
  char          dll_name[SZBUF_DLLNAME_MAX];
  DWORD (WINAPI *xSnmpExtensionInit)(DWORD, HANDLE*, AsnObjectIdentifier*);
  DWORD (WINAPI *xSnmpExtensionInitEx)(AsnObjectIdentifier*);
  DWORD (WINAPI *xSnmpExtensionQuery)(BYTE, SnmpVarBindList* ,AsnInteger32* ,AsnInteger32*);
  DWORD (WINAPI *xSnmpExtensionQueryEx)(DWORD, DWORD, SnmpVarBindList*, AsnOctetString*, AsnInteger32*, AsnInteger32*);
  BOOL  (WINAPI *xSnmpExtensionTrap)( AsnObjectIdentifier *, AsnInteger *, AsnInteger *, AsnTimeticks *, SnmpVarBindList * );
  HANDLE        *subagentTrapEvent;
  netsnmp_handler_registration *my_handler;
  oid           name[MAX_OID_LEN];              //   pSupportedView in Net-SNMP format
  size_t        name_length;
  AsnObjectIdentifier pSupportedView;
} winExtensionAgents;

winExtensionAgents winExtensionAgent[MAX_WINEXT_DLLS];
winExtensionAgents winExtensionAgent_temp;      /* For sorting */
int winExtensionAgent_index = 0;

char *extDLLs[MAX_WINEXT_DLLS];
int extDLLs_index = 0;

HANDLE *subagentTrapEvents[MAX_WINEXT_TRAP_EVENTS];
int subagentTrapEvents_index = 0;

void winExtDLL_free_config_winExtDLL(void);

void read_ExtensionAgents_list();
void read_ExtensionAgents_list2(const TCHAR *);

void subagentTrapCheck();

void send_trap(
    AsnObjectIdentifier *, 
    AsnInteger *, 
    AsnInteger *, 
    AsnTimeticks *,    
    SnmpVarBindList *);

void init_winExtDLL(void)
{
  // Windows SNMP
  DWORD dwUptimeReference = 0;
  HANDLE subagentTrapEvent;
  AsnObjectIdentifier pSupportedView;
  BOOL result;
  HANDLE hThread;
  DWORD IDThread;

  char dll_name[SZBUF_DLLNAME_MAX];
  DWORD (WINAPI *xSnmpExtensionInit)(DWORD, HANDLE*, AsnObjectIdentifier*);
  DWORD (WINAPI *xSnmpExtensionInitEx)(AsnObjectIdentifier*);
  DWORD (WINAPI *xSnmpExtensionQuery)(BYTE, SnmpVarBindList* ,AsnInteger32* ,AsnInteger32*);
  DWORD (WINAPI *xSnmpExtensionQueryEx)(DWORD, DWORD, SnmpVarBindList*, AsnOctetString*, AsnInteger32*, AsnInteger32*);
  BOOL  (WINAPI *xSnmpExtensionTrap)( AsnObjectIdentifier *, AsnInteger *, AsnInteger *, AsnTimeticks *, SnmpVarBindList * );

  // Net-SNMP
  oid name[MAX_OID_LEN];
  size_t length = 0;
  int i;
  int DLLnum = 0;
  int winExtensionAgent_num = 0;
  
  int iter, indx;
  
  netsnmp_handler_registration *my_handler;

  HANDLE hInst = NULL;

  DEBUGMSGTL(("winExtDLL", "init_winExtDLL called\n"));

  read_ExtensionAgents_list();  
  
  DEBUGMSGTL(("winExtDLL", "winExtDLL enabled.\n"));   
  
  DEBUGMSGTL(("winExtDLL", "Size of winExtensionAgent: %d\n",sizeof(winExtensionAgent) / sizeof(winExtensionAgents)));

  for(i=0; i <= sizeof(winExtensionAgent) / sizeof(winExtensionAgents); i++) {
    winExtensionAgent[0].xSnmpExtensionInit = NULL;
    winExtensionAgent[0].xSnmpExtensionInitEx = NULL;
  }

  /* Load all the DLLs */
  for (DLLnum = 0; DLLnum <= extDLLs_index; DLLnum++) {

    if (! (extDLLs[DLLnum]))
      continue;

    DEBUGMSGTL(("winExtDLL", "-----------------------------------------\n"));
    DEBUGMSGTL(("winExtDLL", "DLL to load: %s, DLL number: %d, winExtensionAgent_num: %d\n", extDLLs[DLLnum], DLLnum,
          winExtensionAgent_num));
    DEBUGMSGTL(("winExtDLL", "Size of DLL to load: %d\n", strlen(extDLLs[DLLnum])));
    
    hInst = LoadLibrary(extDLLs[DLLnum]);
    
    if (hInst == NULL)
    {
      DEBUGMSGTL(("winExtDLL","Could not load Windows extension DLL %s.\n", extDLLs[DLLnum]));
      snmp_log(LOG_ERR,
          "Could not load Windows extension DLL: %s.\n", extDLLs[DLLnum]);
      continue;
    }
    else {
      DEBUGMSGTL(("winExtDLL","DLL loaded.\n"));
    }

    // Create local copy of DLL name and functions
    strncpy(dll_name, extDLLs[DLLnum], SZBUF_DLLNAME_MAX-1);
    xSnmpExtensionInit = (DWORD (WINAPI *)(DWORD, HANDLE*, AsnObjectIdentifier*)) 
      GetProcAddress ((HMODULE) hInst, "SnmpExtensionInit");
    xSnmpExtensionInitEx = (DWORD (WINAPI *)(AsnObjectIdentifier*)) 
      GetProcAddress ((HMODULE) hInst, "SnmpExtensionInitEx");
    xSnmpExtensionQuery = (DWORD (WINAPI *)(BYTE, SnmpVarBindList* ,AsnInteger32* ,AsnInteger32*)) 
      GetProcAddress ((HMODULE) hInst, "SnmpExtensionQuery");
    xSnmpExtensionQueryEx = (DWORD (WINAPI *)(DWORD, DWORD, SnmpVarBindList*, AsnOctetString*, AsnInteger32*, AsnInteger32*))
      GetProcAddress ((HMODULE) hInst, "SnmpExtensionQueryEx");
    xSnmpExtensionTrap = (BOOL  (WINAPI *)(AsnObjectIdentifier *, AsnInteger *, AsnInteger *, AsnTimeticks *, SnmpVarBindList * ))
      GetProcAddress ((HMODULE) hInst, "SnmpExtensionTrap");

    if (xSnmpExtensionQuery)
      DEBUGMSGTL(("winExtDLL", "xSnmpExtensionQuery found\n"));
    if (xSnmpExtensionQueryEx)
      DEBUGMSGTL(("winExtDLL", "xSnmpExtensionQueryEx found\n"));
    if (xSnmpExtensionQuery)
      DEBUGMSGTL(("winExtDLL", "xSnmpExtensionTrap found\n"));

    // Store DLL name and functions in winExtensionAgent array
    strncpy(winExtensionAgent[winExtensionAgent_num].dll_name, dll_name, SZBUF_DLLNAME_MAX-1);
    winExtensionAgent[winExtensionAgent_num].xSnmpExtensionInit = xSnmpExtensionInit;
    winExtensionAgent[winExtensionAgent_num].xSnmpExtensionInitEx = xSnmpExtensionInitEx;
    winExtensionAgent[winExtensionAgent_num].xSnmpExtensionQuery = xSnmpExtensionQuery;
    winExtensionAgent[winExtensionAgent_num].xSnmpExtensionQueryEx = xSnmpExtensionQueryEx;
    winExtensionAgent[winExtensionAgent_num].xSnmpExtensionTrap = xSnmpExtensionTrap;

    // Init and get first supported view from Windows SNMP extension DLL  
    result = xSnmpExtensionInit(dwUptimeReference, &subagentTrapEvent, &pSupportedView);

    DEBUGMSGTL(("winExtDLL", "Supported view: "));
    DEBUGMSGWINOID(("winExtDLL", &pSupportedView));
    DEBUGMSG(("winExtDLL", "\n"));

    // Store the subagent's trap handler, even if it's NULL
    winExtensionAgent[winExtensionAgent_num].subagentTrapEvent = subagentTrapEvent;

    // Store the subagent's trap handler in a global array for use by waitformultipleobjects()
    if (subagentTrapEvent) {
      DEBUGMSGTL(("winExtDLL", "Trap handler defined.  Storing...\n"));
      subagentTrapEvents[subagentTrapEvents_index] = subagentTrapEvent;
      subagentTrapEvents_index++;
    }

    // Convert OID from Windows 'supported view' to Net-SNMP
    for (i = 0; i < (pSupportedView.idLength > MAX_OID_LEN?MAX_OID_LEN:pSupportedView.idLength); i++) {
      name[i] = (oid)pSupportedView.ids[i];
    }
    length = i;

    // Store supported view in Net-SNMP format
    memcpy(winExtensionAgent[winExtensionAgent_num].name, name, sizeof(name));
    winExtensionAgent[winExtensionAgent_num].name_length = length;
 
    DEBUGMSGTL(("winExtDLL", "Windows OID converted to Net-SNMP: "));
    DEBUGMSGOID(("winExtDLL", name, length));
    DEBUGMSG(("winExtDLL", "\n"));

    // Store supported view in Windows format
    SnmpUtilOidCpy(&winExtensionAgent[winExtensionAgent_num].pSupportedView,&pSupportedView);

    // Create handler registration
    winExtensionAgent[winExtensionAgent_num].my_handler = netsnmp_create_handler_registration("winExtDLL",
        var_winExtDLL,
        name,
        length,
        HANDLER_CAN_RWRITE);
    
    if (!winExtensionAgent[winExtensionAgent_num].my_handler) {
      snmp_log(LOG_ERR,
          "malloc failed registering handler for winExtDLL");
      DEBUGMSGTL(("winExtDLL", "malloc failed registering handler for winExtDLL"));
      return (-1);
    }
    else {
      DEBUGMSGTL(("winExtDLL", "handler registered\n"));
    }
    
    // Register handler with Net-SNMP
    netsnmp_register_handler(winExtensionAgent[winExtensionAgent_num].my_handler);

    // Check for additional supported views and register them with the same handler
    if (winExtensionAgent[winExtensionAgent_num].xSnmpExtensionInitEx) {
      DEBUGMSGTL(("winExtDLL", "xSnmpExtensionInitEx found\n"));

      winExtensionAgent_num++;

      while (1) { // Loop looking for more supported views
        // Store DLL name and functions in winExtensionAgent array  
        strncpy(winExtensionAgent[winExtensionAgent_num].dll_name, dll_name, SZBUF_DLLNAME_MAX-1);
        winExtensionAgent[winExtensionAgent_num].xSnmpExtensionInit = xSnmpExtensionInit;
        winExtensionAgent[winExtensionAgent_num].xSnmpExtensionInitEx = xSnmpExtensionInitEx;
        winExtensionAgent[winExtensionAgent_num].xSnmpExtensionQuery = xSnmpExtensionQuery;
        winExtensionAgent[winExtensionAgent_num].xSnmpExtensionQueryEx = xSnmpExtensionQueryEx;
        winExtensionAgent[winExtensionAgent_num].xSnmpExtensionTrap = xSnmpExtensionTrap;     
        winExtensionAgent[winExtensionAgent_num].subagentTrapEvent = NULL;
        // Get extra supported view
        result = xSnmpExtensionInitEx(&pSupportedView);

        if (! (result)) {
          winExtensionAgent_num--;
          break;
        }

        DEBUGMSGTL(("winExtDLL", "result of xSnmpExtensionInitEx: %d\n",result));
      
        DEBUGMSGTL(("winExtDLL", "Supported view: "));
        DEBUGMSGWINOID(("winExtDLL", &pSupportedView));
        DEBUGMSG(("winExtDLL", "\n"));
        
        // Convert OID from Windows 'supported view' to Net-SNMP
        for (i = 0; i < (pSupportedView.idLength > MAX_OID_LEN?MAX_OID_LEN:pSupportedView.idLength); i++) {
          name[i] = (oid)pSupportedView.ids[i];
        }
        length = i;
  
        // Store supported view in Net-SNMP format
        memcpy(winExtensionAgent[winExtensionAgent_num].name, name, sizeof(name));
        winExtensionAgent[winExtensionAgent_num].name_length = length;

        DEBUGMSGTL(("winExtDLL", "Windows OID converted to Net-SNMP: "));
        DEBUGMSGOID(("winExtDLL", name, length));
        DEBUGMSG(("winExtDLL", "\n"));
  
        // Store supported view in Windows format
        SnmpUtilOidCpy(&winExtensionAgent[winExtensionAgent_num].pSupportedView,&pSupportedView);
  
        // Create handler registration
        winExtensionAgent[winExtensionAgent_num].my_handler = netsnmp_create_handler_registration("winExtDLL",
            var_winExtDLL,
            name,
            length,
            HANDLER_CAN_RWRITE);
        
        if (!winExtensionAgent[winExtensionAgent_num].my_handler) {
          snmp_log(LOG_ERR,
              "malloc failed registering handler for winExtDLL");
          DEBUGMSGTL(("winExtDLL", "malloc failed registering handler for winExtDLL"));
          return (-1);
        }
        else {
          DEBUGMSGTL(("winExtDLL", "handler registered\n"));
        }
        
        // Register handler with Net-SNMP
        netsnmp_register_handler(winExtensionAgent[winExtensionAgent_num].my_handler);

        winExtensionAgent_num++;
      } 
    }
    winExtensionAgent_num++;
  }


  winExtensionAgent_index = winExtensionAgent_num-1;             // Array index
  DEBUGMSGTL(("winExtDLL", "winExtensionAgent_index: %d\n",winExtensionAgent_index));

  /* Reverse sort array of winExtensionAgents */
  i = sizeof(winExtensionAgent) / sizeof(winExtensionAgents);
  //DEBUGMSGTL(("winExtDLL", "Sorting...\n"));
  for (iter=0; iter < i-1; iter++) {
    for (indx=0; indx < i-1; indx++) {
      if (snmp_oidtree_compare(winExtensionAgent[indx].name, winExtensionAgent[indx].name_length,
            winExtensionAgent[indx+1].name, winExtensionAgent[indx+1].name_length) < 0) {
        winExtensionAgent_temp = winExtensionAgent[indx];
        winExtensionAgent[indx] = winExtensionAgent[indx+1];
        winExtensionAgent[indx+1] = winExtensionAgent_temp;
      }
    }
  }
  DEBUGMSGTL(("winExtDLL", "Dumping sorted Windows extension OIDs\n"));
  for (i=0; winExtensionAgent[i].xSnmpExtensionInit; i++) {
    DEBUGMSGTL(("winExtDLL", "DLL name: %s, view: ",winExtensionAgent[i].dll_name));
    DEBUGMSGOID(("winExtDLL", winExtensionAgent[i].name, winExtensionAgent[i].name_length));
    DEBUGMSG(("winExtDLL", "\n"));
  }

  DEBUGMSGTL(("winExtDLL", "Number of subagentTrapEvents: %d\n",subagentTrapEvents_index));

  if (subagentTrapEvents_index) {
    
    DEBUGMSGTL(("winExtDLL", "Setting alarm check for subagent trap events every 5 seconds\n"));
    snmp_alarm_register(5,      /* seconds */
        SA_REPEAT,              /* repeat (every x seconds). */
        subagentTrapCheck,      /* our callback */
        NULL                    /* no callback data needed */
        );    
  }
}

int
var_winExtDLL(netsnmp_mib_handler *handler,
              netsnmp_handler_registration *reginfo,
              netsnmp_agent_request_info *reqinfo,
              netsnmp_request_info *requests)
{

    netsnmp_request_info *request = requests;
    netsnmp_variable_list *var;
    oid             oid_requested[MAX_OID_LEN];
    size_t          oid_requested_length;
    
    static char     ret_szbuf_temp[SZBUF_MAX];          // Holder for return strings
    static oid      ret_oid[MAX_OID_LEN];               // Holder for return OIDs
    static size_t   ret_oid_length = 0;                 // Holder for return OIDs
    static long     ret_long;                           // Holder for all other returns

    static char     set_szbuf_temp[SZBUF_MAX];          // Holder for set strings    
    static oid      set_oid[MAX_OID_LEN];               // Holder for set OIDs
    static size_t   set_oid_length = 0;                 // Holder for set OIDs
    oid             *temp_oid;
    size_t          temp_oid_length;
    
    static int      temp;
    static u_long   accesses = 7;
    u_char          netsnmp_ASN_type;
    u_char          windows_ASN_type;

    char          *stringtemp;

    // WinSNMP variables:
    BOOL result;   
    SnmpVarBind *mySnmpVarBind;
    AsnInteger32 pErrorStatus;
    AsnInteger32 pErrorIndex;
    SnmpVarBindList pVarBindList;  
    int i=0;
    int j=0;

    DWORD (WINAPI *xSnmpExtensionQuery)(BYTE, SnmpVarBindList* ,AsnInteger32* ,AsnInteger32*);
    DWORD (WINAPI *xSnmpExtensionQueryEx)(DWORD, DWORD, SnmpVarBindList*, AsnOctetString*, AsnInteger32*, AsnInteger32*);
    DWORD (WINAPI *next_xSnmpExtensionQuery)(BYTE, SnmpVarBindList* ,AsnInteger32* ,AsnInteger32*);
    DWORD (WINAPI *next_xSnmpExtensionQueryEx)(DWORD, DWORD, SnmpVarBindList*, AsnOctetString*, AsnInteger32*, AsnInteger32*);
    AsnObjectIdentifier winExtensionAgent_pSupportedView;               // Support view of this extension agent
    oid winExtensionAgent_name[MAX_OID_LEN];                            // Support view of this extension agent
    size_t winExtensionAgent_name_length;                               // Support view of this extension agent

    int match_winExtensionAgent_index = -1;

    DEBUGMSGTL(("winExtDLL", "-----------------------------------------\n"));
    DEBUGMSGTL(("winExtDLL", "var_winExtDLL handler starting, mode = %d\n",
                reqinfo->mode));
    
    switch (reqinfo->mode) {
    case MODE_GET:
    case MODE_GETNEXT:

      if (reqinfo->mode == MODE_GET)
        DEBUGMSGTL(("winExtDLL", "GET requested\n"));
      else if (reqinfo->mode == MODE_GETNEXT)
        DEBUGMSGTL(("winExtDLL", "GETNEXT requested\n"));
      
      for (request = requests; request; request=request->next) {
        
        var = request->requestvb;

        // Make copy of requested OID
        for (i = 0; i < (var->name_length > MAX_OID_LEN? MAX_OID_LEN: var->name_length); i++) {
          oid_requested[i] = var->name[i];
        }
        oid_requested_length = i;
        
        DEBUGMSGTL(("winExtDLL", "Requested: "));
        DEBUGMSGOID(("winExtDLL", oid_requested, oid_requested_length));
        DEBUGMSG(("winExtDLL", "\n"));       

        DEBUGMSGTL(("winExtDLL", "Var type requested: %d\n",var->type));

        /* Loop through all the winExtensionAgent's looking for a matching handler */
        xSnmpExtensionQuery = NULL;
        xSnmpExtensionQueryEx = NULL;
        next_xSnmpExtensionQuery = NULL;
        next_xSnmpExtensionQueryEx = NULL;

        DEBUGMSGTL(("winExtDLL", "Looping through all the winExtensionAgent's looking for a matching handler (exact).\n"));
        // Search starting with lowest so a walk of .1.3 starts with the lowest extension
        for (i = winExtensionAgent_index; winExtensionAgent[i].xSnmpExtensionInit && i >= 0; i--) {          

          /*DEBUGMSGTL(("winExtDLL", "Comparing: "));
          DEBUGMSGOID(("winExtDLL", var->name, var->name_length));
          DEBUGMSG(("winExtDLL", "\n"));       

          DEBUGMSGTL(("winExtDLL", "to:        "));
          DEBUGMSGOID(("winExtDLL", winExtensionAgent[i].name, winExtensionAgent[i].name_length));
          DEBUGMSG(("winExtDLL", "\n"));*/

          if (snmp_oidtree_compare(var->name, var->name_length, winExtensionAgent[i].name, 
                winExtensionAgent[i].name_length) == 0) {

            DEBUGMSGTL(("winExtDLL", "Found exact match: "));
            DEBUGMSGOID(("winExtDLL", winExtensionAgent[i].name, winExtensionAgent[i].name_length));
            DEBUGMSG(("winExtDLL", "\n"));
            match_winExtensionAgent_index = i;
            //DEBUGMSGTL(("winExtDLL", "Index: %d\n",match_winExtensionAgent_index));
            break;
          }
        }

        if (match_winExtensionAgent_index == -1) {
          DEBUGMSGTL(("winExtDLL", "Looping through all the winExtensionAgent's looking for a matching handler (next best).\n"));
          for (i=0; winExtensionAgent[i].xSnmpExtensionInit && i < MAX_WINEXT_DLLS; i++) {
            
            /*DEBUGMSGTL(("winExtDLL", "Comparing: "));
            DEBUGMSGOID(("winExtDLL", var->name, var->name_length));
            DEBUGMSG(("winExtDLL", "\n"));       
            
            DEBUGMSGTL(("winExtDLL", "to:        "));
            DEBUGMSGOID(("winExtDLL", winExtensionAgent[i].name, winExtensionAgent[i].name_length));
            DEBUGMSG(("winExtDLL", "\n"));       
            DEBUGMSGTL(("winExtDLL", "and:       "));
            DEBUGMSGOID(("winExtDLL", winExtensionAgent[i+1].name, winExtensionAgent[i+1].name_length));
            DEBUGMSG(("winExtDLL", "\n"));       */

            if (snmp_oidtree_compare(var->name, var->name_length, winExtensionAgent[i].name, 
                  winExtensionAgent[i].name_length) <= 0 &&
                snmp_oidtree_compare(var->name, var->name_length, winExtensionAgent[i+1].name, 
                  winExtensionAgent[i+1].name_length) >= 0                      // FIXME:  Checking past the last extension?
               ) {
              DEBUGMSGTL(("winExtDLL", "Found best match: "));
              DEBUGMSGOID(("winExtDLL", winExtensionAgent[i].name, winExtensionAgent[i].name_length));
              DEBUGMSG(("winExtDLL", "\n"));
              match_winExtensionAgent_index = i;
              //DEBUGMSGTL(("winExtDLL", "Index: %d\n",match_winExtensionAgent_index));
              break;
            }
          }
        }

        //DEBUGMSG(("winExtDLL", "Index: %d\n",match_winExtensionAgent_index));

        if (match_winExtensionAgent_index == -1) {
          DEBUGMSGTL(("winExtDLL","Could not find a handler for the requested OID.  This should never happen!!\n"));
          return SNMP_ERR_GENERR;
        }

        // Make copy of current extension's OID
        for (j = 0; j < (winExtensionAgent[i].name_length > MAX_OID_LEN? MAX_OID_LEN: winExtensionAgent[i].name_length); j++) {
          winExtensionAgent_name[j] = winExtensionAgent[i].name[j];
        }
        winExtensionAgent_name_length = j;

        xSnmpExtensionQuery = winExtensionAgent[i].xSnmpExtensionQuery;
        xSnmpExtensionQueryEx = winExtensionAgent[i].xSnmpExtensionQueryEx;
        SnmpUtilOidCpy(&winExtensionAgent_pSupportedView, &winExtensionAgent[i].pSupportedView);

        if (! (xSnmpExtensionQuery || xSnmpExtensionQueryEx)) {
          if (reqinfo->mode == MODE_GET) {
            DEBUGMSGTL(("winExtDLL","Could not find a handler for the requested OID.  This should never happen!!\n"));
            return SNMP_ERR_GENERR;
          }
          else {
            DEBUGMSGTL(("winExtDLL","Could not find a handler for the requested OID.  Returning SNMP_ERR_NOSUCHNAME.\n"));
            return SNMP_ERR_NOSUCHNAME;
          }
        }
        
        // Query
        pVarBindList.list = (SnmpVarBind *) SnmpUtilMemAlloc(sizeof (SnmpVarBind));
        if (pVarBindList.list) {

          // Convert OID from Net-SNMP to Windows         
         
          pVarBindList.list->name.ids = (UINT *) SnmpUtilMemAlloc(sizeof (UINT) *var->name_length);
          
          if (pVarBindList.list->name.ids) {
            // Actual copy
            
            for (i = 0; i < var->name_length; i++) {
              pVarBindList.list->name.ids[i] = (UINT)var->name[i];
            }
            pVarBindList.list->name.idLength = i;
            
            DEBUGMSGTL(("winExtDLL", "Windows OID: "));
            DEBUGMSGWINOID(("winExtDLL", pVarBindList.list));
            DEBUGMSG(("winExtDLL", "\n"));
            
          }
          else {
            DEBUGMSGTL(("winExtDLL", "\nyCould not allocate memory for Windows SNMP varbind.\n"));
            return (0);
          }

          //pVarBindList.list = pVarBindList.list;

          pVarBindList.len = 1;          
	}
        else {
          DEBUGMSGTL(("winExtDLL", "Could not allocate memory for Windows SNMP varbind list.\n"));
          return (0);
        }        
		
        if (reqinfo->mode == MODE_GET) {
          DEBUGMSGTL(("winExtDLL", "win: MODE_GET\n"));
          if (xSnmpExtensionQueryEx) {
            DEBUGMSGTL(("winExtDLL", "Calling xSnmpExtensionQueryEx\n"));
            result = xSnmpExtensionQueryEx(SNMP_PDU_GET, 1, &pVarBindList, NULL, &pErrorStatus, &pErrorIndex);
          }
          else { 
            DEBUGMSGTL(("winExtDLL", "Calling xSnmpExtensionQuery\n"));
            result = xSnmpExtensionQuery(SNMP_PDU_GET, &pVarBindList, &pErrorStatus, &pErrorIndex);
          } 
        }
        else if (reqinfo->mode == MODE_GETNEXT) {
          DEBUGMSGTL(("winExtDLL", "win: MODE_GETNEXT -\n"));
          if (xSnmpExtensionQueryEx) {
            DEBUGMSGTL(("winExtDLL", "Calling xSnmpExtensionQueryEx\n"));
            result = xSnmpExtensionQueryEx(SNMP_PDU_GETNEXT, 1, &pVarBindList, NULL, &pErrorStatus, &pErrorIndex);
          }
          else { 
            DEBUGMSGTL(("winExtDLL", "Calling xSnmpExtensionQuery\n"));
            result = xSnmpExtensionQuery(SNMP_PDU_GETNEXT, &pVarBindList, &pErrorStatus, &pErrorIndex);
          } 

          DEBUGMSGTL(("winExtDLL", "Windows OID returned from xSnmpExtensionQuery(Ex): "));
          DEBUGMSGWINOID(("winExtDLL", pVarBindList.list));
          DEBUGMSG(("winExtDLL", "\n"));

          // Convert OID from Windows to Net-SNMP so Net-SNMP has the new 'next' OID
          // FIXME:  Do we need to realloc var->name or is is MAX_OID_LEN?
          for (i = 0; i < (pVarBindList.list->name.idLength > MAX_OID_LEN?MAX_OID_LEN:pVarBindList.list->name.idLength); i++) {
            var->name[i] = (oid)pVarBindList.list->name.ids[i];
          }
          var->name_length = i;

          DEBUGMSGTL(("winExtDLL", "Comparing: "));
          DEBUGMSGOID(("winExtDLL", var->name, var->name_length));
          DEBUGMSG(("winExtDLL", "\n"));       

          DEBUGMSGTL(("winExtDLL", "to:        "));
          DEBUGMSGOID(("winExtDLL", winExtensionAgent_name, winExtensionAgent_name_length));
          DEBUGMSG(("winExtDLL", "\n"));       
          

          // If the OID we got back is less than or equal to what we requested, increment the current supported view
          // and send it back instead.
          if (snmp_oid_compare(var->name, var->name_length, oid_requested, oid_requested_length) <= 0) {
            DEBUGMSGTL(("winExtDLL", "xSnmpExtensionQuery(Ex) returned an OID less than or equal to what we requested\n"));
            DEBUGMSGTL(("winExtDLL", " Setting return OID to be equal to current supported view, plus one\n"));

            // Set var->name to be equal to current supported view, plus one
            for (i = 0; i < (winExtensionAgent_name_length > MAX_OID_LEN? MAX_OID_LEN: winExtensionAgent_name_length); i++) {
              var->name[i] = winExtensionAgent_name[i];
            }
            var->name_length = i;
            var->name[i-1]++;

            //return SNMP_ERR_NOSUCHNAME;
            
          }

          // If the OID we got back is outside our view, increment the current supported view and send it back instead.
          // This is for Windows extension agents that support multiple views.  We want Net-SNMP to decide if we should be
          // called for this OID, not the extension agent, just in case there is a Net-SNMP agent in between.
          else if (snmp_oidtree_compare(var->name, var->name_length, winExtensionAgent_name, winExtensionAgent_name_length) > 0) {
            DEBUGMSGTL(("winExtDLL", "xSnmpExtensionQuery(Ex) returned an OID outside our view\n"));
            DEBUGMSGTL(("winExtDLL", " Setting return OID to be equal to current supported view, plus one\n"));

            // Set var->name to be equal to current supported view, plus one
            for (i = 0; i < (winExtensionAgent_name_length > MAX_OID_LEN? MAX_OID_LEN: winExtensionAgent_name_length); i++) {
              var->name[i] = winExtensionAgent_name[i];
            }
            var->name_length = i;
            var->name[i-1]++;

            //return SNMP_ERR_NOSUCHNAME;
            
          }
       }       
        DEBUGMSGTL(("winExtDLL", "OID being sent back to Net-SNMP: "));
        DEBUGMSGOID(("winExtDLL", var->name, var->name_length));
        DEBUGMSG(("winExtDLL", "\n"));

        DEBUGMSGTL(("winExtDLL", "win: Result of xSnmpExtensionQuery(Ex): %d\n",result));
        
        DEBUGMSGTL(("winExtDLL", "win: Error status of xSnmpExtensionQuery(Ex): %d\n",pErrorStatus));

        DEBUGMSGTL(("winExtDLL", "win: asnType: %d\n",pVarBindList.list->value.asnType));
      
        // Set Net-SNMP ASN type based on closest match to Windows ASN type
        switch (pVarBindList.list->value.asnType) {
          case MS_ASN_OCTETSTRING:
            netsnmp_ASN_type = ASN_OCTET_STR;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_OCTETSTRING = ASN_OCTET_STR\n"));
            break;
          case MS_ASN_INTEGER:          // And MS_ASN_INTEGER32
            netsnmp_ASN_type = ASN_INTEGER;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_INTEGER = ASN_INTEGER\n"));
            break;
          case MS_ASN_UNSIGNED32:       // SNMP v2
            netsnmp_ASN_type = ASN_UNSIGNED;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_UNSIGNED32 = ASN_UNSIGNED\n"));
            break;
          case MS_ASN_COUNTER64:       // SNMP v2
            netsnmp_ASN_type = ASN_COUNTER64;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_COUNTER64 = ASN_COUNTER64\n"));
            break;
          case MS_ASN_BITS:
            netsnmp_ASN_type = ASN_BIT_STR;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_BITS = ASN_BIT_STR\n"));
            break;
          case MS_ASN_OBJECTIDENTIFIER:
            netsnmp_ASN_type = ASN_OBJECT_ID;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_OBJECTIDENTIFIER = ASN_OBJECT_ID\n"));
            break;
          case MS_ASN_SEQUENCE:
            netsnmp_ASN_type = ASN_SEQUENCE;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_SEQUENCE = ASN_SEQUENCE\n"));
            break;
          case MS_ASN_IPADDRESS:
            netsnmp_ASN_type = ASN_IPADDRESS;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_IPADDRESS = ASN_IPADDRESS\n"));
            break;
          case MS_ASN_COUNTER32:
            netsnmp_ASN_type = ASN_COUNTER;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_COUNTER32 = ASN_COUNTER\n"));
            break;
          case MS_ASN_GAUGE32:
            netsnmp_ASN_type = ASN_GAUGE;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_GAUGE32 = ASN_GAUGE\n"));
            break;
          case MS_ASN_TIMETICKS:
            netsnmp_ASN_type = ASN_TIMETICKS;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_TIMETICKS = ASN_TIMETICKS\n"));
            break;
          case MS_ASN_OPAQUE:
            netsnmp_ASN_type = ASN_OPAQUE;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_OPAQUE = ASN_OPAQUE\n"));
            break;
          default:
            netsnmp_ASN_type = ASN_INTEGER;
            DEBUGMSGTL(("winExtDLL", "unknown MS ASN type.  Defaulting to ASN_INTEGER\n"));
            break;
        }

        DEBUGMSGTL(("winExtDLL", "Net-SNMP object type for returned value: %d\n",netsnmp_ASN_type));

        switch (pVarBindList.list->value.asnType) {
          case MS_ASN_IPADDRESS:
          case MS_ASN_OCTETSTRING:           
            
            strncpy(ret_szbuf_temp, pVarBindList.list->value.asnValue.string.stream, (pVarBindList.list->value.asnValue.string.length > 
                  SZBUF_MAX?SZBUF_MAX:pVarBindList.list->value.asnValue.string.length));
           
            if (pVarBindList.list->value.asnValue.string.length < SZBUF_MAX) 
              ret_szbuf_temp[pVarBindList.list->value.asnValue.string.length] = '\0';
            else
              ret_szbuf_temp[SZBUF_MAX-1] = '\0';

            // Printing strings that have a comma in them via DEBUGMSGTL doesn't work..
            DEBUGMSGTL(("winExtDLL", "Note:  Sometimes junk is outputted here even though the string is fine.  Problem with DEBUGMSGTL?\n",ret_szbuf_temp));
            DEBUGMSGTL(("winExtDLL", "win: String: %s\n",ret_szbuf_temp));
            DEBUGMSGTL(("winExtDLL", "win: length of string response: %d\n",strlen(ret_szbuf_temp)));
            
            snmp_set_var_typed_value(var, netsnmp_ASN_type,
                ret_szbuf_temp,
                strlen(ret_szbuf_temp));
            //return SNMP_ERR_NOERROR;           
            break;

          case MS_ASN_INTEGER:          // And MS_ASN_INTEGER32
          case MS_ASN_UNSIGNED32:
          case MS_ASN_COUNTER64:
          case MS_ASN_BITS:
          case MS_ASN_SEQUENCE:
          case MS_ASN_COUNTER32:
          case MS_ASN_GAUGE32:
          case MS_ASN_TIMETICKS:
          case MS_ASN_OPAQUE:

            DEBUGMSGTL(("winExtDLL", "win: Long: %ld\n",pVarBindList.list->value.asnValue.number));

            ret_long = pVarBindList.list->value.asnValue.number;

            // Return results
            snmp_set_var_typed_value(var, netsnmp_ASN_type,
                &ret_long,
                sizeof(ret_long));
            //return SNMP_ERR_NOERROR;           
            break;

          case MS_ASN_OBJECTIDENTIFIER:
            // Convert OID to Net-SNMP

            DEBUGMSGTL(("winExtDLL", "Returned OID: "));
            DEBUGMSGWINOID(("winExtDLL", &pVarBindList.list->value.asnValue.object));
            DEBUGMSG(("winExtDLL", "\n"));
           
            // Convert OID from Windows to Net-SNMP
            for (i = 0; i < (pVarBindList.list->value.asnValue.object.idLength > MAX_OID_LEN?MAX_OID_LEN:
                  pVarBindList.list->value.asnValue.object.idLength); i++) {
              ret_oid[i] = (oid)pVarBindList.list->value.asnValue.object.ids[i];
            }
            ret_oid_length = i;
           
            DEBUGMSGTL(("winExtDLL", "Windows OID converted to Net-SNMP: "));
            DEBUGMSGOID(("winExtDLL", ret_oid, ret_oid_length));
            DEBUGMSG(("winExtDLL", "\n"));
                      
            snmp_set_var_typed_value(var, netsnmp_ASN_type,
                ret_oid,
                ret_oid_length  * sizeof(oid));
            //return SNMP_ERR_NOERROR;           
            
            break;
            
          default:
            // The Windows agent didn't return data so set values to NULL
            // FIXME:  We never get here.  We set it to INTEGER above..
            snmp_set_var_typed_value(var, NULL,
                NULL,
                NULL);
            break;
        }
        if (&pVarBindList)
          SnmpUtilVarBindListFree(&pVarBindList);
      }  
      break;

    case MODE_SET_RESERVE1:     
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:

      DEBUGMSGTL(("winExtDLL", "SET requested\n"));
      
      for (request = requests; request; request=request->next) {

        var = request->requestvb;
        
        DEBUGMSGTL(("winExtDLL", "requested: "));
        DEBUGMSGOID(("winExtDLL", var->name, var->name_length));
        DEBUGMSG(("winExtDLL", "\n"));       

        DEBUGMSGTL(("winExtDLL", "Var type requested: %d\n",var->type));

        /* Loop through all the winExtensionAgent's looking for a matching handler */
        xSnmpExtensionQuery = NULL;
        xSnmpExtensionQueryEx = NULL;
        for (i=0; winExtensionAgent[i].xSnmpExtensionInit && i < MAX_WINEXT_DLLS; i++) {
          DEBUGMSGTL(("winExtDLL", "Looping through all the winExtensionAgent's looking for a matching handler.\n"));
          
          if (snmp_oidtree_compare(var->name, var->name_length, winExtensionAgent[i].name, 
                winExtensionAgent[i].name_length) >= 0) {
            DEBUGMSGTL(("winExtDLL", "Found match: "));
            DEBUGMSGOID(("winExtDLL", winExtensionAgent[i].name, winExtensionAgent[i].name_length));
            DEBUGMSG(("winExtDLL", "\n"));
            xSnmpExtensionQuery = winExtensionAgent[i].xSnmpExtensionQuery;
            xSnmpExtensionQueryEx = winExtensionAgent[i].xSnmpExtensionQueryEx;
            break;
          }
        }

        if (! (xSnmpExtensionQuery || xSnmpExtensionQueryEx)) {
          DEBUGMSGTL(("winExtDLL","Could not find a handler for the requested OID.  This should never happen!!\n"));
          return SNMP_ERR_GENERR;
        }

        // Set Windows ASN type based on closest match to Net-SNMP ASN type
        switch (var->type) {
          case ASN_OCTET_STR:
            windows_ASN_type = MS_ASN_OCTETSTRING;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_OCTETSTRING = ASN_OCTET_STR\n"));
            break;
          case ASN_INTEGER:          // And MS_ASN_INTEGER32
            windows_ASN_type = MS_ASN_INTEGER;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_INTEGER = ASN_INTEGER\n"));
            break;
          case ASN_UNSIGNED:
            windows_ASN_type = MS_ASN_UNSIGNED32;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_UNSIGNED32 = ASN_UNSIGNED\n"));
            break;
          case ASN_COUNTER64:
            windows_ASN_type = MS_ASN_COUNTER64;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_COUNTER64 = ASN_COUNTER64\n"));
            break;
          case ASN_BIT_STR:
            windows_ASN_type = MS_ASN_BITS;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_BITS = ASN_BIT_STR\n"));
            break;
          case ASN_OBJECT_ID:
            windows_ASN_type = MS_ASN_OBJECTIDENTIFIER;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_OBJECTIDENTIFIER = ASN_OBJECT_ID\n"));
            break;
          case ASN_SEQUENCE:
            windows_ASN_type = MS_ASN_SEQUENCE;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_SEQUENCE = ASN_SEQUENCE\n"));
            break;
          case ASN_IPADDRESS:
            windows_ASN_type = MS_ASN_IPADDRESS;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_IPADDRESS = ASN_IPADDRESS\n"));
            break;
          case ASN_COUNTER:
            windows_ASN_type = MS_ASN_COUNTER32;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_COUNTER32 = ASN_COUNTER\n"));
            break;
//          case ASN_GAUGE:                     // Same as UNSIGNED
//            windows_ASN_type = MS_ASN_GAUGE32;
//            DEBUGMSGTL(("winExtDLL", "MS_ASN_GAUGE32 = ASN_GAUGE\n"));
//          break;
          case ASN_TIMETICKS:
            windows_ASN_type = MS_ASN_TIMETICKS;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_TIMETICKS = ASN_TIMETICKS\n"));
            break;
          case ASN_OPAQUE:
            windows_ASN_type = MS_ASN_OPAQUE;
            DEBUGMSGTL(("winExtDLL", "MS_ASN_OPAQUE = ASN_OPAQUE\n"));
            break;
          default:
            windows_ASN_type = MS_ASN_INTEGER;
            DEBUGMSGTL(("winExtDLL", "unknown MS ASN type.  Defaulting to ASN_INTEGER\n"));
            break;
        }

        DEBUGMSGTL(("winExtDLL", "Net-SNMP object type for returned value: %d\n",windows_ASN_type));

        // Query
        pVarBindList.list = (SnmpVarBind *) SnmpUtilMemAlloc(sizeof (SnmpVarBind));
        if (pVarBindList.list) {
          // Convert OID from Net-SNMP to Windows         
          pVarBindList.list->name.ids = (UINT *) SnmpUtilMemAlloc(sizeof (UINT) *var->name_length);

          if (pVarBindList.list->name.ids) {
            // Actual copy
            for (i = 0; i < var->name_length; i++) {
              pVarBindList.list->name.ids[i] = (UINT)var->name[i];
            }
            pVarBindList.list->name.idLength = i;

            // Print OID
            DEBUGMSGTL(("winExtDLL","Windows OID length: %d\n",pVarBindList.list->name.idLength));
            DEBUGMSGTL(("winExtDLL","Windows OID: "));
            DEBUGMSGWINOID(("winExtDLL", pVarBindList.list));
            DEBUGMSG(("winExtDLL", "\n"));
          }
          else {
            DEBUGMSGTL(("winExtDLL", "\nyCould not allocate memory for Windows SNMP varbind.\n"));
            return (0);
          }
          //pVarBindList.list = pVarBindList.list;
          pVarBindList.len = 1;          
	}
        else {
          DEBUGMSGTL(("winExtDLL", "\nyCould not allocate memory for Windows SNMP varbind list.\n"));
          return (0);
        }
		        
        // Set Windows ASN type
        pVarBindList.list->value.asnType = windows_ASN_type;
          
        switch (var->type) {
          case ASN_IPADDRESS:
          case ASN_OCTET_STR:            

            strncpy(set_szbuf_temp, var->val.string, (var->val_len > SZBUF_MAX?SZBUF_MAX:var->val_len));

            // Make sure string is terminated
            set_szbuf_temp[var->val_len > SZBUF_MAX?SZBUF_MAX:var->val_len] = '\0';

            DEBUGMSGTL(("winExtDLL", "String to write: %s\n",set_szbuf_temp));
            DEBUGMSGTL(("winExtDLL", "Length of string to write: %d\n",strlen(set_szbuf_temp)));

            pVarBindList.list->value.asnValue.string.stream = set_szbuf_temp;
            pVarBindList.list->value.asnValue.string.length = strlen(set_szbuf_temp);
            pVarBindList.list->value.asnValue.string.dynamic = 0;
            
            break;

          case ASN_INTEGER:          // And MS_ASN_INTEGER32
          case ASN_UNSIGNED:
          case ASN_COUNTER64:
          case ASN_BIT_STR:
          case ASN_SEQUENCE:
          case ASN_COUNTER:
          case ASN_TIMETICKS:
          case ASN_OPAQUE:        
            
            pVarBindList.list->value.asnValue.number = *(var->val.integer);
            break;
              
          case ASN_OBJECT_ID:
            
            // Convert OID from Net-SNMP to Windows
            temp_oid = var->val.objid;
            temp_oid_length = var->val_len / sizeof(oid);           
            
            DEBUGMSGTL(("winExtDLL","Sizeof var->val.objid: %d\n", temp_oid_length));

            DEBUGMSGTL(("winExtDLL","OID: from user "));
            DEBUGMSGOID(("winExtDLL", temp_oid, temp_oid_length));
            DEBUGMSG(("winExtDLL","\n"));
            
            mySnmpVarBind->name.ids = (UINT *) SnmpUtilMemAlloc(sizeof (UINT) * temp_oid_length);

            if (mySnmpVarBind->name.ids) {
              // Actual copy
              for (i = 0; i < temp_oid_length; i++) {
                mySnmpVarBind->name.ids[i] = (UINT)temp_oid[i];
              }
              mySnmpVarBind->name.idLength = i;
              
              // Print OID
              DEBUGMSGTL(("winExtDLL","Windows OID length: %d\n",mySnmpVarBind->name.idLength));
              DEBUGMSGTL(("winExtDLL","Windows OID: "));
              DEBUGMSGWINOID(("winExtDLL", mySnmpVarBind));
              DEBUGMSG(("winExtDLL", "\n"));
            }
            else {
              DEBUGMSGTL(("winExtDLL", "\nyCould not allocate memory for Windows SNMP varbind.\n"));
              return SNMP_ERR_GENERR;
            }
            SnmpUtilOidCpy(&pVarBindList.list->value.asnValue.object, &mySnmpVarBind->name);
            //pVarBindList.list->value.asnValue.object = mySnmpVarBind->name;
            if (mySnmpVarBind)
              SnmpUtilVarBindFree(mySnmpVarBind);
            
          default:
            break;      
        }  

        if (xSnmpExtensionQueryEx) {
          DEBUGMSGTL(("winExtDLL", "Calling xSnmpExtensionQueryEx\n"));
          result = xSnmpExtensionQueryEx(SNMP_PDU_SET, 1, &pVarBindList, NULL, &pErrorStatus, &pErrorIndex);
        }
        else { 
          DEBUGMSGTL(("winExtDLL", "Calling xSnmpExtensionQuery\n"));
          result = xSnmpExtensionQuery(SNMP_PDU_SET, &pVarBindList, &pErrorStatus, &pErrorIndex);
        }

        DEBUGMSGTL(("winExtDLL", "win: Result of xSnmpExtensionQuery: %d\n",result));        
        DEBUGMSGTL(("winExtDLL", "win: Error status of xSnmpExtensionQuery: %d\n",pErrorStatus));
        DEBUGMSGTL(("winExtDLL", "win: asnType: %d\n",pVarBindList.list->value.asnType));

        if (result == 0) {
          DEBUGMSGTL(("winExtDLL", "\nyxWindows SnmpExtensionQuery failure.\n"));
          return SNMP_ERR_GENERR;
        }
        
        if (&pVarBindList)
          SnmpUtilVarBindListFree(&pVarBindList);

        if (pErrorStatus) {
          switch (pErrorStatus) {
            case SNMP_ERRORSTATUS_INCONSISTENTNAME:
              return SNMP_ERR_GENERR;
            default:
              return pErrorStatus;
              break;
          }
        }
      }
      break;     

    case MODE_SET_UNDO:
    case MODE_SET_COMMIT:
    case MODE_SET_FREE:

      break;
      
    default:
        snmp_log(LOG_WARNING, "unsupported mode for winExtDLL called (%d)\n",
                               reqinfo->mode);
        return SNMP_ERR_NOERROR;
    }

    return SNMP_ERR_NOERROR;
}

void winExtDLL_free_config_winExtDLL(void) {
}

void read_ExtensionAgents_list() {
  HKEY          hKey; 
  unsigned char * key_value = NULL;
  DWORD         key_value_size = 0;
  DWORD         key_value_type = 0;
  DWORD         valueSize = MAX_VALUE_NAME; 
  int           i;
  TCHAR         valueName[MAX_VALUE_NAME];
  TCHAR         valueName2[MAX_VALUE_NAME];
  DWORD         retCode;
  
  DEBUGMSGTL(("winExtDLL", "read_ExtensionAgents_list called\n"));

  /* The Windows SNMP service stores the list of extension agents to be loaded in the
   * registry under HKLM\SYSTEM\CurrentControlSet\Services\SNMP\Parameters\ExtensionAgents.
   * This list contains a list of other keys that contain the actual file path to the DLL.
   */

  /* Open SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters\\ExtensionAgent */
  retCode = RegOpenKeyExA(
      HKEY_LOCAL_MACHINE, 
      "SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters\\ExtensionAgents", 
      0, 
      KEY_QUERY_VALUE, 
      &hKey);
  
  if (retCode == ERROR_SUCCESS) {
    /* Enumerate list of extension agents.  This is a list of other keys that contain the
     * actual filename of the extension agent.  */
    for (i=0; retCode==ERROR_SUCCESS; i++) 
    { 
      valueSize = MAX_VALUE_NAME; 
      valueName[0] = '\0'; 
      retCode = RegEnumValue(
          hKey,
          i,
          valueName, 
          &valueSize, 
          NULL, 
          NULL,
          NULL,
          NULL);
      
      if (retCode == ERROR_SUCCESS ) 
      { 
        /* Get key name that contains the actual filename of the extension agent */
        DEBUGMSGTL(("winExtDLL", "-----------------------------------------\n"));
        DEBUGMSGTL(("winExtDLL", "Registry: (%d) %s\n", i+1, valueName));
        
        key_value_size = MAX_VALUE_NAME;
        if (RegQueryValueExA(
              hKey, 
              valueName, 
              NULL, 
              &key_value_type, 
              valueName2, 
              &key_value_size) == ERROR_SUCCESS) {
        }
        DEBUGMSGTL(("winExtDLL", "key_value: %s\n",valueName2));
        read_ExtensionAgents_list2(valueName2);
        extDLLs_index++;
      }
    }
    if (extDLLs_index)
      extDLLs_index--;
  }
}

void read_ExtensionAgents_list2(const TCHAR *keyName) {
  HKEY          hKey; 
  unsigned char * key_value = NULL;
  DWORD         key_value_size = 0;
  DWORD         key_value_type = 0;
  DWORD         valueSize = MAX_VALUE_NAME; 
  TCHAR         valueName[MAX_VALUE_NAME];
  TCHAR         valueNameExpanded[MAX_VALUE_NAME];
  int           i;
  DWORD         retCode;
  
  DEBUGMSGTL(("winExtDLL", "read_ExtensionAgents_list2 called\n"));
  DEBUGMSGTL(("winExtDLL", "Registry: Opening key %s\n", keyName));

  /* Open extension agent's key */
  retCode = RegOpenKeyExA(
      HKEY_LOCAL_MACHINE, 
      keyName, 
      0, 
      KEY_QUERY_VALUE, 
      &hKey);
  
  if (retCode == ERROR_SUCCESS) {
    /* Read Pathname value */

    DEBUGMSGTL(("winExtDLL", "Registry: Reading value for %s\n", keyName));
       
    key_value_size = MAX_VALUE_NAME;
    retCode = RegQueryValueExA(
        hKey, 
        "Pathname", 
        NULL, 
        &key_value_type, 
        valueName, 
        &key_value_size);
    
    if (retCode == ERROR_SUCCESS) {
      valueName[key_value_size-1] = NULL;               /* Make sure last element is a NULL */        
      DEBUGMSGTL(("winExtDLL", "Extension agent Pathname size: %d\n",key_value_size));
      DEBUGMSGTL(("winExtDLL", "Extension agent Pathname: %s\n",valueName));

      if (ExpandEnvironmentStrings(valueName, valueNameExpanded, MAX_VALUE_NAME)) {
        DEBUGMSGTL(("winExtDLL", "Extension agent Pathname expanded: %s\n",valueNameExpanded));
        if (extDLLs_index < MAX_WINEXT_DLLS) {

          extDLLs[extDLLs_index] = strdup(valueNameExpanded);
          
          if (extDLLs[extDLLs_index]) {
            strcpy(extDLLs[extDLLs_index], valueNameExpanded );
            DEBUGMSGTL(("winExtDLL", "Extension agent Pathname expanded extDLLs: %s\n",extDLLs[extDLLs_index]));
            DEBUGMSGTL(("winExtDLL", "Extension agent Pathname size: %d\n",strlen(extDLLs[extDLLs_index])));
          }
          else {
            DEBUGMSGTL(("winExtDLL", "Could not allocate memory for extDLLs[%d]\n",extDLLs_index));
          }
        }
      }
      else {
        DEBUGMSGTL(("winExtDLL", "ExpandEnvironmentStrings failed\n"));
      }      
    }
  }
}

// Called by alarm to check for traps waiting to be processed.
void subagentTrapCheck() {
  DWORD dwWaitResult;
  BOOL bResult;
  int i;
  netsnmp_variable_list *notification_vars = NULL;

  // Windows SNMP
  AsnObjectIdentifier   pEnterprise;
  AsnInteger            pGenericTrap;
  AsnInteger            pSpecificTrap;
  AsnTimeticks          pTimeStamp;
  SnmpVarBindList       pVariableBindings;
 
  DEBUGMSGTL(("winExtDLL", "subagentTrapMonitor called\n"));

  dwWaitResult = WaitForMultipleObjects(
      subagentTrapEvents_index,
      subagentTrapEvents,
      FALSE,
      0);

  if (! (dwWaitResult) || (dwWaitResult == WAIT_TIMEOUT))
    return;
  
  DEBUGMSGTL(("winExtDLL", "---------------------------------------------\n"));
  DEBUGMSGTL(("winExtDLL", "subagentTrapCheck found a trap event (index: %d)\n",dwWaitResult));
  
  /* Loop through all the winExtensionAgent's looking for a matching handler */
  for (i=0;  winExtensionAgent[i].xSnmpExtensionInit && i < MAX_WINEXT_DLLS; i++) {
    DEBUGMSGTL(("winExtDLL", "Looping through all the winExtensionAgent's looking for a matching trap handler.\n"));

    if (winExtensionAgent[i].subagentTrapEvent == subagentTrapEvents[dwWaitResult]) {
      DEBUGMSGTL(("winExtDLL", "Found match: "));
      DEBUGMSGOID(("winExtDLL", winExtensionAgent[i].name, winExtensionAgent[i].name_length));
      DEBUGMSG(("winExtDLL", "\n"));

      if (winExtensionAgent[i].xSnmpExtensionTrap)
        DEBUGMSGTL(("winExtDLL", "xSnmpExtensionTrap exists for this subagent\n"));
      else {
        DEBUGMSGTL(("winExtDLL", "xSnmpExtensionTrap does NOT exist for this subagent\n"));
        continue;
      }

      pEnterprise.ids = NULL;
      pEnterprise.idLength = 0;
      pGenericTrap = pSpecificTrap = NULL;
      pTimeStamp = 0;
      pVariableBindings.list = NULL;
      pVariableBindings.len = 0;

      DEBUGMSGTL(("winExtDLL", "Calling SnmpExtensionTrap\n"));
      bResult = winExtensionAgent[i].xSnmpExtensionTrap(
          &pEnterprise,
          &pGenericTrap,
          &pSpecificTrap,
          &pTimeStamp,
          &pVariableBindings);
      
      DEBUGMSGTL(("winExtDLL", "result of SnmpExtensionTrap call:  %d\n",bResult));

      
      DEBUGMSGTL(("winExtDLL", "GenericTrap: %d\n",pGenericTrap));
      DEBUGMSGTL(("winExtDLL", "SpecificTrap: %d\n",pSpecificTrap));

      if (pEnterprise.idLength)
        DEBUGMSGTL(("winExtDLL", "pEnterprise is not 0\n"));
      else
        DEBUGMSGTL(("winExtDLL", "pEnterprise is 0\n"));

      // Assume that if enterprise length is >0, it's a real trap and not a cleanup call
      // FIXME:  Not sure if this is correct.  Need to test with agent that sends a non-enterprise trap
      if (pEnterprise.idLength) {
        
        // Send the trap
        send_trap(
            &pEnterprise, 
            &pGenericTrap, 
            &pSpecificTrap, 
            &pTimeStamp,
            &pVariableBindings);
      }

      // Look for more traps from this agent (if result is TRUE there are more traps)
      while(bResult) {
        DEBUGMSGTL(("winExtDLL", "More traps to process.  Calling SnmpExtensionTrap again\n"));

        pEnterprise.ids = NULL;
        pEnterprise.idLength = 0;
        pGenericTrap = pSpecificTrap = NULL;
        pTimeStamp = 0;
        pVariableBindings.list = NULL;
        pVariableBindings.len = 0;

        bResult = winExtensionAgent[i].xSnmpExtensionTrap(
            &pEnterprise,
            &pGenericTrap,
            &pSpecificTrap,
            &pTimeStamp,
            &pVariableBindings);
        
        DEBUGMSGTL(("winExtDLL", "result of SnmpExtensionTrap call:  %d\n",bResult));

        // Assume that if enterprise length is >0, it's a real trap and not a cleanup call
        // FIXME:  Not sure if this is correct.  Need to test with agent that sends a non-enterprise trap
        if (pEnterprise.idLength) {
          
          DEBUGMSGTL(("winExtDLL", "GenericTrap: %d\n",pGenericTrap));
          DEBUGMSGTL(("winExtDLL", "SpecificTrap: %d\n",pSpecificTrap));

          if (pEnterprise.idLength)
            DEBUGMSGTL(("winExtDLL", "pEnterprise is not 0\n"));
          else
            DEBUGMSGTL(("winExtDLL", "pEnterprise is 0\n"));
          
          // Send the trap
          send_trap(
              &pEnterprise, 
              &pGenericTrap, 
              &pSpecificTrap,
              &pTimeStamp,
              &pVariableBindings);
        }
        SnmpUtilVarBindListFree(&pVariableBindings);
      }
      break;
    }
  }

  // Events should be auto-reset, but just in case..
  ResetEvent(dwWaitResult);

  return 1;
}


void send_trap(
    AsnObjectIdentifier *pEnterprise, 
    AsnInteger *pGenericTrap, 
    AsnInteger *pSpecificTrap, 
    AsnTimeticks *pTimeStamp,
    SnmpVarBindList *pVariableBindings) {

  int           i, j;
  SnmpVarBind   mySnmpVarBind;
  oid           my_oid[MAX_OID_LEN];               // Holder for pVariableBindings OIDs
  size_t        my_oid_length = 0;                 // Holder for pVariableBindings OIDs

  oid           enterprise_oid[MAX_OID_LEN];       // Holder for enterprise OID
  size_t        enterprise_oid_length = 0;          // Holder for enterprise OID

  oid           ret_oid[MAX_OID_LEN];               // Holder for return OIDs
  size_t        ret_oid_length = 0;                 // Holder for return OIDs

  char          *stringtemp;

  /*
   * here is where we store the variables to be sent in the trap 
   */
  netsnmp_variable_list *notification_vars = NULL;
  
  /*
   * In the notification, we have to assign our notification OID to
   * the snmpTrapOID.0 object. Here is it's definition. 
   */
  oid             objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
  size_t          objid_snmptrap_len = OID_LENGTH(objid_snmptrap);

  DEBUGMSGTL(("winExtDLL", "send_trap() called\n"));
  DEBUGMSGTL(("winExtDLL", "pVariableBindings length: %d\n",pVariableBindings->len));

  if (*pGenericTrap != 6) {
    DEBUGMSGTL(("winExtDLL", "Working on generic trap\n"));
    DEBUGMSGTL(("winExtDLL", "sending v1 trap\n"));
    send_easy_trap(*pGenericTrap, *pSpecificTrap);
    return;
  }

  // Convert enterprise OID from Windows to Net-SNMP so Net-SNMP
  for (j = 0; j < (pEnterprise->idLength > MAX_OID_LEN?MAX_OID_LEN:pEnterprise->idLength); j++) {
    enterprise_oid[j] = (oid)pEnterprise->ids[j];
  }
  enterprise_oid_length = j;

  DEBUGMSGTL(("winExtDLL", "Enterprise OID: "));
  DEBUGMSGOID(("winExtDLL", enterprise_oid, enterprise_oid_length));
  DEBUGMSG(("winExtDLL", "\n"));

  // We need to copy .0.specific trap to copy of enterprise and use that for objid_snmptrap!!

  for (i = 0; i < enterprise_oid_length; i++) {
    my_oid[i] = enterprise_oid[i];
  }
  my_oid_length = i;

  my_oid[my_oid_length] = 0;
  my_oid_length++;

  my_oid[my_oid_length] = *pSpecificTrap;
  my_oid_length++;

  DEBUGMSGTL(("winExtDLL", "Trap OID (snmpTrapOID.0): "));
  DEBUGMSGOID(("winExtDLL", my_oid, my_oid_length));
  DEBUGMSG(("winExtDLL", "\n"));

  /*
   * add in the trap definition object 
   */
  snmp_varlist_add_variable(&notification_vars,
      /*
       * the snmpTrapOID.0 variable 
       */
      objid_snmptrap, objid_snmptrap_len,
      /*
       * value type is an OID 
       */
      ASN_OBJECT_ID,
      /*
       * value contents is our notification OID 
       */
      (u_char *) my_oid,
      /*
       * size in bytes
       */
      my_oid_length  * sizeof(oid));

  for (i = 0; i< pVariableBindings->len; i++) {

    mySnmpVarBind = pVariableBindings->list[i];

    // Convert OID from Windows to Net-SNMP so Net-SNMP
    for (j = 0; j < (mySnmpVarBind.name.idLength > MAX_OID_LEN?MAX_OID_LEN:mySnmpVarBind.name.idLength); j++) {
      my_oid[j] = (oid)mySnmpVarBind.name.ids[j];
    }
    my_oid_length = j;

    DEBUGMSGTL(("winExtDLL", "OID in trap variable binding: "));
    DEBUGMSGOID(("winExtDLL", my_oid, my_oid_length));
    DEBUGMSG(("winExtDLL", "\n"));

    // Set Net-SNMP ASN type based on closest match to Windows ASN type
    switch (mySnmpVarBind.value.asnType) {
      case MS_ASN_OCTETSTRING:        // AsnOctetString
        stringtemp = netsnmp_strdup_and_null(mySnmpVarBind.value.asnValue.string.stream, mySnmpVarBind.value.asnValue.string.length);
        DEBUGMSGTL(("winExtDLL", "MS_ASN_OCTETSTRING = ASN_OCTET_STR\n"));
        DEBUGMSGTL(("winExtDLL", "MS_ASN_OCTETSTRING = %s\n",stringtemp));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_OCTET_STR,
            stringtemp,
            strlen(stringtemp));
        break;
      case MS_ASN_INTEGER:          // And MS_ASN_INTEGER32
        DEBUGMSGTL(("winExtDLL", "MS_ASN_INTEGER = ASN_INTEGER\n"));
        DEBUGMSGTL(("winExtDLL", "MS_ASN_INTEGER = %d\n",mySnmpVarBind.value.asnValue.number));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_INTEGER,
            (u_char *)&mySnmpVarBind.value.asnValue.number,
            sizeof(mySnmpVarBind.value.asnValue.number));
        break;
      case MS_ASN_UNSIGNED32:       // SNMP v2
        DEBUGMSGTL(("winExtDLL", "MS_ASN_UNSIGNED32 = ASN_UNSIGNED\n"));
        DEBUGMSGTL(("winExtDLL", "MS_ASN_UNSIGNED32 = %d\n",mySnmpVarBind.value.asnValue.unsigned32));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_UNSIGNED,
            (u_char *)&mySnmpVarBind.value.asnValue.unsigned32,
            sizeof(mySnmpVarBind.value.asnValue.unsigned32));
        break;
      case MS_ASN_COUNTER64:       // SNMP v2
        DEBUGMSGTL(("winExtDLL", "MS_ASN_COUNTER64 = ASN_COUNTER64\n"));
        DEBUGMSGTL(("winExtDLL", "MS_ASN_COUNTER64 = %d\n",mySnmpVarBind.value.asnValue.counter64));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_COUNTER64,
            (u_char *)&mySnmpVarBind.value.asnValue.counter64,
            sizeof(mySnmpVarBind.value.asnValue.counter64));
        break;
      case MS_ASN_BITS:               // AsnOctetString
        stringtemp = netsnmp_strdup_and_null(mySnmpVarBind.value.asnValue.bits.stream, mySnmpVarBind.value.asnValue.bits.length);
        DEBUGMSGTL(("winExtDLL", "MS_ASN_BITS = ASN_BIT_STR\n"));
        DEBUGMSGTL(("winExtDLL", "MS_ASN_BITS = %s\n",stringtemp));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_BIT_STR,
            stringtemp,
            strlen(stringtemp));
        break;
      case MS_ASN_OBJECTIDENTIFIER:   // AsnObjectIdentifier
        DEBUGMSGTL(("winExtDLL", "MS_ASN_OBJECTIDENTIFIER = ASN_OBJECT_ID\n"));
        // Convert OID to Net-SNMP

        DEBUGMSGTL(("winExtDLL", "Returned OID: "));
        DEBUGMSGWINOID(("winExtDLL", &mySnmpVarBind.value.asnValue.object));
        DEBUGMSG(("winExtDLL", "\n"));
        
        // Convert OID from Windows to Net-SNMP
        for (i = 0; i < (mySnmpVarBind.value.asnValue.object.idLength > MAX_OID_LEN?MAX_OID_LEN:
              mySnmpVarBind.value.asnValue.object.idLength); i++) {
          ret_oid[i] = (oid)mySnmpVarBind.value.asnValue.object.ids[i];
        }
        ret_oid_length = i;
        
        DEBUGMSGTL(("winExtDLL", "Windows OID converted to Net-SNMP: "));
        DEBUGMSGOID(("winExtDLL", ret_oid, ret_oid_length));
        DEBUGMSG(("winExtDLL", "\n"));

        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_OBJECT_ID,
            (u_char *)&ret_oid,
            ret_oid_length * sizeof(oid));
        break;
      case MS_ASN_SEQUENCE:           // AsnOctetString
        stringtemp = netsnmp_strdup_and_null(mySnmpVarBind.value.asnValue.sequence.stream, mySnmpVarBind.value.asnValue.sequence.length);
        DEBUGMSGTL(("winExtDLL", "MS_ASN_SEQUENCE = ASN_SEQUENCE\n"));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_SEQUENCE,
            stringtemp,
            strlen(stringtemp));
        break;
      case MS_ASN_IPADDRESS:          // AsnOctetString
        stringtemp = netsnmp_strdup_and_null(mySnmpVarBind.value.asnValue.address.stream, mySnmpVarBind.value.asnValue.address.length);
        DEBUGMSGTL(("winExtDLL", "MS_ASN_IPADDRESS = ASN_IPADDRESS\n"));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_IPADDRESS,
            stringtemp,
            strlen(stringtemp));
        break;
      case MS_ASN_COUNTER32:          
        DEBUGMSGTL(("winExtDLL", "MS_ASN_COUNTER32 = ASN_COUNTER\n"));
        DEBUGMSGTL(("winExtDLL", "MS_ASN_COUNTER32 = %d\n",mySnmpVarBind.value.asnValue.counter));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_COUNTER,
            (u_char *)&mySnmpVarBind.value.asnValue.counter,
            sizeof(mySnmpVarBind.value.asnValue.counter));
        break;
      case MS_ASN_GAUGE32:
        DEBUGMSGTL(("winExtDLL", "MS_ASN_GAUGE32 = ASN_GAUGE\n"));
        DEBUGMSGTL(("winExtDLL", "MS_ASN_GAUGE32 = %d\n",mySnmpVarBind.value.asnValue.gauge));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_GAUGE,
            (u_char *)&mySnmpVarBind.value.asnValue.gauge,
            sizeof(mySnmpVarBind.value.asnValue.gauge));
        break;
      case MS_ASN_TIMETICKS:
        DEBUGMSGTL(("winExtDLL", "MS_ASN_TIMETICKS = ASN_TIMETICKS\n"));
        DEBUGMSGTL(("winExtDLL", "MS_ASN_TIMETICKS = %d\n",mySnmpVarBind.value.asnValue.ticks));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_TIMETICKS,
            (u_char *)&mySnmpVarBind.value.asnValue.ticks,
            sizeof(mySnmpVarBind.value.asnValue.ticks));
        break;
      case MS_ASN_OPAQUE:             // AsnOctetString
        stringtemp = netsnmp_strdup_and_null(mySnmpVarBind.value.asnValue.arbitrary.stream, mySnmpVarBind.value.asnValue.arbitrary.length);
        DEBUGMSGTL(("winExtDLL", "MS_ASN_OPAQUE = ASN_OPAQUE\n"));
        snmp_varlist_add_variable(&notification_vars,
            my_oid, my_oid_length,
            ASN_OPAQUE,
            stringtemp,
            strlen(stringtemp));          break;
      default:
        DEBUGMSGTL(("winExtDLL", "Defaulting to ASN_INTEGER\n"));
        break;
    }
  }
  /*
   * send the trap out.  This will send it to all registered
   * receivers (see the "SETTING UP TRAP AND/OR INFORM DESTINATIONS"
   * section of the snmpd.conf manual page. 
   */

  DEBUGMSGTL(("winExtDLL", "sending v2 trap\n"));
  send_v2trap(notification_vars);

  /*
   * free the created notification variable list 
   */
  DEBUGMSGTL(("winExtDLL", "cleaning up\n"));
  snmp_free_varbind(notification_vars);
}

/* DEBUGMSGWINOID */
void
debugmsg_win_oid(const char *token, const AsnObjectIdentifier * theoid)
{
    u_char          buf[1024];
    u_char          temp[10];
    size_t          buf_len = 0, out_len = 0;
    int             i;

    buf[0] = '\0';

    for (i = 0; i < theoid->idLength; i++) {
      sprintf(temp, ".%d", theoid->ids[i]);
      strcat(buf, temp);
    }

    if (buf != NULL) {
      debugmsg(token, "%s", buf);
      //DEBUGMSGTL((token, "%s\n", buf));
    }
}


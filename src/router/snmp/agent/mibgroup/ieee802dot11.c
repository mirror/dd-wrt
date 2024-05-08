/****************************************************************************
*                                                                           *
*  File Name:           ieee802dot11.c                                      *
*  Used By:                                                                 *
*                                                                           *
*  Operating System:                                                        *
*  Purpose:                                                                 *
*                                                                           *
*  Comments:                                                                *
*                                                                           *
*  Author:              Larry Simmons                                       *
*                       lsimmons@avantcom.com                               *
*                       www.avantcom.com                                    *
*                                                                           *
*  Creation Date:       09/02/03                                            *
*                                                                           *
*   Ver    Date   Inits Modification                                        *
*  ----- -------- ----- ------------                                        *
*  0.0.1 09/02/03  LRS  created                                             *
*  0.0.2 09/24/03  LRS  wouldn't build after fresh ./configure              *
****************************************************************************/
/****************************************************************************
*                               Includes                                    *
****************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "util_funcs/header_generic.h" /* utility function declarations */
#include "ieee802dot11.h"
#include "iwlib.h"

/****************************************************************************
*                                Defines                                    *
****************************************************************************/
#define DISPLAYWIEXT                        // display wireless ext info
#define TABLE_SIZE   1
//#define MINLOADFREQ 15                    // min reload frequency in seconds
#define MINLOADFREQ 5                       // min reload frequency in seconds      // for testing
#define PROC_NET_DEV      "/proc/net/dev"
#define PROC_NET_WIRELESS "/proc/net/wireless"

#ifndef UCHAR
  typedef unsigned char UCHAR;
#endif

/****************************************************************************
*                            Private Functions                              *
****************************************************************************/
static void loadTables();
static void loadWiExt ( int, char *, struct wireless_info * );
static void load80211Structs ( int, char *, struct wireless_info * );
static void initStructs();

// Wireless Extensions Specific Functions
static void loadWiExtTo80211Structs ( int, char *, struct wireless_info * );
static void displayWiExt ( struct wireless_info );

// Linked List Functions
static void addList ( char *, char *, int );
static void initLists();                    // initialize all the linked lists
static void flushLists();                   // flush all the linked lists
static void flushList ( char * );           // flush a single linked list

// Utility Functions
static int  openSocket ( void );
static int  mWatt2dbm ( int );
static char *htob ( char * );
static int  hasChanged ( char *, int );

/****************************************************************************
*                            Private Variables                              *
****************************************************************************/
static unsigned long lastLoad = 0;          // ET in secs at last table load

static struct avNode *lastNode, *newNode, *np;

/****************************************************************************
*                            External Functions                             *
****************************************************************************/

/****************************************************************************
*   ieee802dot11_variables_oid:                                             *
*       this is the top level oid that we want to register under.  This     *
*       is essentially a prefix, with the suffix appearing in the           *
*       variable below.                                                     *
****************************************************************************/
oid ieee802dot11_variables_oid[] = { 1,2,840,10036 };

/****************************************************************************
*   variable7 ieee802dot11_variables:                                       *
*     this variable defines function callbacks and type return information  *
*     for the ieee802dot11 mib section                                      *
****************************************************************************/
struct variable7 ieee802dot11_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#define   DOT11STATIONID        3
  { DOT11STATIONID      , ASN_OCTET_STR , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,1 } },
#define   DOT11MEDIUMOCCUPANCYLIMIT  4
  { DOT11MEDIUMOCCUPANCYLIMIT, ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,2 } },
#define   DOT11CFPOLLABLE       5
  { DOT11CFPOLLABLE     , ASN_INTEGER   , RONLY , var_dot11StationConfigTable, 4, { 1,1,1,3 } },
#define   DOT11CFPPERIOD        6
  { DOT11CFPPERIOD      , ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,4 } },
#define   DOT11CFPMAXDURATION   7
  { DOT11CFPMAXDURATION , ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,5 } },
#define   DOT11AUTHENTICATIONRESPONSETIMEOUT  8
  { DOT11AUTHENTICATIONRESPONSETIMEOUT, ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,6 } },
#define   DOT11PRIVACYOPTIONIMPLEMENTED  9
  { DOT11PRIVACYOPTIONIMPLEMENTED, ASN_INTEGER   , RONLY , var_dot11StationConfigTable, 4, { 1,1,1,7 } },
#define   DOT11POWERMANAGEMENTMODE  10
  { DOT11POWERMANAGEMENTMODE, ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,8 } },
#define   DOT11DESIREDSSID      11
  { DOT11DESIREDSSID    , ASN_OCTET_STR , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,9 } },
#define   DOT11DESIREDBSSTYPE   12
  { DOT11DESIREDBSSTYPE , ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,10 } },
#define   DOT11OPERATIONALRATESET  13
  { DOT11OPERATIONALRATESET, ASN_OCTET_STR , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,11 } },
#define   DOT11BEACONPERIOD     14
  { DOT11BEACONPERIOD   , ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,12 } },
#define   DOT11DTIMPERIOD       15
  { DOT11DTIMPERIOD     , ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,13 } },
#define   DOT11ASSOCIATIONRESPONSETIMEOUT  16
  { DOT11ASSOCIATIONRESPONSETIMEOUT, ASN_INTEGER   , RWRITE, var_dot11StationConfigTable, 4, { 1,1,1,14 } },
#define   DOT11DISASSOCIATEREASON  17
  { DOT11DISASSOCIATEREASON, ASN_INTEGER   , RONLY , var_dot11StationConfigTable, 4, { 1,1,1,15 } },
#define   DOT11DISASSOCIATESTATION  18
  { DOT11DISASSOCIATESTATION, ASN_OCTET_STR , RONLY , var_dot11StationConfigTable, 4, { 1,1,1,16 } },
#define   DOT11DEAUTHENTICATEREASON  19
  { DOT11DEAUTHENTICATEREASON, ASN_INTEGER   , RONLY , var_dot11StationConfigTable, 4, { 1,1,1,17 } },
#define   DOT11DEAUTHENTICATESTATION  20
  { DOT11DEAUTHENTICATESTATION, ASN_OCTET_STR , RONLY , var_dot11StationConfigTable, 4, { 1,1,1,18 } },
#define   DOT11AUTHENTICATEFAILSTATUS  21
  { DOT11AUTHENTICATEFAILSTATUS, ASN_INTEGER   , RONLY , var_dot11StationConfigTable, 4, { 1,1,1,19 } },
#define   DOT11AUTHENTICATEFAILSTATION  22
  { DOT11AUTHENTICATEFAILSTATION, ASN_OCTET_STR , RONLY , var_dot11StationConfigTable, 4, { 1,1,1,20 } },

#define   DOT11AUTHENTICATIONALGORITHM  26
  { DOT11AUTHENTICATIONALGORITHM, ASN_INTEGER   , RONLY , var_dot11AuthenticationAlgorithmsTable, 4, { 1,2,1,2 } },
#define   DOT11AUTHENTICATIONALGORITHMSENABLE  27
  { DOT11AUTHENTICATIONALGORITHMSENABLE, ASN_INTEGER   , RWRITE, var_dot11AuthenticationAlgorithmsTable, 4, { 1,2,1,3 } },

#define   DOT11WEPDEFAULTKEYVALUE  31
  { DOT11WEPDEFAULTKEYVALUE, ASN_OCTET_STR , RWRITE, var_dot11WEPDefaultKeysTable, 4, { 1,3,1,2 } },

#define   DOT11WEPKEYMAPPINGADDRESS  35
  { DOT11WEPKEYMAPPINGADDRESS, ASN_OCTET_STR , RWRITE, var_dot11WEPKeyMappingsTable, 4, { 1,4,1,2 } },
#define   DOT11WEPKEYMAPPINGWEPON  36
  { DOT11WEPKEYMAPPINGWEPON, ASN_INTEGER   , RWRITE, var_dot11WEPKeyMappingsTable, 4, { 1,4,1,3 } },
#define   DOT11WEPKEYMAPPINGVALUE  37
  { DOT11WEPKEYMAPPINGVALUE, ASN_OCTET_STR , RWRITE, var_dot11WEPKeyMappingsTable, 4, { 1,4,1,4 } },
#define   DOT11WEPKEYMAPPINGSTATUS  38
  { DOT11WEPKEYMAPPINGSTATUS, ASN_INTEGER   , RWRITE, var_dot11WEPKeyMappingsTable, 4, { 1,4,1,5 } },

#define   DOT11PRIVACYINVOKED   41
  { DOT11PRIVACYINVOKED , ASN_INTEGER   , RWRITE, var_dot11PrivacyTable, 4, { 1,5,1,1 } },
#define   DOT11WEPDEFAULTKEYID  42
  { DOT11WEPDEFAULTKEYID, ASN_INTEGER   , RWRITE, var_dot11PrivacyTable, 4, { 1,5,1,2 } },
#define   DOT11WEPKEYMAPPINGLENGTH  43
  { DOT11WEPKEYMAPPINGLENGTH, ASN_INTEGER   , RWRITE, var_dot11PrivacyTable, 4, { 1,5,1,3 } },
#define   DOT11EXCLUDEUNENCRYPTED  44
  { DOT11EXCLUDEUNENCRYPTED, ASN_INTEGER   , RWRITE, var_dot11PrivacyTable, 4, { 1,5,1,4 } },
#define   DOT11WEPICVERRORCOUNT  45
  { DOT11WEPICVERRORCOUNT, ASN_COUNTER   , RONLY , var_dot11PrivacyTable, 4, { 1,5,1,5 } },
#define   DOT11WEPEXCLUDEDCOUNT  46
  { DOT11WEPEXCLUDEDCOUNT, ASN_COUNTER   , RONLY , var_dot11PrivacyTable, 4, { 1,5,1,6 } },

#define   DOT11MACADDRESS       49
  { DOT11MACADDRESS     , ASN_OCTET_STR , RONLY , var_dot11OperationTable, 4, { 2,1,1,1 } },
#define   DOT11RTSTHRESHOLD     50
  { DOT11RTSTHRESHOLD   , ASN_INTEGER   , RWRITE, var_dot11OperationTable, 4, { 2,1,1,2 } },
#define   DOT11SHORTRETRYLIMIT  51
  { DOT11SHORTRETRYLIMIT, ASN_INTEGER   , RWRITE, var_dot11OperationTable, 4, { 2,1,1,3 } },
#define   DOT11LONGRETRYLIMIT   52
  { DOT11LONGRETRYLIMIT , ASN_INTEGER   , RWRITE, var_dot11OperationTable, 4, { 2,1,1,4 } },
#define   DOT11FRAGMENTATIONTHRESHOLD  53
  { DOT11FRAGMENTATIONTHRESHOLD, ASN_INTEGER   , RWRITE, var_dot11OperationTable, 4, { 2,1,1,5 } },
#define   DOT11MAXTRANSMITMSDULIFETIME  54
  { DOT11MAXTRANSMITMSDULIFETIME, ASN_INTEGER   , RWRITE, var_dot11OperationTable, 4, { 2,1,1,6 } },
#define   DOT11MAXRECEIVELIFETIME  55
  { DOT11MAXRECEIVELIFETIME, ASN_INTEGER   , RWRITE, var_dot11OperationTable, 4, { 2,1,1,7 } },
#define   DOT11MANUFACTURERID   56
  { DOT11MANUFACTURERID , ASN_OCTET_STR , RONLY , var_dot11OperationTable, 4, { 2,1,1,8 } },
#define   DOT11PRODUCTID        57
  { DOT11PRODUCTID      , ASN_OCTET_STR , RONLY , var_dot11OperationTable, 4, { 2,1,1,9 } },

#define   DOT11TRANSMITTEDFRAGMENTCOUNT  60
  { DOT11TRANSMITTEDFRAGMENTCOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,1 } },
#define   DOT11MULTICASTTRANSMITTEDFRAMECOUNT  61
  { DOT11MULTICASTTRANSMITTEDFRAMECOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,2 } },
#define   DOT11FAILEDCOUNT      62
  { DOT11FAILEDCOUNT    , ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,3 } },
#define   DOT11RETRYCOUNT       63
  { DOT11RETRYCOUNT     , ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,4 } },
#define   DOT11MULTIPLERETRYCOUNT  64
  { DOT11MULTIPLERETRYCOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,5 } },
#define   DOT11FRAMEDUPLICATECOUNT  65
  { DOT11FRAMEDUPLICATECOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,6 } },
#define   DOT11RTSSUCCESSCOUNT  66
  { DOT11RTSSUCCESSCOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,7 } },
#define   DOT11RTSFAILURECOUNT  67
  { DOT11RTSFAILURECOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,8 } },
#define   DOT11ACKFAILURECOUNT  68
  { DOT11ACKFAILURECOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,9 } },
#define   DOT11RECEIVEDFRAGMENTCOUNT  69
  { DOT11RECEIVEDFRAGMENTCOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,10 } },
#define   DOT11MULTICASTRECEIVEDFRAMECOUNT  70
  { DOT11MULTICASTRECEIVEDFRAMECOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,11 } },
#define   DOT11FCSERRORCOUNT    71
  { DOT11FCSERRORCOUNT  , ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,12 } },
#define   DOT11TRANSMITTEDFRAMECOUNT  72
  { DOT11TRANSMITTEDFRAMECOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,13 } },
#define   DOT11WEPUNDECRYPTABLECOUNT  73
  { DOT11WEPUNDECRYPTABLECOUNT, ASN_COUNTER   , RONLY , var_dot11CountersTable, 4, { 2,2,1,14 } },

#define   DOT11ADDRESS          77
  { DOT11ADDRESS        , ASN_OCTET_STR , RWRITE, var_dot11GroupAddressesTable, 4, { 2,3,1,2 } },
#define   DOT11GROUPADDRESSESSTATUS  78
  { DOT11GROUPADDRESSESSTATUS, ASN_INTEGER   , RWRITE, var_dot11GroupAddressesTable, 4, { 2,3,1,3 } },

#define   DOT11RESOURCETYPEIDNAME  79
  { DOT11RESOURCETYPEIDNAME, ASN_OCTET_STR , RONLY , var_ieee802dot11, 3, { 3,1,1 } },
#define   DOT11MANUFACTUREROUI  82
  { DOT11MANUFACTUREROUI, ASN_OCTET_STR , RONLY , var_dot11ResourceInfoTable, 5, { 3,1,2,1,1 } },
#define   DOT11MANUFACTURERNAME  83
  { DOT11MANUFACTURERNAME, ASN_OCTET_STR , RONLY , var_dot11ResourceInfoTable, 5, { 3,1,2,1,2 } },
#define   DOT11MANUFACTURERPRODUCTNAME  84
  { DOT11MANUFACTURERPRODUCTNAME, ASN_OCTET_STR , RONLY , var_dot11ResourceInfoTable, 5, { 3,1,2,1,3 } },
#define   DOT11MANUFACTURERPRODUCTVERSION  85
  { DOT11MANUFACTURERPRODUCTVERSION, ASN_OCTET_STR , RONLY , var_dot11ResourceInfoTable, 5, { 3,1,2,1,4 } },

#define   DOT11PHYTYPE          88
  { DOT11PHYTYPE        , ASN_INTEGER   , RONLY , var_dot11PhyOperationTable, 4, { 4,1,1,1 } },
#define   DOT11CURRENTREGDOMAIN  89
  { DOT11CURRENTREGDOMAIN, ASN_INTEGER   , RWRITE, var_dot11PhyOperationTable, 4, { 4,1,1,2 } },
#define   DOT11TEMPTYPE         90
  { DOT11TEMPTYPE       , ASN_INTEGER   , RONLY , var_dot11PhyOperationTable, 4, { 4,1,1,3 } },
#define   DOT11CURRENTTXANTENNA  93
  { DOT11CURRENTTXANTENNA, ASN_INTEGER   , RWRITE, var_dot11PhyAntennaTable, 4, { 4,2,1,1 } },
#define   DOT11DIVERSITYSUPPORT  94
  { DOT11DIVERSITYSUPPORT, ASN_INTEGER   , RONLY , var_dot11PhyAntennaTable, 4, { 4,2,1,2 } },
#define   DOT11CURRENTRXANTENNA  95
  { DOT11CURRENTRXANTENNA, ASN_INTEGER   , RWRITE, var_dot11PhyAntennaTable, 4, { 4,2,1,3 } },
#define   DOT11NUMBERSUPPORTEDPOWERLEVELS  98
  { DOT11NUMBERSUPPORTEDPOWERLEVELS, ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,1 } },
#define   DOT11TXPOWERLEVEL1    99
  { DOT11TXPOWERLEVEL1  , ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,2 } },
#define   DOT11TXPOWERLEVEL2    100
  { DOT11TXPOWERLEVEL2  , ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,3 } },
#define   DOT11TXPOWERLEVEL3    101
  { DOT11TXPOWERLEVEL3  , ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,4 } },
#define   DOT11TXPOWERLEVEL4    102
  { DOT11TXPOWERLEVEL4  , ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,5 } },
#define   DOT11TXPOWERLEVEL5    103
  { DOT11TXPOWERLEVEL5  , ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,6 } },
#define   DOT11TXPOWERLEVEL6    104
  { DOT11TXPOWERLEVEL6  , ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,7 } },
#define   DOT11TXPOWERLEVEL7    105
  { DOT11TXPOWERLEVEL7  , ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,8 } },
#define   DOT11TXPOWERLEVEL8    106
  { DOT11TXPOWERLEVEL8  , ASN_INTEGER   , RONLY , var_dot11PhyTxPowerTable, 4, { 4,3,1,9 } },
#define   DOT11CURRENTTXPOWERLEVEL  107
  { DOT11CURRENTTXPOWERLEVEL, ASN_INTEGER   , RWRITE, var_dot11PhyTxPowerTable, 4, { 4,3,1,10 } },

#define   DOT11HOPTIME          110
  { DOT11HOPTIME        , ASN_INTEGER   , RONLY , var_dot11PhyFHSSTable, 4, { 4,4,1,1 } },
#define   DOT11CURRENTCHANNELNUMBER  111
  { DOT11CURRENTCHANNELNUMBER, ASN_INTEGER   , RWRITE, var_dot11PhyFHSSTable, 4, { 4,4,1,2 } },
#define   DOT11MAXDWELLTIME     112
  { DOT11MAXDWELLTIME   , ASN_INTEGER   , RONLY , var_dot11PhyFHSSTable, 4, { 4,4,1,3 } },
#define   DOT11CURRENTDWELLTIME  113
  { DOT11CURRENTDWELLTIME, ASN_INTEGER   , RWRITE, var_dot11PhyFHSSTable, 4, { 4,4,1,4 } },
#define   DOT11CURRENTSET       114
  { DOT11CURRENTSET     , ASN_INTEGER   , RWRITE, var_dot11PhyFHSSTable, 4, { 4,4,1,5 } },
#define   DOT11CURRENTPATTERN   115
  { DOT11CURRENTPATTERN , ASN_INTEGER   , RWRITE, var_dot11PhyFHSSTable, 4, { 4,4,1,6 } },
#define   DOT11CURRENTINDEX     116
  { DOT11CURRENTINDEX   , ASN_INTEGER   , RWRITE, var_dot11PhyFHSSTable, 4, { 4,4,1,7 } },

#define   DOT11CURRENTCHANNEL   119
  { DOT11CURRENTCHANNEL , ASN_INTEGER   , RWRITE, var_dot11PhyDSSSTable, 4, { 4,5,1,1 } },
#define   DOT11CCAMODESUPPORTED  120
  { DOT11CCAMODESUPPORTED, ASN_INTEGER   , RONLY , var_dot11PhyDSSSTable, 4, { 4,5,1,2 } },
#define   DOT11CURRENTCCAMODE   121
  { DOT11CURRENTCCAMODE , ASN_INTEGER   , RWRITE, var_dot11PhyDSSSTable, 4, { 4,5,1,3 } },
#define   DOT11EDTHRESHOLD      122
  { DOT11EDTHRESHOLD    , ASN_INTEGER   , RWRITE, var_dot11PhyDSSSTable, 4, { 4,5,1,4 } },

#define   DOT11CCAWATCHDOGTIMERMAX  125
  { DOT11CCAWATCHDOGTIMERMAX, ASN_INTEGER   , RWRITE, var_dot11PhyIRTable, 4, { 4,6,1,1 } },
#define   DOT11CCAWATCHDOGCOUNTMAX  126
  { DOT11CCAWATCHDOGCOUNTMAX, ASN_INTEGER   , RWRITE, var_dot11PhyIRTable, 4, { 4,6,1,2 } },
#define   DOT11CCAWATCHDOGTIMERMIN  127
  { DOT11CCAWATCHDOGTIMERMIN, ASN_INTEGER   , RWRITE, var_dot11PhyIRTable, 4, { 4,6,1,3 } },
#define   DOT11CCAWATCHDOGCOUNTMIN  128
  { DOT11CCAWATCHDOGCOUNTMIN, ASN_INTEGER   , RWRITE, var_dot11PhyIRTable, 4, { 4,6,1,4 } },

#define   DOT11REGDOMAINSSUPPORTVALUE  132
  { DOT11REGDOMAINSSUPPORTVALUE, ASN_INTEGER   , RONLY , var_dot11RegDomainsSupportedTable, 4, { 4,7,1,2 } },

#define   DOT11SUPPORTEDTXANTENNA  136
  { DOT11SUPPORTEDTXANTENNA, ASN_INTEGER   , RWRITE, var_dot11AntennasListTable, 4, { 4,8,1,2 } },
#define   DOT11SUPPORTEDRXANTENNA  137
  { DOT11SUPPORTEDRXANTENNA, ASN_INTEGER   , RWRITE, var_dot11AntennasListTable, 4, { 4,8,1,3 } },
#define   DOT11DIVERSITYSELECTIONRX  138
  { DOT11DIVERSITYSELECTIONRX, ASN_INTEGER   , RWRITE, var_dot11AntennasListTable, 4, { 4,8,1,4 } },

#define   DOT11SUPPORTEDDATARATESTXVALUE  142
  { DOT11SUPPORTEDDATARATESTXVALUE, ASN_INTEGER   , RONLY , var_dot11SupportedDataRatesTxTable, 4, { 4,9,1,2 } },

#define   DOT11SUPPORTEDDATARATESRXVALUE  146
  { DOT11SUPPORTEDDATARATESRXVALUE, ASN_INTEGER   , RONLY , var_dot11SupportedDataRatesRxTable, 4, { 4,10,1,2 } },
};
// ( L = length of the oidsuffix )

/****************************************************************************
*                                                                           *
*         init_ieee802dot11() - perform any required initialization         *
*                                                                           *
****************************************************************************/
void init_ieee802dot11 ( void ) {

  /* register ourselves with the agent to handle our mib tree */
  REGISTER_MIB("ieee802dot11", ieee802dot11_variables, variable7,
               ieee802dot11_variables_oid);

  initLists();
}

/****************************************************************************
*                                                                           *
*    shutdown_ieee802dot11() - perform any required cleanup @ shutdown      *
*                                                                           *
****************************************************************************/
void shutdown_ieee802dot11 ( void )
{
  flushLists();
}

/****************************************************************************
*                                                                           *
*   var_ieee802dot11() -                                                    *
*                                                                           *
****************************************************************************/
unsigned char *
var_ieee802dot11 ( struct variable *vp, 
                    oid     *name, 
                    size_t  *length, 
                    int     exact, 
                    size_t  *var_len, 
                    WriteMethod **write_method)
{
  loadTables();                                               

  if ( header_generic ( vp, name, length, exact,var_len,write_method )
                                  == MATCH_FAILED )
    return NULL;

  switch ( vp->magic ) {

    case DOT11RESOURCETYPEIDNAME:
      if ( !haveResourceTypeIDName )
        return NULL;
      *var_len = strlen ( resourceTypeIDName );
      return ( UCHAR * ) resourceTypeIDName;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*  var_dot11StationConfigTable() - return a variable value from the table   *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11StationConfigTable ( struct variable *vp,
                              oid     *name,
                              size_t  *length,
                              int     exact,
                              size_t  *var_len,
                              WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned
  static char MACWork[17];

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &scList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    sc = ( struct scTbl_data * ) np->data;
    rName[vp->namelen] = sc->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) || 
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {

      switch ( vp->magic ) {      // found requested OID, now check for requested variable
        case DOT11STATIONID: 
          if ( sc->haveStationID                     ) found = TRUE; break;
        case DOT11MEDIUMOCCUPANCYLIMIT:
          if ( sc->haveMediumOccupancyLimit          ) found = TRUE; break;
        case DOT11CFPOLLABLE:
          if ( sc->haveCFPPollable                   ) found = TRUE; break;
        case DOT11CFPPERIOD:
          if ( sc->haveCFPPeriod                     ) found = TRUE; break;
        case DOT11CFPMAXDURATION:
          if ( sc->haveMaxDuration                   ) found = TRUE; break;
        case DOT11AUTHENTICATIONRESPONSETIMEOUT:
          if ( sc->haveAuthenticationResponseTimeOut ) found = TRUE; break;
        case DOT11PRIVACYOPTIONIMPLEMENTED:
          if ( sc->havePrivacyOptionImplemented      ) found = TRUE; break;
        case DOT11POWERMANAGEMENTMODE:
          if ( sc->havePowerManagementMode           ) found = TRUE; break;
        case DOT11DESIREDSSID:
          if ( sc->haveDesiredSSID                   ) found = TRUE; break;
        case DOT11DESIREDBSSTYPE:
          if ( sc->haveDesiredBSSType                ) found = TRUE; break;
        case DOT11OPERATIONALRATESET:
          if ( sc->haveOperationalRateSet            ) found = TRUE; break;
        case DOT11BEACONPERIOD: 
          if ( sc->haveBeaconPeriod                  ) found = TRUE; break;
        case DOT11DTIMPERIOD:
          if ( sc->haveDTIMPeriod                    ) found = TRUE; break;
        case DOT11ASSOCIATIONRESPONSETIMEOUT:
          if ( sc->haveAssociationResponseTimeOut    ) found = TRUE; break;
        case DOT11DISASSOCIATEREASON:
          if ( sc->disAssociationReason              ) found = TRUE; break;
        case DOT11DISASSOCIATESTATION:
          if ( sc->haveDisAssociationStation         ) found = TRUE; break;
        case DOT11DEAUTHENTICATEREASON:
          if ( sc->deAuthenticationReason            ) found = TRUE; break;
        case DOT11DEAUTHENTICATESTATION:
          if ( sc->haveDeAuthenticationStation       ) found = TRUE; break;
        case DOT11AUTHENTICATEFAILSTATUS:
          if ( sc->authenticateFailStatus            ) found = TRUE; break;
        case DOT11AUTHENTICATEFAILSTATION:
          if ( sc->haveAuthenticateFailStation       ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11STATIONID: 
//    *write_method = write_dot11StationID;
      MACWork[ 0] = sc->stationID [ 0];
      MACWork[ 1] = sc->stationID [ 1];
      MACWork[ 2] = sc->stationID [ 3];
      MACWork[ 3] = sc->stationID [ 4];
      MACWork[ 4] = sc->stationID [ 6];
      MACWork[ 5] = sc->stationID [ 7];
      MACWork[ 6] = sc->stationID [ 9];
      MACWork[ 7] = sc->stationID [10];
      MACWork[ 8] = sc->stationID [12];
      MACWork[ 9] = sc->stationID [13];
      MACWork[10] = sc->stationID [15];
      MACWork[11] = sc->stationID [16];
      MACWork[12] = '\0';
      *var_len = 6;
      return ( UCHAR * ) htob ( MACWork );

    case DOT11MEDIUMOCCUPANCYLIMIT:
//    *write_method = write_dot11MediumOccupancyLimit;
      sc->mediumOccupancyLimit = 5;
      return ( UCHAR * ) &sc->mediumOccupancyLimit;

    case DOT11CFPOLLABLE:
      return ( UCHAR * ) &sc->CFPPollable;

    case DOT11CFPPERIOD:
//    *write_method = write_dot11CFPPeriod;
      return ( UCHAR * ) &sc->CFPPeriod;

    case DOT11CFPMAXDURATION:
//    *write_method = write_dot11CFPMaxDuration;
      return ( UCHAR * ) &sc->maxDuration;

    case DOT11AUTHENTICATIONRESPONSETIMEOUT:
//    *write_method = write_dot11AuthenticationResponseTimeOut;
      return ( UCHAR * ) &sc->authenticationResponseTimeOut;

    case DOT11PRIVACYOPTIONIMPLEMENTED:
      return ( UCHAR * ) &sc->privacyOptionImplemented;

    case DOT11POWERMANAGEMENTMODE:
//    *write_method = write_dot11PowerManagementMode;
      return ( UCHAR * ) &sc->powerManagementMode;

    case DOT11DESIREDSSID:
//    *write_method = write_dot11DesiredSSID;
      *var_len = strlen ( sc->desiredSSID );
      return ( UCHAR * ) sc->desiredSSID;

    case DOT11DESIREDBSSTYPE:
//    *write_method = write_dot11DesiredBSSType;
      return ( UCHAR * ) &sc->desiredBSSType;

    case DOT11OPERATIONALRATESET:
//    *write_method = write_dot11OperationalRateSet;
      *var_len = strlen ( sc->operationalRateSet );
      return ( UCHAR * ) sc->operationalRateSet;

    case DOT11BEACONPERIOD: 
//    *write_method = write_dot11BeaconPeriod;
      return ( UCHAR * ) &sc->beaconPeriod;

    case DOT11DTIMPERIOD:
//    *write_method = write_dot11DTIMPeriod;
      return ( UCHAR * ) &sc->DTIMPeriod;

    case DOT11ASSOCIATIONRESPONSETIMEOUT:
//    *write_method = write_dot11AssociationResponseTimeOut;
      return ( UCHAR * ) &sc->associationResponseTimeOut;

    case DOT11DISASSOCIATEREASON:
      return ( UCHAR * ) &sc->disAssociationReason;

    case DOT11DISASSOCIATESTATION:
      MACWork[ 0] = sc->disAssociationStation[ 0];
      MACWork[ 1] = sc->disAssociationStation[ 1];
      MACWork[ 2] = sc->disAssociationStation[ 3];
      MACWork[ 3] = sc->disAssociationStation[ 4];
      MACWork[ 4] = sc->disAssociationStation[ 6];
      MACWork[ 5] = sc->disAssociationStation[ 7];
      MACWork[ 6] = sc->disAssociationStation[ 9];
      MACWork[ 7] = sc->disAssociationStation[10];
      MACWork[ 8] = sc->disAssociationStation[12];
      MACWork[ 9] = sc->disAssociationStation[13];
      MACWork[10] = sc->disAssociationStation[15];
      MACWork[11] = sc->disAssociationStation[16];
      MACWork[12] = '\0';
      *var_len = 6;
      return ( UCHAR * ) htob ( MACWork );

    case DOT11DEAUTHENTICATEREASON:
      return ( UCHAR * ) &sc->deAuthenticationReason;

    case DOT11DEAUTHENTICATESTATION:
      MACWork[ 0] = sc->deAuthenticationStation[ 0];
      MACWork[ 1] = sc->deAuthenticationStation[ 1];
      MACWork[ 2] = sc->deAuthenticationStation[ 3];
      MACWork[ 3] = sc->deAuthenticationStation[ 4];
      MACWork[ 4] = sc->deAuthenticationStation[ 6];
      MACWork[ 5] = sc->deAuthenticationStation[ 7];
      MACWork[ 6] = sc->deAuthenticationStation[ 9];
      MACWork[ 7] = sc->deAuthenticationStation[10];
      MACWork[ 8] = sc->deAuthenticationStation[12];
      MACWork[ 9] = sc->deAuthenticationStation[13];
      MACWork[10] = sc->deAuthenticationStation[15];
      MACWork[11] = sc->deAuthenticationStation[16];
      MACWork[12] = '\0';
      *var_len = 6;
      return ( UCHAR * ) htob ( MACWork );

    case DOT11AUTHENTICATEFAILSTATUS:
      return ( UCHAR * ) &sc->authenticateFailStatus;

    case DOT11AUTHENTICATEFAILSTATION:
      MACWork[ 0] = sc->authenticateFailStation[ 0];
      MACWork[ 1] = sc->authenticateFailStation[ 1];
      MACWork[ 2] = sc->authenticateFailStation[ 3];
      MACWork[ 3] = sc->authenticateFailStation[ 4];
      MACWork[ 4] = sc->authenticateFailStation[ 6];
      MACWork[ 5] = sc->authenticateFailStation[ 7];
      MACWork[ 6] = sc->authenticateFailStation[ 9];
      MACWork[ 7] = sc->authenticateFailStation[10];
      MACWork[ 8] = sc->authenticateFailStation[12];
      MACWork[ 9] = sc->authenticateFailStation[13];
      MACWork[10] = sc->authenticateFailStation[15];
      MACWork[11] = sc->authenticateFailStation[16];
      MACWork[12] = '\0';
      *var_len = 6;
      return ( UCHAR * ) htob ( MACWork );

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*  var_dot11AuthenticationAlgorithmsTable() -                               *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11AuthenticationAlgorithmsTable (  struct variable *vp,
                                          oid     *name,
                                          size_t  *length,
                                          int     exact,
                                          size_t  *var_len,
                                          WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &aaList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    aa = ( struct aaTbl_data * ) np->data;
    rName[vp->namelen + 0] = aa->ifIndex;
    rName[vp->namelen + 1] = aa->authenticationAlgorithmsIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11AUTHENTICATIONALGORITHM:
          if ( aa->haveAuthenticationAlgorithm    ) found = TRUE; break;
        case DOT11AUTHENTICATIONALGORITHMSENABLE:
          if ( aa->authenticationAlgorithmsEnable ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 2 ) * sizeof ( oid ));
  *length = vp->namelen + 2;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11AUTHENTICATIONALGORITHM:
      return ( UCHAR * ) &aa->authenticationAlgorithm;
        
    case DOT11AUTHENTICATIONALGORITHMSENABLE:
//    *write_method = write_dot11AuthenticationAlgorithmsEnable;
      return ( UCHAR * ) &aa->authenticationAlgorithmsEnable;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*  var_dot11WEPDefaultKeysTable() -                                         *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11WEPDefaultKeysTable ( struct variable *vp,
                                oid     *name,
                                size_t  *length,
                                int     exact,
                                size_t  *var_len,
                                WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &dfList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    df = ( struct dfTbl_data * ) np->data;
    rName[vp->namelen + 0] = df->ifIndex;
    rName[vp->namelen + 1] = df->WEPDefaultKeyIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11WEPDEFAULTKEYVALUE:
          if ( df->haveWEPDefaultKeyValue ) found = TRUE; break;
      }          
    }          
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 2 ) * sizeof ( oid ));
  *length = vp->namelen + 2;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11WEPDEFAULTKEYVALUE:
//    *write_method = write_dot11WEPDefaultKeyValue;
      *var_len = strlen ( df->WEPDefaultKeyValue );
      return ( UCHAR * ) df->WEPDefaultKeyValue;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*  var_dot11WEPKeyMappingsTable() -                                         *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11WEPKeyMappingsTable ( struct variable *vp,
                                oid     *name,
                                size_t  *length,
                                int     exact,
                                size_t  *var_len,
                                WriteMethod **write_method)
{
  static char MACWork[17];
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &kmList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    km = ( struct kmTbl_data * ) np->data;
    rName[vp->namelen + 0] = km->ifIndex;
    rName[vp->namelen + 1] = km->WEPKeyMappingIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11WEPKEYMAPPINGADDRESS:
          if ( km->haveWEPKeyMappingAddress ) found = TRUE; break;
        case DOT11WEPKEYMAPPINGWEPON:
          if ( km->haveWEPKeyMappingWEPOn   ) found = TRUE; break;
        case DOT11WEPKEYMAPPINGVALUE:
          if ( km->haveWEPKeyMappingValue   ) found = TRUE; break;
        case DOT11WEPKEYMAPPINGSTATUS:
          if ( km->haveWEPKeyMappingStatus  ) found = TRUE; break; 
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 2 ) * sizeof ( oid ));
  *length = vp->namelen + 2;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11WEPKEYMAPPINGADDRESS:
//    *write_method = write_dot11WEPKeyMappingAddress;
      MACWork[ 0] = km->WEPKeyMappingAddress[ 0];
      MACWork[ 1] = km->WEPKeyMappingAddress[ 1];
      MACWork[ 2] = km->WEPKeyMappingAddress[ 3];
      MACWork[ 3] = km->WEPKeyMappingAddress[ 4];
      MACWork[ 4] = km->WEPKeyMappingAddress[ 6];
      MACWork[ 5] = km->WEPKeyMappingAddress[ 7];
      MACWork[ 6] = km->WEPKeyMappingAddress[ 9];
      MACWork[ 7] = km->WEPKeyMappingAddress[10];
      MACWork[ 8] = km->WEPKeyMappingAddress[12];
      MACWork[ 9] = km->WEPKeyMappingAddress[13];
      MACWork[10] = km->WEPKeyMappingAddress[15];
      MACWork[11] = km->WEPKeyMappingAddress[16];
      MACWork[12] = '\0';
      *var_len = 6;
      return ( UCHAR * ) htob ( MACWork );

    case DOT11WEPKEYMAPPINGWEPON:
//    *write_method = write_dot11WEPKeyMappingWEPOn;
      return ( UCHAR * ) &km->WEPKeyMappingWEPOn;

    case DOT11WEPKEYMAPPINGVALUE:
//    *write_method = write_dot11WEPKeyMappingValue;
      *var_len = strlen ( km->WEPKeyMappingValue );
      return ( UCHAR * ) km->WEPKeyMappingValue;

    case DOT11WEPKEYMAPPINGSTATUS:
//    *write_method = write_dot11WEPKeyMappingStatus;
      return ( UCHAR * ) &km->WEPKeyMappingStatus;

    default:
      ERROR_MSG ( "" );
  }
  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_dot11PrivacyTable() -                                               *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11PrivacyTable ( struct variable *vp,
                        oid     *name,
                        size_t  *length,
                        int     exact,
                        size_t  *var_len,
                        WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &prList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    pr = ( struct prTbl_data * ) np->data;
    rName[vp->namelen] = pr->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11PRIVACYINVOKED:
          if ( pr->havePrivacyInvoked      ) found = TRUE; break;
        case DOT11WEPDEFAULTKEYID:
          if ( pr->haveWEPDefaultKeyID     ) found = TRUE; break;
        case DOT11WEPKEYMAPPINGLENGTH:
          if ( pr->haveWEPKeyMappingLength ) found = TRUE; break;
        case DOT11EXCLUDEUNENCRYPTED:
          if ( pr->haveExcludeUnencrypted  ) found = TRUE; break;
        case DOT11WEPICVERRORCOUNT:
          if ( pr->haveWEPICVErrorCount    ) found = TRUE; break;
        case DOT11WEPEXCLUDEDCOUNT:
          if ( pr->haveWEPExcludedCount    ) found = TRUE; break;
      }      
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11PRIVACYINVOKED:
//    *write_method = write_dot11PrivacyInvoked;
      return ( UCHAR * ) &pr->privacyInvoked;

    case DOT11WEPDEFAULTKEYID:
//    *write_method = write_dot11WEPDefaultKeyID;
      return ( UCHAR * ) &pr->WEPDefaultKeyID;

    case DOT11WEPKEYMAPPINGLENGTH:
//    *write_method = write_dot11WEPKeyMappingLength;
      return ( UCHAR * ) &pr->WEPKeyMappingLength;

    case DOT11EXCLUDEUNENCRYPTED:
//    *write_method = write_dot11ExcludeUnencrypted;
      return ( UCHAR * ) &pr->excludeUnencrypted;

    case DOT11WEPICVERRORCOUNT:
      return ( UCHAR * ) &pr->WEPICVErrorCount;
        
    case DOT11WEPEXCLUDEDCOUNT:
      return ( UCHAR * ) &pr->WEPExcludedCount;
        
    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_dot11OperationTable() -                                             *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11OperationTable ( struct variable *vp,
                          oid     *name,
                          size_t  *length,
                          int     exact,
                          size_t  *var_len,
                          WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned
  static char MACWork[17];

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &opList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    op = ( struct opTbl_data * ) np->data;
    rName[vp->namelen] = op->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {

      switch ( vp->magic ) {      // found requested OID, now check for requested variable
        case DOT11MACADDRESS:             
          if ( op->haveMACAddress              ) found = TRUE; break;
        case DOT11RTSTHRESHOLD:           
          if ( op->haveRTSThreshold            ) found = TRUE; break;
        case DOT11SHORTRETRYLIMIT: 
          if ( op->haveShortRetryLimit         ) found = TRUE; break;
        case DOT11LONGRETRYLIMIT:
          if ( op->haveLongRetryLimit          ) found = TRUE; break;
        case DOT11FRAGMENTATIONTHRESHOLD: 
          if ( op->haveFragmentationThreshold  ) found = TRUE; break;
        case DOT11MAXTRANSMITMSDULIFETIME: 
          if ( op->haveMaxTransmitMSDULifetime ) found = TRUE; break;
        case DOT11MAXRECEIVELIFETIME:
          if ( op->haveMaxReceiveLifetime      ) found = TRUE; break;
        case DOT11MANUFACTURERID:
          if ( op->haveManufacturerID          ) found = TRUE; break;
        case DOT11PRODUCTID:
          if ( op->haveProductID               ) found = TRUE; break;
      }
    }
    if ( found ) 
      break;
  }

  if ( !found )
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11MACADDRESS:
      MACWork[ 0] = op->MACAddress[ 0];
      MACWork[ 1] = op->MACAddress[ 1];
      MACWork[ 2] = op->MACAddress[ 3];
      MACWork[ 3] = op->MACAddress[ 4];
      MACWork[ 4] = op->MACAddress[ 6];
      MACWork[ 5] = op->MACAddress[ 7];
      MACWork[ 6] = op->MACAddress[ 9];
      MACWork[ 7] = op->MACAddress[10];
      MACWork[ 8] = op->MACAddress[12];
      MACWork[ 9] = op->MACAddress[13];
      MACWork[10] = op->MACAddress[15];
      MACWork[11] = op->MACAddress[16];
      MACWork[12] = '\0';
      *var_len = 6;
      return ( UCHAR * ) htob ( MACWork );
        
    case DOT11RTSTHRESHOLD:
//    *write_method = write_dot11RTSThreshold;
      return ( UCHAR * ) &op->RTSThreshold;

    case DOT11SHORTRETRYLIMIT:
//    *write_method = write_dot11ShortRetryLimit;
      return ( UCHAR * ) &op->shortRetryLimit;

    case DOT11LONGRETRYLIMIT:
//    *write_method = write_dot11LongRetryLimit;
      return ( UCHAR * ) &op->longRetryLimit;

    case DOT11FRAGMENTATIONTHRESHOLD:
//    *write_method = write_dot11FragmentationThreshold;
      return ( UCHAR * ) &op->fragmentationThreshold;

    case DOT11MAXTRANSMITMSDULIFETIME:
//    *write_method = write_dot11MaxTransmitMSDULifetime;
      return ( UCHAR * ) &op->maxTransmitMSDULifetime;

    case DOT11MAXRECEIVELIFETIME:
//    *write_method = write_dot11MaxReceiveLifetime;
      return ( UCHAR * ) &op->maxReceiveLifetime;

    case DOT11MANUFACTURERID:
      *var_len = strlen ( op->manufacturerID );
      return ( UCHAR * ) op->manufacturerID;

    case DOT11PRODUCTID:
      *var_len = strlen ( op->productID );
      return ( UCHAR * ) op->productID;
        
    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_dot11CountersTable() -                                              *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11CountersTable(struct variable *vp,
          oid     *name,
          size_t  *length,
          int     exact,
          size_t  *var_len,
          WriteMethod **write_method)
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &coList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    co = ( struct coTbl_data * ) np->data;
    rName[vp->namelen] = co->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11TRANSMITTEDFRAGMENTCOUNT:
          if ( co->haveTransmittedFragmentCount    ) found = TRUE; break;
        case DOT11MULTICASTTRANSMITTEDFRAMECOUNT:
          if ( co->haveTransmittedFrameCount       ) found = TRUE; break;
        case DOT11FAILEDCOUNT:
          if ( co->haveFailedCount                 ) found = TRUE; break;
        case DOT11RETRYCOUNT:
          if ( co->haveRetryCount                  ) found = TRUE; break;
        case DOT11MULTIPLERETRYCOUNT:
          if ( co->haveMultipleRetryCount          ) found = TRUE; break;
        case DOT11FRAMEDUPLICATECOUNT:
          if ( co->haveFrameDuplicateCount         ) found = TRUE; break;
        case DOT11RTSSUCCESSCOUNT:
          if ( co->haveRTSSuccessCount             ) found = TRUE; break;
        case DOT11RTSFAILURECOUNT:
          if ( co->haveRTSFailureCount             ) found = TRUE; break;
        case DOT11ACKFAILURECOUNT:
          if ( co->haveACKFailureCount             ) found = TRUE; break;
        case DOT11RECEIVEDFRAGMENTCOUNT:
          if ( co->haveReceivedFragmentCount       ) found = TRUE; break;
        case DOT11MULTICASTRECEIVEDFRAMECOUNT:
          if ( co->haveMulticastReceivedFrameCount ) found = TRUE; break;
        case DOT11FCSERRORCOUNT:
          if ( co->haveFCSErrorCount               ) found = TRUE; break;
        case DOT11TRANSMITTEDFRAMECOUNT:
          if ( co->haveTransmittedFrameCount       ) found = TRUE; break;
        case DOT11WEPUNDECRYPTABLECOUNT:
          if ( co->haveWEPUndecryptableCount       ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11TRANSMITTEDFRAGMENTCOUNT:       return ( UCHAR * ) &co->transmittedFragmentCount;
    case DOT11MULTICASTTRANSMITTEDFRAMECOUNT: return ( UCHAR * ) &co->transmittedFrameCount;
    case DOT11FAILEDCOUNT:                    return ( UCHAR * ) &co->failedCount;
    case DOT11RETRYCOUNT:                     return ( UCHAR * ) &co->retryCount;
    case DOT11MULTIPLERETRYCOUNT:             return ( UCHAR * ) &co->multipleRetryCount;
    case DOT11FRAMEDUPLICATECOUNT:            return ( UCHAR * ) &co->frameDuplicateCount;
    case DOT11RTSSUCCESSCOUNT:                return ( UCHAR * ) &co->RTSSuccessCount;
    case DOT11RTSFAILURECOUNT:                return ( UCHAR * ) &co->RTSFailureCount;
    case DOT11ACKFAILURECOUNT:                return ( UCHAR * ) &co->ACKFailureCount;
    case DOT11RECEIVEDFRAGMENTCOUNT:          return ( UCHAR * ) &co->receivedFragmentCount;
    case DOT11MULTICASTRECEIVEDFRAMECOUNT:    return ( UCHAR * ) &co->multicastReceivedFrameCount;
    case DOT11FCSERRORCOUNT:                  return ( UCHAR * ) &co->FCSErrorCount;
    case DOT11TRANSMITTEDFRAMECOUNT:          return ( UCHAR * ) &co->transmittedFrameCount;
    case DOT11WEPUNDECRYPTABLECOUNT:          return ( UCHAR * ) &co->WEPUndecryptableCount;
        
    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_dot11GroupAddressesTable() -                                        *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11GroupAddressesTable(struct variable *vp,
          oid     *name,
          size_t  *length,
          int     exact,
          size_t  *var_len,
          WriteMethod **write_method)
{
  static char MACWork[17];
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &gaList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    ga = ( struct gaTbl_data * ) np->data;
    rName[vp->namelen + 0] = ga->ifIndex;
    rName[vp->namelen + 1] = ga->groupAddressesIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11ADDRESS:
          if ( ga->haveAddress              ) found = TRUE; break;
        case DOT11GROUPADDRESSESSTATUS:
          if ( ga->haveGroupAddressesStatus ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 2 ) * sizeof ( oid ));
  *length = vp->namelen + 2;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11ADDRESS:
//    *write_method = write_dot11Address;
      MACWork[ 0] = ga->address[ 0];
      MACWork[ 1] = ga->address[ 1];
      MACWork[ 2] = ga->address[ 3];
      MACWork[ 3] = ga->address[ 4];
      MACWork[ 4] = ga->address[ 6];
      MACWork[ 5] = ga->address[ 7];
      MACWork[ 6] = ga->address[ 9];
      MACWork[ 7] = ga->address[10];
      MACWork[ 8] = ga->address[12];
      MACWork[ 9] = ga->address[13];
      MACWork[10] = ga->address[15];
      MACWork[11] = ga->address[16];
      MACWork[12] = '\0';
      *var_len = 6;
      return ( UCHAR * ) htob ( MACWork );

    case DOT11GROUPADDRESSESSTATUS:
//    *write_method = write_dot11GroupAddressesStatus;
      return ( UCHAR * ) &ga->groupAddressesStatus;

    default:
      ERROR_MSG ( "" );
  }
  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_dot11ResourceInfoTable() -                                          *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11ResourceInfoTable ( struct variable *vp,
                              oid     *name,
                              size_t  *length,
                              int     exact,
                              size_t  *var_len,
                              WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &riList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    ri = ( struct riTbl_data * ) np->data;
    rName[vp->namelen] = ri->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11MANUFACTUREROUI:
          if ( ri->haveManufacturerOUI            ) found = TRUE; break;
        case DOT11MANUFACTURERNAME:
          if ( ri->haveManufacturerName           ) found = TRUE; break;
        case DOT11MANUFACTURERPRODUCTNAME:
          if ( ri->haveManufacturerProductName    ) found = TRUE; break;
        case DOT11MANUFACTURERPRODUCTVERSION:
          if ( ri->haveManufacturerProductVersion ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11MANUFACTUREROUI:
      *var_len = strlen ( ri->manufacturerOUI );
      return ( UCHAR * ) ri->manufacturerOUI;
        
    case DOT11MANUFACTURERNAME:
      *var_len = strlen ( ri->manufacturerName );
      return ( UCHAR * ) ri->manufacturerName;
        
    case DOT11MANUFACTURERPRODUCTNAME:
      *var_len = strlen ( ri->manufacturerProductName );
      return ( UCHAR * ) ri->manufacturerProductName;
        
    case DOT11MANUFACTURERPRODUCTVERSION:
      *var_len = strlen ( ri->manufacturerProductVersion );
      return ( UCHAR * ) ri->manufacturerProductVersion;
        
    default: 
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_dot11PhyOperationTable() -                                          *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11PhyOperationTable ( struct variable *vp,
                              oid     *name,
                              size_t  *length,
                              int     exact,
                              size_t  *var_len,
                              WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &poList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    po = ( struct poTbl_data * ) np->data;
    rName[vp->namelen] = po->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11PHYTYPE:
          if ( po->havePHYType          ) found = TRUE; break;
        case DOT11CURRENTREGDOMAIN:
          if ( po->haveCurrentRegDomain ) found = TRUE; break;
        case DOT11TEMPTYPE:
          if ( po->haveTempType         ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11PHYTYPE:
      return ( UCHAR * ) &po->PHYType;
        
    case DOT11CURRENTREGDOMAIN:
//    *write_method = write_dot11CurrentRegDomain;
      return ( UCHAR * ) &po->currentRegDomain;

    case DOT11TEMPTYPE:
      return ( UCHAR * ) &po->tempType;
        
    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_dot11PhyAntennaTable() -                                            *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11PhyAntennaTable ( struct variable *vp,
                            oid     *name,
                            size_t  *length,
                            int     exact,
                            size_t  *var_len,
                            WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &paList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    pa = ( struct paTbl_data * ) np->data;
    rName[vp->namelen] = pa->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11CURRENTTXANTENNA:
          if ( pa->haveCurrentTxAntenna ) found = TRUE; break;
        case DOT11DIVERSITYSUPPORT:
          if ( pa->haveDiversitySupport ) found = TRUE; break;
        case DOT11CURRENTRXANTENNA:
          if ( pa->haveCurrentRxAntenna ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11CURRENTTXANTENNA:
//    *write_method = write_dot11CurrentTxAntenna;
      return ( UCHAR * ) &pa->currentTxAntenna;

    case DOT11DIVERSITYSUPPORT:
      return ( UCHAR * ) &pa->diversitySupport;
        
    case DOT11CURRENTRXANTENNA:
//    *write_method = write_dot11CurrentRxAntenna;
      return ( UCHAR * ) &pa->currentRxAntenna;

    default:
      ERROR_MSG ( "" );
  }
  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_dot11PhyTxPowerTable() -                                            *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11PhyTxPowerTable ( struct variable *vp,
                            oid     *name,
                            size_t  *length,
                            int     exact,
                            size_t  *var_len,
                            WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &ptList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    pt = ( struct ptTbl_data * ) np->data;
    rName[vp->namelen] = pt->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11NUMBERSUPPORTEDPOWERLEVELS:
          if ( pt->haveNumberSupportedPowerLevels ) found = TRUE; break;
        case DOT11TXPOWERLEVEL1:
          if ( pt->haveTxPowerLevel1   ) found = TRUE; break;
        case DOT11TXPOWERLEVEL2:
          if ( pt->haveTxPowerLevel2   ) found = TRUE; break;
        case DOT11TXPOWERLEVEL3:
          if ( pt->haveTxPowerLevel3   ) found = TRUE; break;
        case DOT11TXPOWERLEVEL4:
          if ( pt->haveTxPowerLevel4   ) found = TRUE; break;
        case DOT11TXPOWERLEVEL5:
          if ( pt->haveTxPowerLevel5   ) found = TRUE; break;
        case DOT11TXPOWERLEVEL6:
          if ( pt->haveTxPowerLevel6   ) found = TRUE; break;
        case DOT11TXPOWERLEVEL7:
          if ( pt->haveTxPowerLevel7   ) found = TRUE; break;
        case DOT11TXPOWERLEVEL8:
          if ( pt->haveTxPowerLevel8   ) found = TRUE; break;
        case DOT11CURRENTTXPOWERLEVEL:
          if ( pt->currentTxPowerLevel ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11NUMBERSUPPORTEDPOWERLEVELS: 
      return ( UCHAR * ) &pt->numberSupportedPowerLevels;

    case DOT11TXPOWERLEVEL1: return ( UCHAR * ) &pt->TxPowerLevel1;
    case DOT11TXPOWERLEVEL2: return ( UCHAR * ) &pt->TxPowerLevel2;
    case DOT11TXPOWERLEVEL3: return ( UCHAR * ) &pt->TxPowerLevel3;
    case DOT11TXPOWERLEVEL4: return ( UCHAR * ) &pt->TxPowerLevel4;
    case DOT11TXPOWERLEVEL5: return ( UCHAR * ) &pt->TxPowerLevel5;
    case DOT11TXPOWERLEVEL6: return ( UCHAR * ) &pt->TxPowerLevel6;
    case DOT11TXPOWERLEVEL7: return ( UCHAR * ) &pt->TxPowerLevel7;
    case DOT11TXPOWERLEVEL8: return ( UCHAR * ) &pt->TxPowerLevel8;
        
    case DOT11CURRENTTXPOWERLEVEL:
//    *write_method = write_dot11CurrentTxPowerLevel;
      return ( UCHAR * ) &pt->currentTxPowerLevel;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*     var_dot11PhyFHSSTable() -                                             *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11PhyFHSSTable ( struct variable *vp,
                        oid     *name,
                        size_t  *length,
                        int     exact,
                        size_t  *var_len,
                        WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &pfList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    pf = ( struct pfTbl_data * ) np->data;
    rName[vp->namelen] = pf->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11HOPTIME:
          if ( pf->haveHopTime              ) found = TRUE; break;
        case DOT11CURRENTCHANNELNUMBER:
          if ( pf->haveCurrentChannelNumber ) found = TRUE; break;
        case DOT11MAXDWELLTIME:
          if ( pf->haveMaxDwellTime         ) found = TRUE; break;
        case DOT11CURRENTDWELLTIME:
          if ( pf->haveCurrentDwellTime     ) found = TRUE; break;
        case DOT11CURRENTSET:
          if ( pf->haveCurrentSet           ) found = TRUE; break;
        case DOT11CURRENTPATTERN:
          if ( pf->haveCurrentPattern       ) found = TRUE; break;
        case DOT11CURRENTINDEX:
          if ( pf->haveCurrentIndex         ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11HOPTIME:
      return ( UCHAR * ) &pf->hopTime;
        
    case DOT11CURRENTCHANNELNUMBER:
//    *write_method = write_dot11CurrentChannelNumber;
      return ( UCHAR * ) &pf->currentChannelNumber;

    case DOT11MAXDWELLTIME:
      return ( UCHAR * ) &pf->maxDwellTime;
        
    case DOT11CURRENTDWELLTIME:
//    *write_method = write_dot11CurrentDwellTime;
      return ( UCHAR * ) &pf->currentDwellTime;

    case DOT11CURRENTSET:
//    *write_method = write_dot11CurrentSet;
      return ( UCHAR * ) &pf->currentSet;

    case DOT11CURRENTPATTERN:
//    *write_method = write_dot11CurrentPattern;
      return ( UCHAR * ) &pf->currentPattern;

    case DOT11CURRENTINDEX:
//    *write_method = write_dot11CurrentIndex;
      return ( UCHAR * ) &pf->currentIndex;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*     var_dot11PhyDSSSTable() -                                             *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11PhyDSSSTable ( struct variable *vp,
                        oid     *name,
                        size_t  *length,
                        int     exact,
                        size_t  *var_len,
                        WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &pdList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    pd = ( struct pdTbl_data * ) np->data;
    rName[vp->namelen] = pd->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11CURRENTCHANNEL:
          if ( pd->haveCurrentChannel   ) found = TRUE; break;
        case DOT11CCAMODESUPPORTED:
          if ( pd->haveCCAModeSupported ) found = TRUE; break;
        case DOT11CURRENTCCAMODE:
          if ( pd->haveCurrentCCAMode   ) found = TRUE; break;
        case DOT11EDTHRESHOLD:
          if ( pd->haveEDThreshold      ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11CURRENTCHANNEL:
//    *write_method = write_dot11CurrentChannel;
      return ( UCHAR * ) &pd->currentChannel;

    case DOT11CCAMODESUPPORTED:
      return ( UCHAR * ) &pd->CCAModeSupported;
        
    case DOT11CURRENTCCAMODE:
//    *write_method = write_dot11CurrentCCAMode;
      return ( UCHAR * ) &pd->currentCCAMode;

    case DOT11EDTHRESHOLD:
//    *write_method = write_dot11EDThreshold;
      return ( UCHAR * ) &pd->EDThreshold;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*     var_dot11PhyIRTable() -                                             *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11PhyIRTable ( struct variable *vp,
                      oid     *name,
                      size_t  *length,
                      int     exact,
                      size_t  *var_len,
                      WriteMethod **write_method)
{

  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &piList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    pi = ( struct piTbl_data * ) np->data;
    rName[vp->namelen] = pi->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11CCAWATCHDOGTIMERMAX:
          if ( pi->CCAWatchdogTimerMax ) found = TRUE; break;
        case DOT11CCAWATCHDOGCOUNTMAX:
          if ( pi->CCAWatchdogCountMax ) found = TRUE; break;
        case DOT11CCAWATCHDOGTIMERMIN:
          if ( pi->CCAWatchdogTimerMin ) found = TRUE; break;
        case DOT11CCAWATCHDOGCOUNTMIN:
          if ( pi->CCAWatchdogCountMin ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11CCAWATCHDOGTIMERMAX:
//    *write_method = write_dot11CCAWatchdogTimerMax;
      return ( UCHAR * ) &pi->CCAWatchdogTimerMax;

    case DOT11CCAWATCHDOGCOUNTMAX:
//   *write_method = write_dot11CCAWatchdogCountMax;
      return ( UCHAR * ) &pi->CCAWatchdogCountMax;

    case DOT11CCAWATCHDOGTIMERMIN:
//    *write_method = write_dot11CCAWatchdogTimerMin;
      return ( UCHAR * ) &pi->CCAWatchdogTimerMin;

    case DOT11CCAWATCHDOGCOUNTMIN:
//    *write_method = write_dot11CCAWatchdogCountMin;
      return ( UCHAR * ) &pi->CCAWatchdogCountMin;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*     var_dot11RegDomainsSupportedTable() -                                 *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11RegDomainsSupportedTable ( struct variable *vp,
                                    oid     *name,
                                    size_t  *length,
                                    int     exact,
                                    size_t  *var_len,
                                    WriteMethod **write_method)
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &rdList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    rd = ( struct rdTbl_data * ) np->data;
    rName[vp->namelen + 0] = rd->ifIndex;
    rName[vp->namelen + 1] = rd->regDomainsSupportIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11REGDOMAINSSUPPORTVALUE:
          if ( rd->haveRegDomainsSupportValue ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 2 ) * sizeof ( oid ));
  *length = vp->namelen + 2;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11REGDOMAINSSUPPORTVALUE:
      return ( UCHAR * ) &rd->regDomainsSupportValue;
        
    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*     var_dot11AntennasListTable() -                                        *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11AntennasListTable(struct variable *vp,
          oid     *name,
          size_t  *length,
          int     exact,
          size_t  *var_len,
          WriteMethod **write_method)
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &alList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    al = ( struct alTbl_data * ) np->data;
    rName[vp->namelen + 0] = al->ifIndex;
    rName[vp->namelen + 1] = al->antennaListIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11SUPPORTEDTXANTENNA:
          if ( al->haveSupportedTxAntenna   ) found = TRUE; break;
        case DOT11SUPPORTEDRXANTENNA:
          if ( al->haveSupportedRxAntenna   ) found = TRUE; break;
        case DOT11DIVERSITYSELECTIONRX:
          if ( al->haveDiversitySelectionRx ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 2 ) * sizeof ( oid ));
  *length = vp->namelen + 2;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11SUPPORTEDTXANTENNA:
//    *write_method = write_dot11SupportedTxAntenna;
      return ( UCHAR * ) &al->supportedTxAntenna;

    case DOT11SUPPORTEDRXANTENNA:
//    *write_method = write_dot11SupportedRxAntenna;
      return ( UCHAR * ) &al->supportedRxAntenna;

    case DOT11DIVERSITYSELECTIONRX:
//    *write_method = write_dot11DiversitySelectionRx;
      return ( UCHAR * ) &al->diversitySelectionRx;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*     var_dot11SupportedDataRatesTxTable() -                                *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11SupportedDataRatesTxTable ( struct variable *vp,
                                      oid     *name,
                                      size_t  *length,
                                      int     exact,
                                      size_t  *var_len,
                                      WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &rtList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    rt = ( struct rtTbl_data * ) np->data;
    rName[vp->namelen + 0] = rt->ifIndex;
    rName[vp->namelen + 1] = rt->supportedDataRatesTxIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
         case DOT11SUPPORTEDDATARATESTXVALUE:
          if ( rt->haveSupportedDataRatesTxValue ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 2 ) * sizeof ( oid ));
  *length = vp->namelen + 2;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11SUPPORTEDDATARATESTXVALUE:
      return ( UCHAR * ) &rt->supportedDataRatesTxValue;
        
    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*     var_dot11SupportedDataRatesRxTable() -                                *
*                                                                           *
****************************************************************************/
unsigned char *
var_dot11SupportedDataRatesRxTable ( struct variable *vp,
                                      oid     *name,
                                      size_t  *length,
                                      int     exact,
                                      size_t  *var_len,
                                      WriteMethod **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &rrList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    rr = ( struct rrTbl_data * ) np->data;
    rName[vp->namelen + 0] = rr->ifIndex;
    rName[vp->namelen + 1] = rr->supportedDataRatesRxIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 2, name, *length ) >  0 ))) {
      switch ( vp->magic ) {
        case DOT11SUPPORTEDDATARATESRXVALUE:
          if ( rr->haveSupportedDataRatesRxValue ) found = TRUE; break;
      }
    }
    if ( found )
      break;
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 2 ) * sizeof ( oid ));
  *length = vp->namelen + 2;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case DOT11SUPPORTEDDATARATESRXVALUE:
      return ( UCHAR * ) &rr->supportedDataRatesRxValue;
        
    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11StationID(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static unsigned char string[SPRINT_MAX_LEN];
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_OCTET_STR ) {
        fprintf ( stderr, "write to dot11StationID not ASN_OCTET_STR\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( string )) {
        fprintf ( stderr,"write to dot11StationID: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11MediumOccupancyLimit(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11MediumOccupancyLimit not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11MediumOccupancyLimit: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CFPPeriod(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CFPPeriod not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CFPPeriod: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CFPMaxDuration(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CFPMaxDuration not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CFPMaxDuration: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11AuthenticationResponseTimeOut(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11AuthenticationResponseTimeOut not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11AuthenticationResponseTimeOut: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11PowerManagementMode(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11PowerManagementMode not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )) {
        fprintf ( stderr, "write to dot11PowerManagementMode: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11DesiredSSID(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static unsigned char string[SPRINT_MAX_LEN];
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_OCTET_STR ) {
        fprintf ( stderr, "write to dot11DesiredSSID not ASN_OCTET_STR\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( string )){
        fprintf ( stderr, "write to dot11DesiredSSID: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11DesiredBSSType(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11DesiredBSSType not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11DesiredBSSType: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11OperationalRateSet(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static unsigned char string[SPRINT_MAX_LEN];
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_OCTET_STR ) {
        fprintf ( stderr, "write to dot11OperationalRateSet not ASN_OCTET_STR\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( string )){
        fprintf ( stderr, "write to dot11OperationalRateSet: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11BeaconPeriod(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11BeaconPeriod not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11BeaconPeriod: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11DTIMPeriod(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11DTIMPeriod not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11DTIMPeriod: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11AssociationResponseTimeOut(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11AssociationResponseTimeOut not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )) {
        fprintf ( stderr,"write to dot11AssociationResponseTimeOut: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11AuthenticationAlgorithmsEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11AuthenticationAlgorithmsEnable not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11AuthenticationAlgorithmsEnable: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11WEPDefaultKeyValue(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static unsigned char string[SPRINT_MAX_LEN];
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_OCTET_STR ) {
        fprintf ( stderr, "write to dot11WEPDefaultKeyValue not ASN_OCTET_STR\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( string )){
        fprintf ( stderr,"write to dot11WEPDefaultKeyValue: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11WEPKeyMappingAddress(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static unsigned char string[SPRINT_MAX_LEN];
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_OCTET_STR ) {
        fprintf ( stderr, "write to dot11WEPKeyMappingAddress not ASN_OCTET_STR\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( string )) {
        fprintf ( stderr,"write to dot11WEPKeyMappingAddress: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11WEPKeyMappingWEPOn(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11WEPKeyMappingWEPOn not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11WEPKeyMappingWEPOn: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11WEPKeyMappingValue(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static unsigned char string[SPRINT_MAX_LEN];
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_OCTET_STR ) {
        fprintf ( stderr, "write to dot11WEPKeyMappingValue not ASN_OCTET_STR\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( string )) {
        fprintf ( stderr, "write to dot11WEPKeyMappingValue: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11WEPKeyMappingStatus(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {
    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11WEPKeyMappingStatus not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11WEPKeyMappingStatus: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11PrivacyInvoked(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11PrivacyInvoked not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11PrivacyInvoked: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11WEPDefaultKeyID(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11WEPDefaultKeyID not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11WEPDefaultKeyID: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11WEPKeyMappingLength(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11WEPKeyMappingLength not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11WEPKeyMappingLength: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11ExcludeUnencrypted(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11ExcludeUnencrypted not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11ExcludeUnencrypted: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11RTSThreshold(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ){
        fprintf ( stderr, "write to dot11RTSThreshold not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11RTSThreshold: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11ShortRetryLimit(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11ShortRetryLimit not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11ShortRetryLimit: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11LongRetryLimit(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11LongRetryLimit not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11LongRetryLimit: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11FragmentationThreshold(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11FragmentationThreshold not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11FragmentationThreshold: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11MaxTransmitMSDULifetime(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11MaxTransmitMSDULifetime not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11MaxTransmitMSDULifetime: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:

      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11MaxReceiveLifetime(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11MaxReceiveLifetime not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11MaxReceiveLifetime: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11Address(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static unsigned char string[SPRINT_MAX_LEN];
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_OCTET_STR ) {
        fprintf ( stderr, "write to dot11Address not ASN_OCTET_STR\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( string )){
        fprintf ( stderr, "write to dot11Address: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11GroupAddressesStatus(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11GroupAddressesStatus not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11GroupAddressesStatus: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentRegDomain(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentRegDomain not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CurrentRegDomain: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentTxAntenna(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentTxAntenna not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CurrentTxAntenna: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentRxAntenna(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentRxAntenna not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11CurrentRxAntenna: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;

  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentTxPowerLevel(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentTxPowerLevel not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CurrentTxPowerLevel: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentChannelNumber(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentChannelNumber not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11CurrentChannelNumber: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentDwellTime(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentDwellTime not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CurrentDwellTime: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentSet(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentSet not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CurrentSet: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentPattern(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentPattern not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CurrentPattern: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentIndex(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentIndex not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CurrentIndex: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentChannel(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentChannel not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CurrentChannel: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CurrentCCAMode(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CurrentCCAMode not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11CurrentCCAMode: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11EDThreshold(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11EDThreshold not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11EDThreshold: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CCAWatchdogTimerMax(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CCAWatchdogTimerMax not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CCAWatchdogTimerMax: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CCAWatchdogCountMax(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CCAWatchdogCountMax not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CCAWatchdogCountMax: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CCAWatchdogTimerMin(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CCAWatchdogTimerMin not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CCAWatchdogTimerMin: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11CCAWatchdogCountMin(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11CCAWatchdogCountMin not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11CCAWatchdogCountMin: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11SupportedTxAntenna(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11SupportedTxAntenna not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11SupportedTxAntenna: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11SupportedRxAntenna(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11SupportedRxAntenna not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr,"write to dot11SupportedRxAntenna: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
****************************************************************************/
int
write_dot11DiversitySelectionRx(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
  static long *long_ret;
  int size;

  switch ( action ) {

    case RESERVE1:
      if ( var_val_type != ASN_INTEGER ) {
        fprintf ( stderr, "write to dot11DiversitySelectionRx not ASN_INTEGER\n" );
        return SNMP_ERR_WRONGTYPE;
      }
      if ( var_val_len > sizeof ( long_ret )){
        fprintf ( stderr, "write to dot11DiversitySelectionRx: bad length\n" );
        return SNMP_ERR_WRONGLENGTH;
      }
      break;

    case RESERVE2:
    case FREE:
    case ACTION:
    case UNDO:
      break;

    case COMMIT:
      break;
  }

  return SNMP_ERR_NOERROR;
}

/****************************************************************************
*                                                                           *
*                      loadTables() - Load the Tables                       *
*                                                                           *
****************************************************************************/
static void loadTables()
{
  int skfd;                               // generic raw socket desc
  struct iwreq wrq;                       // ioctl request structure
  struct ifreq ifr;
  struct timeval et;                      // elapsed time
  struct wireless_info info;              // workarea for wireless ioctl information
  FILE *fp;
  char  bfr[1024], ifName[1024];
  char *s, *t;

  gettimeofday ( &et, ( struct timezone * ) 0 );  // get time-of-day
  if ( et.tv_sec < lastLoad + MINLOADFREQ )       // only reload so often
    return;
  lastLoad = et.tv_sec;

  skfd = openSocket();                            // open socket
  if ( skfd < 0 ) {
    syslog ( LOG_ERR, "SNMP ieee802dot11.loadTables() - %s\n", "socket open failure" );
    return;
  }

  flushLists();

  // find interfaces in /proc/net/dev and find the wireless interfaces
  fp = fopen ( PROC_NET_DEV, "r" );
  if ( fp ) {
    while ( fgets ( bfr, sizeof ( bfr ), fp )) {
      if ( strstr ( bfr, ":" )) {
        s = bfr; t = ifName;
        while ( isspace ( *s ))                     // discard white space
          *s++;
        while ( *s != ':' )                         // get interface name
          *t++ = *s++;
        *t = '\0';

        // verify as a wireless device
        memset (( char * ) &info, 0, sizeof ( struct wireless_info ));
        strncpy ( wrq.ifr_name, ifName, IFNAMSIZ );
        if ( ioctl ( skfd, SIOCGIWNAME, &wrq ) >= 0 ) {
          printf ( "%s ifName: %s\n", "loadTables() -", ifName );
          initStructs();
          loadWiExt( skfd, ifName, &info );
          displayWiExt ( info );
          load80211Structs ( skfd, ifName, &info );
        }
      }
    }
    fclose ( fp );
  }

  close ( skfd );
}

/****************************************************************************
*                                                                           *
*              load80211Structs() - load the 802.11 structures              *
*                                                                           *
****************************************************************************/
static void 
load80211Structs ( int skfd, char *ifName, struct wireless_info *wi )
{
  int rc, ifIndex = 0;
  struct ifreq ifr;
  char  MACAddress [ MACADDR_LEN + 1 ];

  strcpy ( ifr.ifr_name, ifName );
  rc = ioctl ( skfd, SIOCGIFHWADDR, &ifr );
  if ( rc >= 0 ) {

    sprintf ( MACAddress, "%02X:%02X:%02X:%02X:%02X:%02X\0", 
                 ( UCHAR ) ifr.ifr_hwaddr.sa_data[0], ( UCHAR ) ifr.ifr_hwaddr.sa_data[1], 
                 ( UCHAR ) ifr.ifr_hwaddr.sa_data[2], ( UCHAR ) ifr.ifr_hwaddr.sa_data[3], 
                 ( UCHAR ) ifr.ifr_hwaddr.sa_data[4], ( UCHAR ) ifr.ifr_hwaddr.sa_data[5] );

    nSc.haveStationID = TRUE;
    strcpy  ( nSc.stationID, MACAddress );
    nOp.haveMACAddress = TRUE;
    strcpy  ( nOp.MACAddress, MACAddress );
    nRi.haveManufacturerOUI = TRUE;
    strncpy ( nRi.manufacturerOUI, MACAddress, MAN_OUI_LEN ); 
unsigned int if_nametoindex (const char *);

    ifIndex = if_nametoindex ( ifName );
    if ( !ifIndex ) {
      syslog ( LOG_ERR, "SNMP %s - %s %s\n", 
        "ieee802dot11.load80211Structs()", ifName, "has no ifIndex" );
      return;
    }

    loadWiExtTo80211Structs ( ifIndex, ifName, wi );

    if ( hasChanged (( char * ) &nSc, sizeof ( nSc ))) {
      nSc.ifIndex = ifIndex;
      sprintf ( nSc.UID, "%04d\0", nSc.ifIndex );
      strcpy ( nSc.ifName, ifName );
      addList (( char * ) &scList, ( char * ) &nSc, sizeof ( nSc ));
    }

    if ( hasChanged (( char * ) &nPr, sizeof ( nPr ))) {
      nPr.ifIndex = ifIndex;
      sprintf ( nPr.UID, "%04d\0", nPr.ifIndex );
      strcpy ( nPr.ifName, ifName );
      addList (( char * ) &prList, ( char * ) &nPr, sizeof ( nPr ));
    }

    if ( hasChanged (( char * ) &nOp, sizeof ( nOp ))) {
      nOp.ifIndex = ifIndex;
      sprintf ( nOp.UID, "%04d\0", nOp.ifIndex );
      strcpy ( nOp.ifName, ifName );
      addList (( char * ) &opList, ( char * ) &nOp, sizeof ( nOp ));
    }

    if ( hasChanged (( char * ) &nCo, sizeof ( nCo ))) {
      nCo.ifIndex = ifIndex;
      sprintf ( nCo.UID, "%04d\0", nCo.ifIndex );
      strcpy ( nCo.ifName, ifName );
      addList (( char * ) &coList, ( char * ) &nCo, sizeof ( nCo ));
    }

    if ( hasChanged (( char * ) &nRi, sizeof ( nRi ))) {
      nRi.ifIndex = ifIndex;
      sprintf ( nRi.UID, "%04d\0", nRi.ifIndex );
      strcpy ( nRi.ifName, ifName );
      addList (( char * ) &riList, ( char * ) &nRi, sizeof ( nRi ));
    }

    if ( hasChanged (( char * ) &nPo, sizeof ( nPo ))) {
      nPo.ifIndex = ifIndex;
      sprintf ( nPo.UID, "%04d\0", nPo.ifIndex );
      strcpy ( nPo.ifName, ifName );
      addList (( char * ) &poList, ( char * ) &nPo, sizeof ( nPo ));
    }

    if ( hasChanged (( char * ) &nPa, sizeof ( nPa ))) {
      nPa.ifIndex = ifIndex;
      sprintf ( nPa.UID, "%04d\0", nPa.ifIndex );
      strcpy ( nPa.ifName, ifName );
      addList (( char * ) &paList, ( char * ) &nPa, sizeof ( nPa ));
    }

    if ( hasChanged (( char * ) &nPt, sizeof ( nPt ))) {
      nPt.ifIndex = ifIndex;
      sprintf ( nPt.UID, "%04d\0", nPt.ifIndex );
      strcpy ( nPt.ifName, ifName );
      addList (( char * ) &ptList, ( char * ) &nPt, sizeof ( nPt ));
    }

    if ( hasChanged (( char * ) &nPf, sizeof ( nPf ))) {
      nPf.ifIndex = ifIndex;
      sprintf ( nPf.UID, "%04d\0", nPf.ifIndex );
      strcpy ( nPf.ifName, ifName );
      addList (( char * ) &pfList, ( char * ) &nPf, sizeof ( nPf ));
    }

    if ( hasChanged (( char * ) &nPd, sizeof ( nPd ))) {
      nPd.ifIndex = ifIndex;
      sprintf ( nPd.UID, "%04d\0", nPd.ifIndex );
      strcpy ( nPd.ifName, ifName );
      addList (( char * ) &pdList, ( char * ) &nPd, sizeof ( nPd ));
    }

    if ( hasChanged (( char * ) &nPi, sizeof ( nPi ))) {
      nPi.ifIndex = ifIndex;
      sprintf ( nPi.UID, "%04d\0", nPi.ifIndex );
      strcpy ( nPi.ifName, ifName );
      addList (( char * ) &piList, ( char * ) &nPi, sizeof ( nPi ));
    }
  }

//printf ( "%s - ifIndex: %d ifName: %s UID: %s\n", 
//         "load80211Structs() - HASCHANGED", ifIndex, ifName, nSc.UID );
}

/****************************************************************************
*                                                                           *
*                     initStructs() - initialize structures                 *
*                                                                           *
****************************************************************************/
static void initStructs()
{
  int i;

  // 802.11 MIB Stuctures
  memset (( char * ) &nSc, 0, sizeof ( nSc ));  memset (( char * ) &nAa, 0, sizeof ( nAa ));
  memset (( char * ) &nDf, 0, sizeof ( nDf ));  memset (( char * ) &nKm, 0, sizeof ( nKm ));
  memset (( char * ) &nPr, 0, sizeof ( nPr ));  memset (( char * ) &nOp, 0, sizeof ( nOp ));
  memset (( char * ) &nCo, 0, sizeof ( nCo ));  memset (( char * ) &nGa, 0, sizeof ( nGa ));
  memset (( char * ) &nRi, 0, sizeof ( nRi ));  memset (( char * ) &nPo, 0, sizeof ( nPo ));
  memset (( char * ) &nPa, 0, sizeof ( nPa ));  memset (( char * ) &nPt, 0, sizeof ( nPt ));
  memset (( char * ) &nPf, 0, sizeof ( nPf ));  memset (( char * ) &nPd, 0, sizeof ( nPd ));
  memset (( char * ) &nPi, 0, sizeof ( nPi ));  memset (( char * ) &nRd, 0, sizeof ( nRd ));
  memset (( char * ) &nAl, 0, sizeof ( nAl ));  memset (( char * ) &nRt, 0, sizeof ( nRt ));
  memset (( char * ) &nRr, 0, sizeof ( nRr ));

  // Wireless Extensions
  wepCurrentKey = 0;
  haveWepCurrentKey = FALSE;
  for ( i = 0; i < MAX_WEP_KEYS; i++ ) {
    wep[i].len = 0;
    wep[i].key[0] = '\0';
    wep[i].haveKey = FALSE;
  }
}

/****************************************************************************
*                                                                           *
*                Wireless Extensions Specific Functions                     *
*                                                                           *
****************************************************************************/
/****************************************************************************
*                                                                           *
* loadWiExtTo80211Structs() - load wireless extensions to 802.11 structures *
*                                                                           *
****************************************************************************/
static void 
loadWiExtTo80211Structs ( int ifIndex, char *ifName, struct wireless_info *wi )
{
  int i, j = 0;

  // dot11Smt Group
  // dot11StationConfigTable
  nSc.havePrivacyOptionImplemented = TRUE;
  nSc.privacyOptionImplemented = 1;           // assume we support WEP

  if ( wi->has_power ) {
    nSc.havePowerManagementMode = TRUE;
    nSc.powerManagementMode = 1;              // assume power is active
    if ( !wi->power.disabled && 
          wi->power.flags & IW_POWER_MIN )
      nSc.powerManagementMode = 2;            // power save mode
  }

  if ( wi->has_essid && strlen ( wi->essid )) {
    nSc.haveDesiredSSID = TRUE;
    strcpy ( nSc.desiredSSID, wi->essid ); 
  }

  if ( wi->has_mode ) {
    nSc.haveDesiredBSSType = TRUE;
    if ( wi->mode == IW_MODE_ADHOC ) 
      nSc.desiredBSSType = 2;         // independent
    else if ( wi->has_ap_addr )
      nSc.desiredBSSType = 1;         // infrastructure
    else
      nSc.desiredBSSType = 3;         // any
  }

  if ( wi->has_range ) {
    for ( i = 0; i < wi->range.num_bitrates && j < 126; i++ ) {
      nSc.haveOperationalRateSet = TRUE;
      nSc.operationalRateSet[j++] = ( char ) ( wi->range.bitrate[i] / 500000L );
    }
  }

  // dot11AuthenticationAlgorithmsTable
  nAa.haveAuthenticationAlgorithm = TRUE;           // it's a rule to always have 
  nAa.haveAuthenticationAlgorithmsEnable = TRUE;    //    'open' supported
  nAa.ifIndex = ifIndex;
  nAa.authenticationAlgorithmsIndex = 1;            // index number one
  nAa.authenticationAlgorithm = 1;                  // 1 => open key
  sprintf ( nAa.UID, "%04d%04d\0", nAa.ifIndex, nAa.authenticationAlgorithmsIndex );
  nAa.authenticationAlgorithmsEnable = 1;           // enabled by default
  if ( ( wi->has_key                        ) &&
       ( wi->key_size  != 0                 ) &&
      !( wi->key_flags & IW_ENCODE_DISABLED ))
    nAa.authenticationAlgorithmsEnable = 2;
  addList (( char * ) &aaList, ( char * ) &nAa, sizeof ( nAa ));

  nAa.haveAuthenticationAlgorithm = TRUE;           // I'm gonna assume we always support WEP
  nAa.haveAuthenticationAlgorithmsEnable = TRUE;
  nAa.ifIndex = ifIndex;
  nAa.authenticationAlgorithmsIndex = 2;            // index number 2
  nAa.authenticationAlgorithm = 2;                  // 2 => shared key
  sprintf ( nAa.UID, "%04d%04d\0", nAa.ifIndex, nAa.authenticationAlgorithmsIndex );
  nAa.authenticationAlgorithmsEnable = 2;
  if ( ( wi->has_key                        ) &&
       ( wi->key_size  != 0                 ) &&
      !( wi->key_flags & IW_ENCODE_DISABLED ))
    nAa.authenticationAlgorithmsEnable = 1;         // disabled by default
  addList (( char * ) &aaList, ( char * ) &nAa, sizeof ( nAa ));

  //dot11WEPDefaultKeysTable
  if ( wi->has_range ) {
    for ( i = 0; i < MAX_WEP_KEYS; i++ ) {
      nDf.haveWEPDefaultKeyValue = TRUE;
      nDf.ifIndex = ifIndex;
      nDf.WEPDefaultKeyIndex = i + 1;               // index number
      sprintf ( nDf.UID, "%04d%04d\0", nDf.ifIndex, nDf.WEPDefaultKeyIndex );
      if ( wep[i].haveKey )
        strcpy ( nDf.WEPDefaultKeyValue, "*****" );
      else
        nDf.WEPDefaultKeyValue[0] = '\0';
      addList (( char * ) &dfList, ( char * ) &nDf, sizeof ( nDf ));
    }
  }

  // dot11PrivacyTable
  nPr.havePrivacyInvoked = TRUE;
  nPr.privacyInvoked = 2;                   // 2 => FALSE
  nPr.haveWEPDefaultKeyID = TRUE;
  nPr.WEPDefaultKeyID = 0;
  nPr.haveExcludeUnencrypted = TRUE;
  nPr.excludeUnencrypted = 2;               // 2 => FALSE
  if ( wi->has_range ) {
    if ( ( wi->key_size != 0 ) &&
        !( wi->key_flags & IW_ENCODE_DISABLED )) {
       nPr.privacyInvoked = 1;
       if ( wi->key_flags & IW_ENCODE_RESTRICTED )
          nPr.excludeUnencrypted = 1;
       nPr.WEPDefaultKeyID = wepCurrentKey;
    }
  }

  // dot11Mac Group
  // dot11OperationTable
  if ( wi->has_range ) {
    nOp.haveRTSThreshold = TRUE;
    nOp.RTSThreshold = wi->range.max_rts;
  }

  if ( wi->has_frag && wi->frag.value ) {
    nOp.haveFragmentationThreshold = TRUE;
    nOp.fragmentationThreshold = wi->frag.value;
  }

  // dot11Phy Group
  // dot11PhyOperationTable
  if ( strstr ( wi->name, "IEEE 802.11-FS"      )) nPo.PHYType = 1;   // So what if I
  if ( strstr ( wi->name, "IEEE 802.11-DS"      )) nPo.PHYType = 2;   // made up a couple?
  if ( strstr ( wi->name, "IEEE 802.11-IR"      )) nPo.PHYType = 3;   
  if ( strstr ( wi->name, "IEEE 802.11-OFDM"    )) nPo.PHYType = 4;   // 802.11a
  if ( strstr ( wi->name, "IEEE 802.11-OFDM/DS" )) nPo.PHYType = 5;   // 802.11g
  if ( strstr ( wi->name, "IEEE 802.11-TURBO"   )) nPo.PHYType = 6;   // Atheros TURBO mode
  if ( nPo.PHYType ) nPo.havePHYType = TRUE;

  // dot11PhyDSSSTable
  if ( wi->has_range ) { // && wi->freq <= ( double ) 2483000000 ) {  // DSSS frequencies only
    for ( i = 0; i < wi->range.num_frequency; i++ ) {
      if ((( double ) ( wi->range.freq[i].e * 10 ) * ( double ) wi->range.freq[i].m ) == wi->freq ) {
        nPd.haveCurrentChannel = TRUE;
        nPd.currentChannel = wi->range.freq[i].i; 
      }
    }
  }

  // dot11SupportedDataRatesTxTable
  if ( wi->has_range ) {
    for ( i = 0; i < wi->range.num_bitrates; i++ ) {
      nRt.ifIndex = ifIndex;
      nRt.supportedDataRatesTxIndex = i + 1;
      nRt.supportedDataRatesTxValue = wi->range.bitrate[i] / 500000L;
      nRt.haveSupportedDataRatesTxValue = TRUE;
      sprintf ( nRt.UID, "%04d%04d\0", nRt.ifIndex, nRt.supportedDataRatesTxIndex );
      strcpy ( nRt.ifName, ifName );
      addList (( char * ) &rtList, ( char * ) &nRt, sizeof ( nRt ));
    }
  }

  // dot11SupportedDataRatesRxTable
  if ( wi->has_range ) {
    for ( i = 0; i < wi->range.num_bitrates; i++ ) {
      nRr.ifIndex = ifIndex;
      nRr.supportedDataRatesRxIndex = i + 1;
      nRr.supportedDataRatesRxValue = wi->range.bitrate[i] / 500000L;
      nRr.haveSupportedDataRatesRxValue = TRUE;
      sprintf ( nRr.UID, "%04d%04d\0", nRr.ifIndex, nRr.supportedDataRatesRxIndex );
      strcpy ( nRr.ifName, ifName );
      addList (( char * ) &rrList, ( char * ) &nRr, sizeof ( nRr ));
    }
  }

//printf ( "%s max_encoding_tokens: %d\n", 
//          "loadWiExtTo80211Structs() - ", wi->range.max_encoding_tokens );
}

/****************************************************************************
*                                                                           *
*      loadWiExt() - load wireless extensions structures;                   *
*                    use ioctl calls and read /proc/net/wireless            *
*                                                                           *
****************************************************************************/
static void loadWiExt ( int skfd, char *ifname, struct wireless_info *wi )
{
  struct iwreq wrq;                       // ioctl request structure
  FILE *fp;
  char  bfr[1024];
  char  buffer[sizeof ( iwrange ) * 2]; /* Large enough */
  char *s, *t;
  int i, j;

  strncpy ( wrq.ifr_name, ifname, IFNAMSIZ );

  /* Get wireless name */
  if ( ioctl ( skfd, SIOCGIWNAME, &wrq ) >= 0 ) {
    strncpy ( wi->name, wrq.u.name, IFNAMSIZ );
    wi->name[IFNAMSIZ] = '\0';
  }

  /* Get ranges */    // NOTE: some version checking in iwlib.c
  memset ( buffer, 0, sizeof ( buffer ));
  wrq.u.data.pointer = ( caddr_t ) &buffer;
  wrq.u.data.length = sizeof ( buffer );
  wrq.u.data.flags = 0;
  if ( ioctl ( skfd, SIOCGIWRANGE, &wrq ) >= 0 ) {
    memcpy (( char * ) &wi->range, buffer, sizeof ( iwrange ));
    wi->has_range = 1;
  }

  /* Get network ID */
  if ( ioctl ( skfd, SIOCGIWNWID, &wrq ) >= 0 ) {
    memcpy ( &wi->nwid, &wrq.u.nwid, sizeof ( iwparam ));
    wi->has_nwid = 1;
  }

  /* Get frequency / channel */         // THIS NUMBER LOOKS FUNNY
  if ( ioctl ( skfd, SIOCGIWFREQ, &wrq ) >= 0 ) {
    wi->has_freq = 1;
    wi->freq = (( double ) wrq.u.freq.m ) * pow ( 10, wrq.u.freq.e );
  }

  /* Get sensitivity */
  if ( ioctl ( skfd, SIOCGIWSENS, &wrq ) >= 0 ) {
    wi->has_sens = 1;
    memcpy ( &wi->sens, &wrq.u.sens, sizeof ( iwparam ));
  }

  /* Get encryption information */
  wrq.u.data.pointer = ( caddr_t ) &wi->key;
  wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
  wrq.u.data.flags = 0;
  if ( ioctl ( skfd, SIOCGIWENCODE, &wrq ) >= 0 ) {
    wi->has_key = 1;
    wi->key_size = wrq.u.data.length;
    wi->key_flags = wrq.u.data.flags;
    wepCurrentKey = wrq.u.data.flags & IW_ENCODE_INDEX;
  }

  for ( i = 0; i < wi->range.max_encoding_tokens; i++ ) {
    wrq.u.data.pointer = ( caddr_t ) &wi->key;
    wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
    wrq.u.data.flags = i;
    if ( ioctl ( skfd, SIOCGIWENCODE, &wrq ) >= 0 ) {
      if ( ( wrq.u.data.length != 0 ) &&
          !( wrq.u.data.flags & IW_ENCODE_DISABLED )) {
        wep[i].len = wrq.u.data.length;
        wep[i].haveKey = TRUE;
        t = wep[i].key;
        for ( j = 0; j < wrq.u.data.length; j++ ) {
          if (( j & 0x1 ) == 0 && j != 0 )
        	  strcpy ( t++, "-");
          sprintf ( t, "%.2X", wi->key[j] );
          t += 2;
        }
        t = '\0';
      }
    }
  }

  /* Get ESSID */
  wrq.u.essid.pointer = ( caddr_t ) &wi->essid;
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  wrq.u.essid.flags = 0;
  if ( ioctl ( skfd, SIOCGIWESSID, &wrq ) >= 0 ) {
    wi->has_essid = 1;
    wi->essid_on = wrq.u.data.flags;
  }

  /* Get AP address */
  if ( ioctl ( skfd, SIOCGIWAP, &wrq ) >= 0 ) {
    wi->has_ap_addr = 1;
    memcpy ( &wi->ap_addr, &wrq.u.ap_addr, sizeof ( sockaddr ));
  }

  /* Get NickName */
  wrq.u.essid.pointer = ( caddr_t ) &wi->nickname;
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  wrq.u.essid.flags = 0;
  if ( ioctl ( skfd, SIOCGIWNICKN, &wrq ) >= 0 ) {
    if ( wrq.u.data.length > 1 )
      wi->has_nickname = 1;
  }

  /* Get bit rate */
  if ( ioctl ( skfd, SIOCGIWRATE, &wrq ) >= 0 ) {
    wi->has_bitrate = 1;
    memcpy ( &wi->bitrate, &wrq.u.bitrate, sizeof ( iwparam ));
  }

  /* Get RTS threshold */
  if ( ioctl ( skfd, SIOCGIWRTS, &wrq ) >= 0 ) {
    wi->has_rts = 1;
    memcpy ( &wi->rts, &wrq.u.rts, sizeof ( iwparam ));
  }

  /* Get fragmentation threshold */
  if ( ioctl ( skfd, SIOCGIWFRAG, &wrq ) >= 0 ) {
      wi->has_frag = 1;
      memcpy ( &wi->frag, &wrq.u.frag, sizeof ( iwparam ));
    }

  /* Get operation mode */
  if ( ioctl ( skfd, SIOCGIWMODE, &wrq ) >= 0 ) {
      wi->mode = wrq.u.mode;
      if ( wi->mode < IW_NUM_OPER_MODE && wi->mode >= 0 )
        wi->has_mode = 1;
  }

  /* Get Power Management settings */                 // #if WIRELESS_EXT > 9
  wrq.u.power.flags = 0;
  if ( ioctl ( skfd, SIOCGIWPOWER, &wrq ) >= 0 ) {
    wi->has_power = 1;
    memcpy ( &wi->power, &wrq.u.power, sizeof ( iwparam ));
  }

  /* Get retry limit/lifetime */                      // #if WIRELESS_EXT > 10
  if ( ioctl ( skfd, SIOCGIWRETRY, &wrq ) >= 0 ) {    
    wi->has_retry = 1;
    memcpy ( &wi->retry, &wrq.u.retry, sizeof ( iwparam ));
  }

  /* Get stats */                                     // #if WIRELESS_EXT > 11
  wrq.u.data.pointer = ( caddr_t ) &wi->stats;
  wrq.u.data.length = 0;
  wrq.u.data.flags = 1;   /* Clear updated flag */
  if ( ioctl ( skfd, SIOCGIWSTATS, &wrq ) < 0 )
    wi->has_stats = 1;

  if ( !wi->has_stats ) {                        // no ioctl support, go to file
    fp = fopen ( PROC_NET_WIRELESS, "r" );
    if ( fp ) {
      while ( fgets ( bfr, sizeof ( bfr ), fp )) {
        bfr [ sizeof ( bfr ) - 1 ] = '\0';        // no buffer overruns here!
        strtok (( char * ) &bfr, "\n" );          // '\n' => '\0'
        if ( strstr ( bfr, ifname ) && strstr ( bfr, ":" )) {
          wi->has_stats = 1;
          s = bfr;
          s = strchr ( s, ':' ); s++;             /* Skip ethX:   */
          s = strtok ( s, " " );                  /* ' ' => '\0'  */
          sscanf ( s, "%X", &wi->stats.status ); // status 

          s = strtok ( NULL, " " );               // link quality
          if ( strchr ( s, '.' ) != NULL )
            wi->stats.qual.updated |= 1;
          sscanf ( s, "%d", &wi->stats.qual.qual );

          s = strtok ( NULL, " " );               // signal level
          if ( strchr ( s,'.' ) != NULL )
            wi->stats.qual.updated |= 2;
          sscanf ( s, "%d", &wi->stats.qual.level );

          s = strtok ( NULL, " " );               // noise level
          if ( strchr ( s, '.' ) != NULL )
            wi->stats.qual.updated += 4;
          sscanf ( s, "%d", &wi->stats.qual.noise );

          s = strtok ( NULL, " " ); sscanf ( s, "%d", &wi->stats.discard.nwid     );
          s = strtok ( NULL, " " ); sscanf ( s, "%d", &wi->stats.discard.code     );
          s = strtok ( NULL, " " ); sscanf ( s, "%d", &wi->stats.discard.fragment );
          s = strtok ( NULL, " " ); sscanf ( s, "%d", &wi->stats.discard.retries  );
          s = strtok ( NULL, " " ); sscanf ( s, "%d", &wi->stats.discard.misc     );
          s = strtok ( NULL, " " ); sscanf ( s, "%d", &wi->stats.miss.beacon      );
        }
      }
      fclose ( fp );
    }
  }

// printf ( "%s bfr: %s\n", "loadTables()", bfr );
}

/****************************************************************************
*                                                                           *
*       displayWiExt() - show what I got from Wireless Extensions           *
*                                                                           *
****************************************************************************/
static void displayWiExt ( struct wireless_info info )
{
#ifdef DISPLAYWIEXT
  int i;
  char title[] = "displayWiExt() -";

  printf ( "========================================\n" );
  printf ( "===> Wireless Extension IOCTL calls <===\n" );
  printf ( "========================================\n" );

  if ( strlen ( info.name ))
    printf ( "%s name: %s\n", "SIOCGIWNAME", info.name );
  else
    printf ( "%s\n", "no info.name support" );

  if ( info.has_nickname = 1 )
    printf ( "%s nickname: %s\n", "SIOCGIWNICKN", info.nickname );
  else
    printf ( "%s %s\n", "SIOCGIWNICKN", " ===> no info.nickname support" );

  if ( info.has_essid ) 
    printf ( "%s essid_on: %d essid: %s\n", "SIOCGIWESSID", info.essid_on, info.essid );
  else
    printf ( "%s %s\n", "SIOCGIWESSID", " ===> no info.essid support" );

  if ( info.has_range ) {
    printf ( "%s throughput: %d\n",           "SIOCGIWRANGE", info.range.throughput );
    printf ( "%s min_nwid: %d\n",             "SIOCGIWRANGE", info.range.min_nwid  );
    printf ( "%s max_nwid: %d\n",             "SIOCGIWRANGE", info.range.max_nwid  );
    printf ( "%s sensitivity: %d\n",          "SIOCGIWRANGE", info.range.sensitivity );
    printf ( "%s num_bitrates: %d\n",         "SIOCGIWRANGE", info.range.num_bitrates );
    for ( i = 0; i < info.range.num_bitrates; i++ ) 
      printf ( "%s bitrate[%d]: %d\n",        "SIOCGIWRANGE", i, info.range.bitrate[i]  );
    printf ( "%s min_rts: %d\n",              "SIOCGIWRANGE", info.range.min_rts );
    printf ( "%s max_rts: %d\n",              "SIOCGIWRANGE", info.range.max_rts );
    printf ( "%s min_frag: %d\n",             "SIOCGIWRANGE", info.range.min_frag );
    printf ( "%s max_frag: %d\n",             "SIOCGIWRANGE", info.range.max_frag );
    printf ( "%s min_pmp: %d\n",              "SIOCGIWRANGE", info.range.min_pmp );
    printf ( "%s max_pmp: %d\n",              "SIOCGIWRANGE", info.range.max_pmp );
    printf ( "%s min_pmt: %d\n",              "SIOCGIWRANGE", info.range.min_pmt );
    printf ( "%s max_pmt: %d\n",              "SIOCGIWRANGE", info.range.max_pmt );
    printf ( "%s pmp_flags: %d\n",            "SIOCGIWRANGE", info.range.pmp_flags );
    printf ( "%s pmt_flags: %d\n",            "SIOCGIWRANGE", info.range.pmt_flags );
    printf ( "%s pm_capa: %d\n",              "SIOCGIWRANGE", info.range.pm_capa );
    printf ( "%s num_encoding_sizes: %d\n",   "SIOCGIWRANGE", info.range.num_encoding_sizes );
    for ( i = 0; i < info.range.num_encoding_sizes; i++ ) 
      printf ( "%s encoding_size[%d]: %d\n",  "SIOCGIWRANGE", i, info.range.encoding_size[i]  );
    printf ( "%s max_encoding_tokens: %d\n",  "SIOCGIWRANGE", info.range.max_encoding_tokens );
//  printf ( "%s encoding_login_index: %d\n", "SIOCGIWRANGE", info.range.encoding_login_index );
    printf ( "%s txpower_capa: %d\n",         "SIOCGIWRANGE", info.range.txpower_capa );
    printf ( "%s num_txpower: %d dBm\n",      "SIOCGIWRANGE", info.range.num_txpower );
    for ( i = 0; i < info.range.num_txpower; i++ ) 
      printf ( "%s txpower[%d]: %d\n",        "SIOCGIWRANGE", i, info.range.txpower[i]  );
    printf ( "%s we_version_compiled: %d\n",  "SIOCGIWRANGE", info.range.we_version_compiled );
    printf ( "%s we_version_source: %d\n",    "SIOCGIWRANGE", info.range.we_version_source );
    printf ( "%s retry_capa: %d\n",           "SIOCGIWRANGE", info.range.retry_capa );
    printf ( "%s retry_flags: %d\n",          "SIOCGIWRANGE", info.range.retry_flags );
    printf ( "%s r_time_flags: %d\n",         "SIOCGIWRANGE", info.range.r_time_flags );
    printf ( "%s min_retry: %d\n",            "SIOCGIWRANGE", info.range.min_retry );
    printf ( "%s max_retry: %d\n",            "SIOCGIWRANGE", info.range.max_retry );
    printf ( "%s min_r_time: %d\n",           "SIOCGIWRANGE", info.range.min_r_time );
    printf ( "%s max_r_time: %d\n",           "SIOCGIWRANGE", info.range.max_r_time );
    printf ( "%s num_channels: %d\n",         "SIOCGIWRANGE", info.range.num_channels );
    printf ( "%s num_frequency: %d\n",        "SIOCGIWRANGE", info.range.num_frequency );
    for ( i = 0; i < info.range.num_frequency; i++ ) 
      printf ( "%s freq[%d].i: %d freq[%d].e: %d freq[%d].m: %d\n", "SIOCGIWRANGE", 
                i, info.range.freq[i].i, i, info.range.freq[i].e, i, info.range.freq[i].m );
  }
  else
    printf ( "%s %s\n", "SIOCGIWRANGE", " ===> no info.range support" );

  if ( info.has_nwid ) 
    printf ( "%s nwid - disabled: %d value: %X\n", "SIOCGIWNWID", info.nwid.disabled, info.nwid.value );
  else
    printf ( "%s %s\n", "SIOCGIWNWID", " ===> no info.nwid support" );

  if ( info.has_freq ) {
//  printf ( "%s freq: %g\n", "SIOCGIWFREQ", info.freq / GIGA );
    printf ( "%s freq: %g\n", "SIOCGIWFREQ", info.freq );
  }
  else
    printf ( "%s %s\n", "SIOCGIWFREQ", " ===> no info.freq support" );

  if ( info.has_sens )
    printf ( "%s sens: %d\n", "SIOCGIWSENS", info.sens );
  else
    printf ( "%s %s\n", "SIOCGIWSENS", " ===> no info.sens support" );

  if ( info.has_key ) {
    printf ( "%s key_size: %d key_flags: %d wepCurrentKey: %d\n", 
              "SIOCGIWENCODE", info.key_size, info.key_flags, wepCurrentKey );
    printf ( "%s MODE: %d DISABLED: %d INDEX: %d OPEN: %d RESTRICTED: %d NOKEY: %d TEMP: %d\n",
              "SIOCGIWENCODE",                           info.key_flags & IW_ENCODE_MODE,
              info.key_flags & IW_ENCODE_DISABLED ? 1:0, info.key_flags & IW_ENCODE_INDEX,
              info.key_flags & IW_ENCODE_OPEN     ? 1:0, info.key_flags & IW_ENCODE_RESTRICTED ? 1:0,
              info.key_flags & IW_ENCODE_NOKEY    ? 1:0, info.key_flags & IW_ENCODE_TEMP       ? 1:0 );
  }
  else
    printf ( "%s %s\n", "SIOCGIWENCODE", " ===> no info.key support" );

  for ( i = 0; i < MAX_WEP_KEYS; i++ ) {
    if ( wep[i].haveKey )
      printf ( "%s wep[%d].len: %d wep[%d].key: %s\n", 
                "SIOCGIWENCODE", i, wep[i].len, i, wep[i].key );
  }

  if ( info.has_ap_addr )
    printf ( "%s ap_addr.sa_data: %02X:%02X:%02X:%02X:%02X:%02X ap_addr.sa_family: %d\n", 
              "SIOCGIWAP",  ( UCHAR ) info.ap_addr.sa_data[0], ( UCHAR ) info.ap_addr.sa_data[1], 
                            ( UCHAR ) info.ap_addr.sa_data[2], ( UCHAR ) info.ap_addr.sa_data[3], 
                            ( UCHAR ) info.ap_addr.sa_data[4], ( UCHAR ) info.ap_addr.sa_data[5], 
                                      info.ap_addr.sa_family );
  else
    printf ( "%s %s\n", "SIOCGIWAP", " ===> no ap_addr information" );

  if ( info.has_bitrate )
    printf ( "%s bitrate: %d value: %d fixed: %d disabled: %d flags: %d\n", 
              "SIOCGIWRATE", info.bitrate, info.bitrate.value, info.bitrate.fixed, 
                             info.bitrate.disabled, info.bitrate.flags );
  else
    printf ( "%s %s\n", "SIOCGIWRATE", " ===> no info.bitrate support" );

  if ( info.has_rts )
    printf ( "%s rts: %d\n", "SIOCGIWRTS", info.rts );
  else
    printf ( "%s %s\n", "SIOCGIWRTS", " ===> no info.rts support" );

  if ( info.has_frag )
    printf ( "%s frag: %d\n", "SIOCGIWFRAG", info.frag );
  else
    printf ( "%s %s\n", "SIOCGIWFRAG", " ===> no info.frag support" );

  if ( info.has_mode ) 
    printf ( "%s mode: %d\n", "SIOCGIWMODE", info.mode );
  else
    printf ( "%s %s\n", "SIOCGIWMODE", " ===> no info.mode support" );

  if ( info.has_power ) {
    printf ( "%s power: %d\n", "SIOCGIWPOWER", info.power );
    printf ( "%s disabled: %d MIN: %d MAX: %d TIMEOUT: %d RELATIVE: %d\n",
              "SIOCGIWPOWER",
              info.power.disabled                  ? 1:0, 
              info.power.flags & IW_POWER_MIN      ? 1:0, 
              info.power.flags & IW_POWER_MAX      ? 1:0, 
              info.power.flags & IW_POWER_TIMEOUT  ? 1:0, 
              info.power.flags & IW_POWER_RELATIVE ? 1:0 ); 
    printf ( "%s UNICAST: %d MULTICAST: %d ALL: %d FORCE: %d REPEATER: %d\n",
              "SIOCGIWPOWER",
              info.power.flags & IW_POWER_UNICAST_R   ? 1:0, 
              info.power.flags & IW_POWER_MULTICAST_R ? 1:0, 
              info.power.flags & IW_POWER_ALL_R       ? 1:0, 
              info.power.flags & IW_POWER_FORCE_S     ? 1:0, 
              info.power.flags & IW_POWER_REPEATER    ? 1:0 ); 
  }
  else
    printf ( "%s %s\n", "SIOCGIWPOWER", " ===> no info.power support" );

  if ( info.has_retry )
    printf ( "%s retry: %d\n", "SIOCGIWRETRY", info.retry );
  else
    printf ( "%s %s\n", "SIOCGIWRETRY", " ===> no info.retry support" );

  if ( info.has_stats ) {
    printf ( "%s status: %d\n",           "SIOCGIWSTATS", info.stats.status           );
    printf ( "%s qual.level: %d\n",       "SIOCGIWSTATS", info.stats.qual.level       );
    printf ( "%s qual.noise: %d\n",       "SIOCGIWSTATS", info.stats.qual.noise       );
    printf ( "%s qual.qual: %d\n",        "SIOCGIWSTATS", info.stats.qual.qual        );
    printf ( "%s qual.updated: %d\n",     "SIOCGIWSTATS", info.stats.qual.updated     );
    printf ( "%s discard.code: %d\n",     "SIOCGIWSTATS", info.stats.discard.code     );
    printf ( "%s discard.fragment: %d\n", "SIOCGIWSTATS", info.stats.discard.fragment );
    printf ( "%s discard.misc: %d\n",     "SIOCGIWSTATS", info.stats.discard.misc     );
    printf ( "%s discard.nwid: %d\n",     "SIOCGIWSTATS", info.stats.discard.nwid     );
    printf ( "%s discard.retries: %d\n",  "SIOCGIWSTATS", info.stats.discard.retries  );
    printf ( "%s miss.beacon: %d\n",      "SIOCGIWSTATS", info.stats.miss.beacon      );
  }
  else
    printf ( "%s %s\n", "SIOCGIWSTATS", " ===> no info.stats support" );

  if ( info.txpower.flags & IW_TXPOW_MWATT )
    printf ( "%s txpower1: %d dBm disabled: %d fixed: %d flags: %d\n", "SIOCGIWRANGE", 
      mWatt2dbm ( info.txpower.value ), info.txpower.disabled, info.txpower.fixed, info.txpower.flags);
  else
    printf ( "%s txpower2: %d dBm disabled: %d fixed: %d flags: %d\n", "SIOCGIWRANGE", info.txpower.value, info.txpower.disabled, info.txpower.fixed, info.txpower.flags );

  if ( info.has_range ) 
    if ( info.sens.value < 0 )
      printf ( "%s sens: %d dBm\n", "SIOCGIWRANGE", info.sens.value );
    else
      printf ( "%s sens: %d/%d\n", "SIOCGIWRANGE", info.sens.value, info.range.sensitivity );

  if ( info.has_range && ( info.stats.qual.level != 0 ))
      if ( info.stats.qual.level > info.range.max_qual.level )
        /* Statistics are in dBm (absolute power measurement) */
        printf ( "%s Quality: %d/%d Signal level: %d dBm Noise level: %d dBm\n",
                  "SIOCGIWRANGE",
                  info.stats.qual.qual, info.range.max_qual.qual,
                  info.stats.qual.level - 0x100,
                  info.stats.qual.noise - 0x100 );
      else
        printf (  "%s Quality: %d/%d Signal level: %d/%d Noise level: %d/%d",
                  "SIOCGIWRANGE",
                  info.stats.qual.qual,  info.range.max_qual.qual,
                  info.stats.qual.level, info.range.max_qual.level,
                  info.stats.qual.noise, info.range.max_qual.noise );

#endif // #ifdef DISPLAYWIEXT
}

/****************************************************************************
*                                                                           *
*                        Linked List Functions                              *
*                                                                           *
****************************************************************************/
/****************************************************************************
*                                                                           *
*                addList() - add an entry to a linked list                  *
*                                                                           *
****************************************************************************/
static void 
addList ( char *l, char *data, int len  )
{
  char uid[256];
  LIST_HEAD ( , avNode ) *list;       

  // NOTE: this assumes the UID is at the beginning of the 
  //       data structure and that UIDs are strings
  
  list = ( LIST_HEAD ( , avNode ) * ) l;            // NOTE: don't know how to get 
  strcpy ( uid, data );                             //  rid of compiler warning on
                                                    //  LISTHEAD typecast
  // create a new node and the data that goes in it
  newNode = malloc ( sizeof ( struct avNode ));
  newNode->data = malloc ( len );
  memcpy ( newNode->data, data, len );

  // this deals with an empty list
  if ( LIST_EMPTY ( list )) {
    LIST_INSERT_HEAD ( list, newNode, nodes );
    return;
  }

  // this deals with UIDs that match
  for ( np = LIST_FIRST ( list ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    if ( strncmp ( uid, np->data, strlen ( uid )) == 0 ) {                      // found matching UID
      LIST_INSERT_AFTER ( np, newNode, nodes );
      if ( np->data )
        free ( np->data );
      LIST_REMOVE ( np, nodes );
      free ( np );
      return;
    }
  }

  // this deals with inserting a new UID in the list
  for ( np = LIST_FIRST ( list ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    lastNode = np;
    if ( strncmp ( np->data, uid, strlen ( uid )) > 0 ) {                       // old ID > new ID AND
      LIST_INSERT_BEFORE ( np, newNode, nodes );
      return;
    }
  }

  // this deals with a UID that needs to go on the end of the list
  LIST_INSERT_AFTER ( lastNode, newNode, nodes );

  return;
}

/****************************************************************************
*                                                                           *
*              initLists() - initialize all the linked lists                *
*                                                                           *
****************************************************************************/
static void initLists()
{
  LIST_INIT ( &scList );  LIST_INIT ( &aaList );  LIST_INIT ( &dfList );
  LIST_INIT ( &kmList );  LIST_INIT ( &prList );
  LIST_INIT ( &opList );  LIST_INIT ( &coList );
  LIST_INIT ( &gaList );  LIST_INIT ( &riList );  LIST_INIT ( &poList );
  LIST_INIT ( &paList );  LIST_INIT ( &ptList );  LIST_INIT ( &pfList );
  LIST_INIT ( &pdList );  LIST_INIT ( &piList );  LIST_INIT ( &rdList );
  LIST_INIT ( &alList );  LIST_INIT ( &rtList );  LIST_INIT ( &rrList );
}
/****************************************************************************
*                                                                           *
*                 flushLists() - flush all linked lists                     *
*                                                                           *
****************************************************************************/
static void flushLists()
{
  flushList (( char * ) &scList );  flushList (( char * ) &aaList );
  flushList (( char * ) &dfList );  flushList (( char * ) &kmList );
  flushList (( char * ) &prList );
  flushList (( char * ) &opList );  flushList (( char * ) &coList );
  flushList (( char * ) &gaList );  flushList (( char * ) &riList );
  flushList (( char * ) &poList );  flushList (( char * ) &paList );
  flushList (( char * ) &ptList );  flushList (( char * ) &pfList );
  flushList (( char * ) &pdList );  flushList (( char * ) &piList );
  flushList (( char * ) &rdList );  flushList (( char * ) &alList );
  flushList (( char * ) &rtList );  flushList (( char * ) &rrList );
}

/****************************************************************************
*                                                                           *
*                   flushList() - flush a linked list                       *
*                                                                           *
****************************************************************************/
static void flushList ( char *l )
{
  LIST_HEAD ( , avNode ) *list;
  
  list = ( LIST_HEAD ( , avNode ) * ) l;    // NOTE: don't know how to get 
  while ( !LIST_EMPTY ( list )) {           //  rid of compiler warning on
    np = LIST_FIRST ( list );               //  LISTHEAD typecast
    if ( np->data )
      free ( np->data );
    LIST_REMOVE ( np, nodes );
    free ( np );
  }
}

/****************************************************************************
*                                                                           *
*                            Utility Functions                              *
*                                                                           *
****************************************************************************/
/****************************************************************************
*                                                                           *
*        The following two routines were taken directly from iwlib.c        *
*                                                                           *
****************************************************************************/
 /*
 * Open a socket.
 * Depending on the protocol present, open the right socket. The socket
 * will allow us to talk to the driver.
 */
static int openSocket ( void )
{
  static const int families[] = {
    AF_INET, AF_IPX, AF_AX25, AF_APPLETALK
  };
  unsigned int  i;
  int   sock;

  /*
   * Now pick any (exisiting) useful socket family for generic queries
   * Note : don't open all the socket, only returns when one matches,
   * all protocols might not be valid.
   * Workaround by Jim Kaba <jkaba@sarnoff.com>
   * Note : in 99% of the case, we will just open the inet_sock.
   * The remaining 1% case are not fully correct...
   */

  /* Try all families we support */
  for(i = 0; i < sizeof(families)/sizeof(int); ++i) {
      /* Try to open the socket, if success returns it */
      sock = socket(families[i], SOCK_DGRAM, 0);
      if(sock >= 0)
  return sock;
  }

  return -1;
}

/*------------------------------------------------------------------*/
/*
 * Convert a value in milliWatt to a value in dBm.
 */
static int mWatt2dbm ( int in )
{
#ifdef WE_NOLIBM
  /* Version without libm : slower */
  double  fin = (double) in;
  int   res = 0;

  /* Split integral and floating part to avoid accumulating rounding errors */
  while(fin > 10.0)
    {
      res += 10;
      fin /= 10.0;
    }
  while(fin > 1.000001) /* Eliminate rounding errors, take ceil */
    {
      res += 1;
      fin /= LOG10_MAGIC;
    }
  return(res);
#else /* WE_NOLIBM */
  /* Version with libm : faster */
  return((int) (ceil(10.0 * log10((double) in))));
#endif  /* WE_NOLIBM */
}

/****************************************************************************
*                                                                           *
*                 htob - converts hex string to binary                      *
*                                                                           *
****************************************************************************/
static char *htob ( char *s )
{
    char nibl, *byt;
    static char bin[20];

    byt = bin;

    while ((nibl = *s++) && nibl != ' ') {    /* While not end of string. */
      nibl -= ( nibl > '9') ?  ('A' - 10): '0';
      *byt = nibl << 4;                              /* place high nibble */
      if((nibl = *s++) && nibl != ' ') {
        nibl -= ( nibl > '9') ?  ('A' - 10): '0';
        *byt |= nibl;                                /*  place low nibble */
      }
      else break;
      ++byt;
    }
    *++byt = '\0';
    return ( bin );
}

/****************************************************************************
*                                                                           *
*           hasChanged() - see if area has been changed from NULLs          *
*                                                                           *
****************************************************************************/
static int hasChanged ( char *loc, int len )
{
  char *wrk;
  int changed = TRUE;

  wrk = malloc ( len );
  memset ( wrk, 0, len );
  if ( memcmp ( loc, wrk, len ) == 0 )
    changed = FALSE;
  free ( wrk );

  return ( changed );
}


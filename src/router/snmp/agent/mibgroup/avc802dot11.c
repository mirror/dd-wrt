/**** Copyright (c) AvantCom Corp 2003 - 2004 ***/
/**** All rights reserved ***/
/*__________________________________________________________________
  |                                                                |
  |         Copyright 2003 - 2004  -  AvantCom Corporation         |
  |                                                                |
  |    This material is the exclusive confidential property of     |
  |    AvantCom Corporation. All rights, including copyrights      |
  |    and patent rights, are reserved. No copies of any portion   |
  |    are to be made by any means without the express written     |
  |    permission of AvantCom Corporation.                         |
  |                                                                |
  |________________________________________________________________|
*/ 
/****************************************************************************
*                                                                           *
*  File Name:           avc802dot11.c                                       *
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
*  Creation Date:       09/24/03                                            *
*                                                                           *
*   Ver    Date   Inits Modification                                        *
*  ----- -------- ----- ------------                                        *
*  0.0.1 09/24/03  LRS  created                                             *
*  0.0.3 11/03/04  LRS  added signal/noise for Prism cards                  *
*                  LRS  made prAcRxBytes and prAcTxBytes Counter64          *
****************************************************************************/
/****************************************************************************
*                               Includes                                    *
****************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "avc802dot11.h"
#include "dirent.h"
#include "iwlib.h"

/****************************************************************************
*                                Defines                                    *
****************************************************************************/
//#define DISPLAYWIEXT                      // display wireless ext info
#define TABLE_SIZE   1
//#define MINLOADFREQ 15                    // min reload frequency in seconds
#define MINLOADFREQ 5                       // min reload frequency in seconds     // for testing

#define HOSTAPPATH        "/proc/net/hostap/"
#define MADWIFIPATH       "/proc/sys/net/"
#define PROC_NET_DEV      "/proc/net/dev"

//#define HOSTAPPATH  "/home/lsimmons/proc/net/hostap/"                            // for testing
//#define MADWIFIPATH "/home/lsimmons/proc/sys/net/"                               // for testing

#define EXPECTED_MIB_VERSION   "Version 0.0.3 - 11/03/2004"
#define AGENT_VERSION          "Version 0.0.3 - 11/03/2004"

#ifndef UCHAR
  typedef unsigned char UCHAR;
#endif

/****************************************************************************
*                            Private Functions                              *
****************************************************************************/
static void loadTables();
static void loadPrismIfStruct ( int, char *, struct wireless_info * );
static void loadHostAPClients();
static void loadMadWiFiStats();
static void initStructs();

// Wireless Extensions Specific Functions
static void loadWiExt ( int, char *, struct wireless_info * );
static void displayWiExt ( struct wireless_info );

// Linked List Functions
static void addList ( char *, char *, int );
static void initLists();                    // initialize all the linked lists
static void flushLists();                   // flush all the linked lists
static void flushList ( char * );           // flush a single linked list

// Utility Functions
static int  openSocket ( void );
static int  mWatt2dbm ( int );
static int  isMACAddress ( char * );
static void strtoupper  ( char * );
static unsigned int htoui ( char * );
static char *htob ( char * );

/****************************************************************************
*                            Private Variables                              *
****************************************************************************/
static unsigned long lastLoad = 0;          // ET in secs at last table load

static struct avc802dot11Node *lastNode, *newNode, *np;

/****************************************************************************
*                            External Functions                             *
****************************************************************************/

/****************************************************************************
*   avc802dot11_variables_oid:                                              *
*       this is the top level oid that we want to register under.  This     *
*       is essentially a prefix, with the suffix appearing in the           *
*       variable below.                                                     *
****************************************************************************/
oid avc802dot11_variables_oid[] = { 1,3,6,1,3,14614 };

/****************************************************************************
*   variable7 avc802dot11_variables:                                        *
*     this variable defines function callbacks and type return information  *
*     for the ieee802dot11 mib section                                      *
****************************************************************************/
struct variable7 avc802dot11_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#define   SYSEXPECTEDMIBVERSION  2
  { SYSEXPECTEDMIBVERSION, ASN_OCTET_STR , RONLY , var_avc802dot11, 2, { 1,2 } },
#define   SYSAGENTVERSION       3
  { SYSAGENTVERSION     , ASN_OCTET_STR , RONLY , var_avc802dot11, 2, { 1,3 } },

#define   PRACMACADDRESS        6
  { PRACMACADDRESS      , ASN_OCTET_STR , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,1 } },
#define   PRACSTATIONTYPE       7
  { PRACSTATIONTYPE     , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,2 } },
#define   PRACLISTENINTERVAL    8
  { PRACLISTENINTERVAL  , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,3 } },
#define   PRACSUPPORTS1MBPS     9
  { PRACSUPPORTS1MBPS   , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,4 } },
#define   PRACSUPPORTS2MBPS     10
  { PRACSUPPORTS2MBPS   , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,5 } },
#define   PRACSUPPORTS5MBPS     11
  { PRACSUPPORTS5MBPS   , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,6 } },
#define   PRACSUPPORTS11MBPS    12
  { PRACSUPPORTS11MBPS  , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,7 } },
#define   PRACJIFFIES           13
  { PRACJIFFIES         , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,8 } },
#define   PRACLASTAUTH          14
  { PRACLASTAUTH        , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,9 } },
#define   PRACLASTASSOC         15
  { PRACLASTASSOC       , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,10 } },
#define   PRACLASTRX            16
  { PRACLASTRX          , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,11 } },
#define   PRACLASTTX            17
  { PRACLASTTX          , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,12 } },
#define   PRACRXPACKETS         18
  { PRACRXPACKETS       , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,13 } },
#define   PRACTXPACKETS         19
  { PRACTXPACKETS       , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,14 } },
#define   PRACRXBYTES           20
  { PRACRXBYTES         , ASN_COUNTER64 , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,15 } },
#define   PRACTXBYTES           21
  { PRACTXBYTES         , ASN_COUNTER64 , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,16 } },
#define   PRACBFRCNT            22
  { PRACBFRCNT          , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,17 } },
#define   PRACLASTRXSILENCE     23
  { PRACLASTRXSILENCE   , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,18 } },
#define   PRACLASTRXSIGNAL      24
  { PRACLASTRXSIGNAL    , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,19 } },
#define   PRACLASTRXRATE        25
  { PRACLASTRXRATE      , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,20 } },
#define   PRACLASTRXFLOW        26
  { PRACLASTRXFLOW      , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,21 } },
#define   PRACTXRATE            27
  { PRACTXRATE          , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,22 } },
#define   PRACTX1MBPS           28
  { PRACTX1MBPS         , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,23 } },
#define   PRACTX2MBPS           29
  { PRACTX2MBPS         , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,24 } },
#define   PRACTX5MBPS           30
  { PRACTX5MBPS         , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,25 } },
#define   PRACTX11MBPS          31
  { PRACTX11MBPS        , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,26 } },
#define   PRACRX1MBPS           32
  { PRACRX1MBPS         , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,27 } },
#define   PRACRX2MBPS           33
  { PRACRX2MBPS         , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,28 } },
#define   PRACRX5MBPS           34
  { PRACRX5MBPS         , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,29 } },
#define   PRACRX11MBPS          35
  { PRACRX11MBPS        , ASN_INTEGER   , RONLY , var_prAssociatedClientsTable, 5, { 2,1,1,1,30 } },

#define   PRIFSIGNAL            38
  { PRIFSIGNAL          , ASN_INTEGER   , RONLY , var_prInterfaceTable, 5, { 2,2,1,1,1 } },
#define   PRIFNOISE             39
  { PRIFNOISE           , ASN_INTEGER   , RONLY , var_prInterfaceTable, 5, { 2,2,1,1,2 } },
#define   PRIFQUALITY           40
  { PRIFQUALITY         , ASN_INTEGER   , RONLY , var_prInterfaceTable, 5, { 2,2,1,1,3 } },
#define   PRIFASSOCCNT          41
  { PRIFASSOCCNT        , ASN_INTEGER   , RONLY , var_prInterfaceTable, 5, { 2,2,1,1,4 } },

#define   AMCWATCHDOG           44
  { AMCWATCHDOG         , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,1 } },
#define   AMCFATALHWINTERRUPT   45
  { AMCFATALHWINTERRUPT , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,2 } },
#define   AMCMISSEDBEACONS      46
  { AMCMISSEDBEACONS    , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,3 } },
#define   AMCRXOVERRUN          47
  { AMCRXOVERRUN        , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,4 } },
#define   AMCRXEOL              48
  { AMCRXEOL            , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,5 } },
#define   AMCTXUNDERRUN         49
  { AMCTXUNDERRUN       , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,6 } },
#define   AMCTXMGMTFRAMES       50
  { AMCTXMGMTFRAMES     , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,7 } },
#define   AMCTXDISCARDS         51
  { AMCTXDISCARDS       , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,8 } },
#define   AMCTXINVALID          52
  { AMCTXINVALID        , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,9 } },
#define   AMCTXQUEUESTOPPED     53
  { AMCTXQUEUESTOPPED   , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,10 } },
#define   AMCTXENCAPFAILED      54
  { AMCTXENCAPFAILED    , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,11 } },
#define   AMCTXNONODE           55
  { AMCTXNONODE         , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,12 } },
#define   AMCTXNODATABUFFER     56
  { AMCTXNODATABUFFER   , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,13 } },
#define   AMCTXNOMGMTBUFFER     57
  { AMCTXNOMGMTBUFFER   , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,14 } },
#define   AMCTXEXCESSIVERETRIES  58
  { AMCTXEXCESSIVERETRIES, ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,15 } },
#define   AMCTXFIFOUNDERRUN     59
  { AMCTXFIFOUNDERRUN   , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,16 } },
#define   AMCTXFILTERED         60
  { AMCTXFILTERED       , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,17 } },
#define   AMCTXSHORTRETRIES     61
  { AMCTXSHORTRETRIES   , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,18 } },
#define   AMCTXLONGRETRIES      62
  { AMCTXLONGRETRIES    , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,19 } },
#define   AMCTXBADRATE          63
  { AMCTXBADRATE        , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,20 } },
#define   AMCTXNOACK            64
  { AMCTXNOACK          , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,21 } },
#define   AMCTXRTS              65
  { AMCTXRTS            , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,22 } },
#define   AMCTXCTS              66
  { AMCTXCTS            , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,23 } },
#define   AMCTXSHORTPREAMBLE    67
  { AMCTXSHORTPREAMBLE  , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,24 } },
#define   AMCTXRSSI             68
  { AMCTXRSSI           , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,25 } },
#define   AMCTXRSSIDELTA        69
  { AMCTXRSSIDELTA      , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,26 } },
#define   AMCRXDESCOVERRUN      70
  { AMCRXDESCOVERRUN    , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,27 } },
#define   AMCRXFRAMETOOSHORT    71
  { AMCRXFRAMETOOSHORT  , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,28 } },
#define   AMCRXCRCERROR         72
  { AMCRXCRCERROR       , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,29 } },
#define   AMCRXFIFOOVERRUN      73
  { AMCRXFIFOOVERRUN    , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,30 } },
#define   AMCRXCRYPTFAILURE     74
  { AMCRXCRYPTFAILURE   , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,31 } },
#define   AMCRXNOSKBUFFER       75
  { AMCRXNOSKBUFFER     , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,32 } },
#define   AMCRXRSSI             76
  { AMCRXRSSI           , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,33 } },
#define   AMCRXRSSIDELTA        77
  { AMCRXRSSIDELTA      , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,34 } },
#define   AMCNOBEACONBUFFER     78
  { AMCNOBEACONBUFFER   , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,35 } },
#define   AMCCALIBRATIONS       79
  { AMCCALIBRATIONS     , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,36 } },
#define   AMCFAILEDCALIBRATIONS  80
  { AMCFAILEDCALIBRATIONS, ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,37 } },
#define   AMCRFGAINRESETS       81
  { AMCRFGAINRESETS     , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,38 } },
#define   AMCRATECONTROLCHECKS  82
  { AMCRATECONTROLCHECKS, ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,39 } },
#define   AMCRATECONTROLRAISE   83
  { AMCRATECONTROLRAISE , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,40 } },
#define   AMCRATECONTROLDROP    84
  { AMCRATECONTROLDROP  , ASN_INTEGER   , RONLY , var_athMacCountersTable, 5, { 3,1,1,1,41 } },

#define   APCTOTALPHYERRORS     87
  { APCTOTALPHYERRORS   , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,1 } },
#define   APCTXUNDERRUN         88
  { APCTXUNDERRUN       , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,2 } },
#define   APCTIMINGERRORS       89
  { APCTIMINGERRORS     , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,3 } },
#define   APCPARITYERRORS       90
  { APCPARITYERRORS     , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,4 } },
#define   APCINVALIDRATE        91
  { APCINVALIDRATE      , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,5 } },
#define   APCINVALIDLENGTH      92
  { APCINVALIDLENGTH    , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,6 } },
#define   APCRADARDETECTED      93
  { APCRADARDETECTED    , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,7 } },
#define   APCINVALIDSERVICE     94
  { APCINVALIDSERVICE   , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,8 } },
#define   APCTXOVERRIDERX       95
  { APCTXOVERRIDERX     , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,9 } },
#define   APCOFDMTIMING         96
  { APCOFDMTIMING       , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,10 } },
#define   APCOFDMSIGNALPARITY   97
  { APCOFDMSIGNALPARITY , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,11 } },
#define   APCOFDMINVALIDRATE    98
  { APCOFDMINVALIDRATE  , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,12 } },
#define   APCOFDMINVALIDLENGTH  99
  { APCOFDMINVALIDLENGTH, ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,13 } },
#define   APCOFDMPOWERDROP      100
  { APCOFDMPOWERDROP    , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,14 } },
#define   APCOFDMSERVICE        101
  { APCOFDMSERVICE      , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,15 } },
#define   APCOFDMRESTART        102
  { APCOFDMRESTART      , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,16 } },
#define   APCCCKTIMING          103
  { APCCCKTIMING        , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,17 } },
#define   APCCCKHEADERCRC       104
  { APCCCKHEADERCRC     , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,18 } },
#define   APCCCKINVALIDRATE     105
  { APCCCKINVALIDRATE   , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,19 } },
#define   APCCCKSERVICE         106
  { APCCCKSERVICE       , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,20 } },
#define   APCCCKRESTART         107
  { APCCCKRESTART       , ASN_INTEGER   , RONLY , var_athPhyCountersTable, 5, { 3,2,1,1,21 } },

};

/*    (L = length of the oidsuffix) */

/****************************************************************************
*                                                                           *
*          init_avc802dot11() - perform any required initialization         *
*                                                                           *
****************************************************************************/
void init_avc802dot11(void) {

  /* register ourselves with the agent to handle our mib tree */
  REGISTER_MIB("avc802dot11", avc802dot11_variables, variable7,
               avc802dot11_variables_oid);

  initLists();
}


/****************************************************************************
*                                                                           *
*     shutdown_avc802dot11() - perform any required cleanup @ shutdown      *
*                                                                           *
****************************************************************************/
void shutdown_avc802dot11 ( void )
{
  flushLists();
}

/****************************************************************************
*                                                                           *
*   var_avc802dot11() -                                                     *
*                                                                           *
****************************************************************************/
unsigned char *
var_avc802dot11(struct variable *vp, 
                oid     *name, 
                size_t  *length, 
                int     exact, 
                size_t  *var_len, 
                WriteMethod **write_method)
{
  static unsigned char string[SPRINT_MAX_LEN];

  if ( header_generic ( vp, name, length, exact, var_len, write_method )
                                  == MATCH_FAILED )
    return NULL;

  switch ( vp->magic ) {

    case SYSEXPECTEDMIBVERSION:
      strcpy ( string, EXPECTED_MIB_VERSION );
      *var_len = strlen ( string );
      return ( UCHAR * ) string;

    case SYSAGENTVERSION:
      strcpy ( string, AGENT_VERSION );
      *var_len = strlen ( string );
      return ( UCHAR * ) string;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_prAssociatedClientsTable() -                                        *
*                                                                           *
****************************************************************************/
unsigned char *
var_prAssociatedClientsTable ( struct variable *vp,
                                oid     *name,
                                size_t  *length,
                                int     exact,
                                size_t  *var_len,
                                WriteMethod **write_method )
{
  oid rName [ MAX_OID_LEN ];                            // OID to be returned
  int found = FALSE;
  static char MACWork[17];
  static struct counter64 c64;

  loadTables();

  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &prAcList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    prAc = ( struct prAcTbl_data * ) np->data;
    rName[vp->namelen + 0] = prAc->ifIndex;
    rName[vp->namelen + 1] = htoui ( &prAc->macAddress[0] );
    rName[vp->namelen + 2] = htoui ( &prAc->macAddress[3] );
    rName[vp->namelen + 3] = htoui ( &prAc->macAddress[6] );
    rName[vp->namelen + 4] = htoui ( &prAc->macAddress[9] );
    rName[vp->namelen + 5] = htoui ( &prAc->macAddress[12] );
    rName[vp->namelen + 6] = htoui ( &prAc->macAddress[15] );
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 7, name, *length ) == 0 )) ||
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 7, name, *length ) >  0 ))) {
      found = TRUE;
      break;
    }
  }

  if ( !found  )
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 7 ) * sizeof ( oid ));
  *length = vp->namelen + 7;
  *write_method = 0;
  *var_len = sizeof ( long );

  switch ( vp->magic ) {

    case PRACMACADDRESS:
      MACWork[ 0] = toupper ( prAc->macAddress[ 0] );
      MACWork[ 1] = toupper ( prAc->macAddress[ 1] );
      MACWork[ 2] = toupper ( prAc->macAddress[ 3] );
      MACWork[ 3] = toupper ( prAc->macAddress[ 4] );
      MACWork[ 4] = toupper ( prAc->macAddress[ 6] );
      MACWork[ 5] = toupper ( prAc->macAddress[ 7] );
      MACWork[ 6] = toupper ( prAc->macAddress[ 9] );
      MACWork[ 7] = toupper ( prAc->macAddress[10] );
      MACWork[ 8] = toupper ( prAc->macAddress[12] );
      MACWork[ 9] = toupper ( prAc->macAddress[13] );
      MACWork[10] = toupper ( prAc->macAddress[15] );
      MACWork[11] = toupper ( prAc->macAddress[16] );
      MACWork[12] = '\0';
      *var_len = 6;
      return ( UCHAR * ) htob ( MACWork );
        
    case PRACSTATIONTYPE:     return ( UCHAR * ) &prAc->stationType;
    case PRACLISTENINTERVAL:  return ( UCHAR * ) &prAc->listenInterval;
    case PRACSUPPORTS1MBPS:   return ( UCHAR * ) &prAc->supports1Mbps;
    case PRACSUPPORTS2MBPS:   return ( UCHAR * ) &prAc->supports2Mbps;
    case PRACSUPPORTS5MBPS:   return ( UCHAR * ) &prAc->supports5Mbps;
    case PRACSUPPORTS11MBPS:  return ( UCHAR * ) &prAc->supports11Mbps;
    case PRACJIFFIES:         return ( UCHAR * ) &prAc->jiffies;
    case PRACLASTAUTH:        return ( UCHAR * ) &prAc->lastAuth;
    case PRACLASTASSOC:       return ( UCHAR * ) &prAc->lastAssoc;
    case PRACLASTRX:          return ( UCHAR * ) &prAc->lastRx;
    case PRACLASTTX:          return ( UCHAR * ) &prAc->lastTx;
    case PRACRXPACKETS:       return ( UCHAR * ) &prAc->rxPackets;
    case PRACTXPACKETS:       return ( UCHAR * ) &prAc->txPackets;

    case PRACRXBYTES:         
      c64.high = ( unsigned long ) ( prAc->rxBytes >> 32 );
      c64.low  = ( unsigned long ) ( prAc->rxBytes & 0xffffffff );
     *var_len = sizeof ( c64 );
       return ( UCHAR * ) &c64;

    case PRACTXBYTES:         
      c64.high = ( unsigned long ) ( prAc->txBytes >> 32 );
      c64.low  = ( unsigned long ) ( prAc->txBytes & 0xffffffff );
     *var_len = sizeof ( c64 );
      return ( UCHAR * ) &c64;

    case PRACBFRCNT:          return ( UCHAR * ) &prAc->bfrCnt;
    case PRACLASTRXSILENCE:   return ( UCHAR * ) &prAc->lastRxSilence;
    case PRACLASTRXSIGNAL:    return ( UCHAR * ) &prAc->lastRxSignal;
    case PRACLASTRXRATE:      return ( UCHAR * ) &prAc->lastRxRate;
    case PRACLASTRXFLOW:      return ( UCHAR * ) &prAc->lastRxFlow;
    case PRACTXRATE:          return ( UCHAR * ) &prAc->txRate;
    case PRACTX1MBPS:         return ( UCHAR * ) &prAc->tx1Mbps;
    case PRACTX2MBPS:         return ( UCHAR * ) &prAc->tx2Mbps;
    case PRACTX5MBPS:         return ( UCHAR * ) &prAc->tx5Mbps;
    case PRACTX11MBPS:        return ( UCHAR * ) &prAc->tx11Mbps;
    case PRACRX1MBPS:         return ( UCHAR * ) &prAc->rx1Mbps;
    case PRACRX2MBPS:         return ( UCHAR * ) &prAc->rx2Mbps;
    case PRACRX5MBPS:         return ( UCHAR * ) &prAc->rx5Mbps;
    case PRACRX11MBPS:        return ( UCHAR * ) &prAc->rx11Mbps;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}


/****************************************************************************
*                                                                           *
*       var_prInterfaceTable()() -                                          *
*                                                                           *
****************************************************************************/
unsigned char *
var_prInterfaceTable ( struct variable  *vp,
    	                  oid             *name,
    	                  size_t          *length,
    	                  int              exact,
    	                  size_t          *var_len,
    	                  WriteMethod    **write_method )
{
  int found = FALSE;
  oid rName [ MAX_OID_LEN ];                            // OID to be returned

  loadTables();
  memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
  for ( np = LIST_FIRST ( &prIfList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    prIf = ( struct prIfTbl_data * ) np->data;
    rName[vp->namelen] = prIf->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) || 
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      found = TRUE;
      break;
    }
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case PRIFSIGNAL:    return ( UCHAR * ) &prIf->signal;
    case PRIFNOISE:     return ( UCHAR * ) &prIf->noise;
    case PRIFQUALITY:   return ( UCHAR * ) &prIf->quality;
    case PRIFASSOCCNT:  return ( UCHAR * ) &prIf->assocCnt;

    default: 
      ERROR_MSG("");
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*       var_athMacCountersTable() -                                         *
*                                                                           *
****************************************************************************/
unsigned char *
var_athMacCountersTable ( struct variable *vp,
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
  for ( np = LIST_FIRST ( &athMacList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    athMac = ( struct athMacTbl_data * ) np->data;
    rName[vp->namelen] = athMac->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) || 
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      found = TRUE;
      break;
    }
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case AMCWATCHDOG:           return ( UCHAR * ) &athMac->watchDog;
    case AMCFATALHWINTERRUPT:   return ( UCHAR * ) &athMac->fatalHwInterrupt;
    case AMCMISSEDBEACONS:      return ( UCHAR * ) &athMac->missedBeacons;
    case AMCRXOVERRUN:          return ( UCHAR * ) &athMac->rxOverRun;
    case AMCRXEOL:              return ( UCHAR * ) &athMac->rxEol;
    case AMCTXUNDERRUN:         return ( UCHAR * ) &athMac->txUnderRun;
    case AMCTXMGMTFRAMES:       return ( UCHAR * ) &athMac->txMgmtFrames;
    case AMCTXDISCARDS:         return ( UCHAR * ) &athMac->txDiscards;
    case AMCTXINVALID:          return ( UCHAR * ) &athMac->txInvalid;
    case AMCTXQUEUESTOPPED:     return ( UCHAR * ) &athMac->txQueueStopped;
    case AMCTXENCAPFAILED:      return ( UCHAR * ) &athMac->txEncapFailed;
    case AMCTXNONODE:           return ( UCHAR * ) &athMac->txNoNode;
    case AMCTXNODATABUFFER:     return ( UCHAR * ) &athMac->txNoDataBuffer;
    case AMCTXNOMGMTBUFFER:     return ( UCHAR * ) &athMac->txNoMgmtBuffer;
    case AMCTXEXCESSIVERETRIES: return ( UCHAR * ) &athMac->txExcessiveRetries;
    case AMCTXFIFOUNDERRUN:     return ( UCHAR * ) &athMac->txFifoUnderRun;
    case AMCTXFILTERED:         return ( UCHAR * ) &athMac->txFiltered;
    case AMCTXSHORTRETRIES:     return ( UCHAR * ) &athMac->txShortRetries;
    case AMCTXLONGRETRIES:      return ( UCHAR * ) &athMac->txLongRetries;
    case AMCTXBADRATE:          return ( UCHAR * ) &athMac->txBadRate;
    case AMCTXNOACK:            return ( UCHAR * ) &athMac->txNoAck;
    case AMCTXRTS:              return ( UCHAR * ) &athMac->txRts;
    case AMCTXCTS:              return ( UCHAR * ) &athMac->txCts;
    case AMCTXSHORTPREAMBLE:    return ( UCHAR * ) &athMac->txShortPreamble;
    case AMCTXRSSI:             return ( UCHAR * ) &athMac->txRssi;
    case AMCTXRSSIDELTA:        return ( UCHAR * ) &athMac->txRssiDelta;
    case AMCRXDESCOVERRUN:      return ( UCHAR * ) &athMac->rxDescOverRun;
    case AMCRXFRAMETOOSHORT:    return ( UCHAR * ) &athMac->rxFrameTooShort;
    case AMCRXCRCERROR:         return ( UCHAR * ) &athMac->rxCrcError;
    case AMCRXFIFOOVERRUN:      return ( UCHAR * ) &athMac->rxFifoOverRun;
    case AMCRXCRYPTFAILURE:     return ( UCHAR * ) &athMac->rxCryptFailure;
    case AMCRXNOSKBUFFER:       return ( UCHAR * ) &athMac->rxNoSkBuffer;
    case AMCRXRSSI:             return ( UCHAR * ) &athMac->rxRssi;
    case AMCRXRSSIDELTA:        return ( UCHAR * ) &athMac->rxRssiDelta;
    case AMCNOBEACONBUFFER:     return ( UCHAR * ) &athMac->noBeaconBuffer;
    case AMCCALIBRATIONS:       return ( UCHAR * ) &athMac->calibrations;
    case AMCFAILEDCALIBRATIONS: return ( UCHAR * ) &athMac->failedCalibrations;
    case AMCRFGAINRESETS:       return ( UCHAR * ) &athMac->rfGainResets;
    case AMCRATECONTROLCHECKS:  return ( UCHAR * ) &athMac->rateControlChecks;
    case AMCRATECONTROLRAISE:   return ( UCHAR * ) &athMac->rateControlRaise;
    case AMCRATECONTROLDROP:    return ( UCHAR * ) &athMac->rateControlDrop;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*   var_athPhyCountersTable() -                                             *
*                                                                           *
****************************************************************************/
unsigned char *
var_athPhyCountersTable ( struct variable *vp,
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
  for ( np = LIST_FIRST ( &athPhyList ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    athPhy = ( struct athPhyTbl_data * ) np->data;
    rName[vp->namelen] = athPhy->ifIndex;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) || 
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 ))) {
      found = TRUE;
      break;
    }
  }

  if ( !found ) 
    return NULL;

  memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
  *length = vp->namelen + 1;
  *var_len = sizeof ( long );
  *write_method = NULL;

  switch ( vp->magic ) {

    case APCTOTALPHYERRORS:     return ( UCHAR * ) &athPhy->totalPhyErrors;
    case APCTXUNDERRUN:         return ( UCHAR * ) &athPhy->txUnderRun;
    case APCTIMINGERRORS:       return ( UCHAR * ) &athPhy->timingErrors;
    case APCPARITYERRORS:       return ( UCHAR * ) &athPhy->parityErrors;
    case APCINVALIDRATE:        return ( UCHAR * ) &athPhy->invalidRate;
    case APCINVALIDLENGTH:      return ( UCHAR * ) &athPhy->invalidLength;
    case APCRADARDETECTED:      return ( UCHAR * ) &athPhy->radarDetected;
    case APCINVALIDSERVICE:     return ( UCHAR * ) &athPhy->invalidService;
    case APCTXOVERRIDERX:       return ( UCHAR * ) &athPhy->txOverrideRx;
    case APCOFDMTIMING:         return ( UCHAR * ) &athPhy->OfdmTiming;
    case APCOFDMSIGNALPARITY:   return ( UCHAR * ) &athPhy->OfdmSignalParity;
    case APCOFDMINVALIDRATE:    return ( UCHAR * ) &athPhy->OfdmInvalidRate;
    case APCOFDMINVALIDLENGTH:  return ( UCHAR * ) &athPhy->OfdmInvalidLength;
    case APCOFDMPOWERDROP:      return ( UCHAR * ) &athPhy->OfdmPowerDrop;
    case APCOFDMSERVICE:        return ( UCHAR * ) &athPhy->OfdmService;
    case APCOFDMRESTART:        return ( UCHAR * ) &athPhy->OfdmRestart;
    case APCCCKTIMING:          return ( UCHAR * ) &athPhy->CckTiming;
    case APCCCKHEADERCRC:       return ( UCHAR * ) &athPhy->CckHeaderCrc;
    case APCCCKINVALIDRATE:     return ( UCHAR * ) &athPhy->CckInvalidRate;
    case APCCCKSERVICE:         return ( UCHAR * ) &athPhy->CckService;
    case APCCCKRESTART:         return ( UCHAR * ) &athPhy->CckRestart;

    default:
      ERROR_MSG ( "" );
  }

  return NULL;
}

/****************************************************************************
*                                                                           *
*                     loadTables() - Load the Tables                        *
*                                                                           *
****************************************************************************/
static void loadTables()
{
  int skfd;                                       // generic raw socket desc
  struct iwreq wrq;                               // ioctl request structure
  struct ifreq ifr;
  struct timeval et;                              // elapsed time
  struct wireless_info info;                      // workarea for wireless ioctl info
  FILE *fp;
  char  bfr[1024], ifName[1024];
  char *s, *t;

  gettimeofday ( &et, ( struct timezone * ) 0 );  // get time-of-day
  if ( et.tv_sec < lastLoad + MINLOADFREQ )       // only reload so often
    return;
  lastLoad = et.tv_sec;

//printf ( "%s %s\n", "avc802dot11.c", " - loadTables()" );
  skfd = openSocket();                            // open socket
  if ( skfd < 0 ) {
    syslog ( LOG_ERR, "SNMP avc802dot11.loadTables() - %s\n", "socket open failure" );
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
          initStructs();
          loadWiExt ( skfd, ifName, &info );
          displayWiExt ( info );
          if ( strstr ( ifName, "wlan" )) 
            loadPrismIfStruct ( skfd, ifName, &info );
        }
      }
    }
    fclose ( fp );
  }

  loadHostAPClients();
  loadMadWiFiStats();
}

/****************************************************************************
*                                                                           *
*              loadHostAPClients() - Load associated clients                *
*                                                                           *
****************************************************************************/
static void loadHostAPClients()
{
  int i;
  DIR *dp;
  FILE *fp;
  char name[512], path[512], bfr[1024];
  struct dirent *dir;

  for ( i = 0; i < MAXRADIOS; i++ ) {
    memset (( char * ) &nPrAc, 0, sizeof ( nPrAc ));
    nPrAc.supports1Mbps = 2;                  // 2 => false
    nPrAc.supports2Mbps = 2;
    nPrAc.supports5Mbps = 2;
    nPrAc.supports11Mbps = 2;
    sprintf ( nPrAc.ifName, "wlan%d\0", i );
    sprintf ( path, "%s/%s/", HOSTAPPATH, nPrAc.ifName );
    dp = opendir ( path );                    // open directory
    if ( dp ) { 
      while  ( dir = readdir ( dp )) {        // loop thru directory entries
        if ( isMACAddress ( dir->d_name )) {  // name look like MAC address?

          sprintf ( name, "%s/%s", path, dir->d_name );
//        printf ( " ===> %s - name: %s <=== \n", "loadTables()", name );

          nPrAc.ifIndex = if_nametoindex ( nPrAc.ifName );
//        nPrAc.ifIndex = i + 1;                              // XXX - for testing //
          strcpy ( nPrAc.macAddress, dir->d_name );
          strtoupper ( nPrAc.macAddress );
          sprintf ( nPrAc.UID, "%04d %s\0", nPrAc.ifIndex, nPrAc.macAddress );

          fp = fopen ( name, "r" );
          if ( fp ) {
            while ( !feof ( fp )) {
              if ( fgets ( bfr, sizeof ( bfr ) - 1, fp )) {
                strtok (( char * ) &bfr, "\n" );          // '\n' => '\0'
//              printf ( "%s\n", bfr );

                if ( strstr ( bfr, MBPS1 ))
                  nPrAc.supports1Mbps = 1;
                if ( strstr ( bfr, MBPS2 ))
                  nPrAc.supports2Mbps = 1;
                if ( strstr ( bfr, MBPS5 ))
                  nPrAc.supports5Mbps = 1;
                if ( strstr ( bfr, MBPS11 ))
                  nPrAc.supports11Mbps = 1;

                if ( strstr ( bfr, SILENCE ))
                  sscanf ( bfr, "last_rx: silence=%d signal=%d rate=%d flow=%d",
                            &nPrAc.lastRxSilence, &nPrAc.lastRxSignal,
                            &nPrAc.lastRxRate,    &nPrAc.lastRxFlow     );

                if ( strstr ( bfr, AP )) 
                  nPrAc.stationType = 2;
                else if ( strstr ( bfr, STA )) 
                  nPrAc.stationType = 1;
                else if ( strstr ( bfr, LISTENINTERVAL ))
                  nPrAc.listenInterval = atol ( &bfr [ strlen ( LISTENINTERVAL )]);
                else if ( strstr ( bfr, JIFFIES ))
                  nPrAc.jiffies= atol ( &bfr [ strlen ( JIFFIES )]);
                else if ( strstr ( bfr, LASTAUTH ))
                  nPrAc.lastAuth= atol ( &bfr [ strlen ( LASTAUTH )]);
                else if ( strstr ( bfr, LASTASSOC ))
                  nPrAc.lastAssoc= atol ( &bfr [ strlen ( LASTASSOC )]);
                else if ( strstr ( bfr, LASTRX ))
                  nPrAc.lastRx = atol ( &bfr [ strlen ( LASTRX )]);
                else if ( strstr ( bfr, LASTTX ))
                  nPrAc.lastTx = atol ( &bfr [ strlen ( LASTTX )]);
                else if ( strstr ( bfr, RXPACKETS ))
                  nPrAc.rxPackets = atol ( &bfr [ strlen ( RXPACKETS )]);
                else if ( strstr ( bfr, TXPACKETS ))
                  nPrAc.txPackets = atol ( &bfr [ strlen ( TXPACKETS )]);
                else if ( strstr ( bfr, RXBYTES ))
                  nPrAc.rxBytes = atoll ( &bfr [ strlen ( RXBYTES )]);
                else if ( strstr ( bfr, TXBYTES ))
                  nPrAc.txBytes = atoll ( &bfr [ strlen ( TXBYTES )]);
                else if ( strstr ( bfr, BFRCNT ))
                  nPrAc.bfrCnt = atol ( &bfr [ strlen ( BFRCNT )]);
                else if ( strstr ( bfr, TXRATE ))
                  nPrAc.txRate = atol ( &bfr [ strlen ( TXRATE )]);
                else if ( strstr ( bfr, TX1M ))
                  nPrAc.tx1Mbps = atol ( &bfr [ strlen ( TX1M )]);
                else if ( strstr ( bfr, TX2M ))
                  nPrAc.tx2Mbps = atol ( &bfr [ strlen ( TX2M )]);
                else if ( strstr ( bfr, TX5M ))
                  nPrAc.tx5Mbps = atol ( &bfr [ strlen ( TX5M )]);
                else if ( strstr ( bfr, TX11M ))
                  nPrAc.tx11Mbps = atol ( &bfr [ strlen ( TX11M )]);
                else if ( strstr ( bfr, RX1M ))
                  nPrAc.rx1Mbps = atol ( &bfr [ strlen ( RX1M )]);
                else if ( strstr ( bfr, RX2M ))
                  nPrAc.rx2Mbps = atol ( &bfr [ strlen ( RX2M )]);
                else if ( strstr ( bfr, RX5M ))
                  nPrAc.rx5Mbps = atol ( &bfr [ strlen ( RX5M )]);
                else if ( strstr ( bfr, RX11M ))
                  nPrAc.rx11Mbps = atol ( &bfr [ strlen ( RX11M )]);
              }
            }
            addList (( char * ) &prAcList, ( char * ) &nPrAc, sizeof ( nPrAc ));
//          printf ( "ifName: %s ifIndex: %d macAddress: %s stationType: %d UID: %s\n", 
//                      nPrAc.ifName, nPrAc.ifIndex, nPrAc.macAddress, 
//                      nPrAc.stationType, nPrAc.UID );
            fclose ( fp );
          }
        }
      }
      closedir ( dp );                        // close directory file
    }
  }
}

/****************************************************************************
*                                                                           *
*              loadMadWiFiStats() - load MadWiFi stats                      *
*                                                                           *
****************************************************************************/
static void loadMadWiFiStats()
{
  int i;
  FILE *fp;
  char name[512], path[512], bfr[1024];

//some day the driver will support more than one radio
//for ( i = 0; i < MAXRADIOS; i++ ) {
    memset (( char * ) &nAthMac, 0, sizeof ( nAthMac ));
    memset (( char * ) &nAthPhy, 0, sizeof ( nAthPhy ));
    sprintf ( nAthMac.ifName, "ath0\0" );
    sprintf ( nAthPhy.ifName, "ath0\0" );
    sprintf ( path, "%s/%s/", MADWIFIPATH, "ath" );
//  sprintf ( nAthMac.ifName, "ath%d\0", i );
//  sprintf ( nAthPhy.ifName, "ath%d\0", i );
//  sprintf ( path, "%s/%s/", MADWIFIPATH, nAthMac.ifName );
    sprintf ( name, "%s/%s", path, "stats" );
//  printf ( "%s - name: %s\n", "loadMadWiFiStats()", name );

    fp = fopen ( name, "r" );
    if ( fp ) {

      nAthMac.ifIndex = if_nametoindex ( nAthMac.ifName );
      nAthPhy.ifIndex = if_nametoindex ( nAthPhy.ifName );
//    nAthMac.ifIndex = i + 1;                              // XXX - for testing //
//    nAthPhy.ifIndex = i + 1;                              // XXX - for testing //
      sprintf ( nAthMac.UID, "%04d\0", nAthMac.ifIndex );
      sprintf ( nAthPhy.UID, "%04d\0", nAthPhy.ifIndex );

      while ( !feof ( fp )) {
        if ( fgets ( bfr, sizeof ( bfr ) - 1, fp )) {
          strtok (( char * ) &bfr, "\n" );                  // '\n' => '\0'
//        printf ( "%s\n", bfr );

          if ( strstr ( bfr, WATCHDOG ))
            nAthMac.watchDog = atol ( &bfr [ strlen ( WATCHDOG )]);
          else if ( strstr ( bfr, HARDWARE ))
            nAthMac.fatalHwInterrupt = atol ( &bfr [ strlen ( HARDWARE )]);
          else if ( strstr ( bfr, BMISS ))
            nAthMac.missedBeacons = atol ( &bfr [ strlen ( BMISS )]);
          else if ( strstr ( bfr, RXORN ))
            nAthMac.rxOverRun = atol ( &bfr [ strlen ( RXORN )]);
          else if ( strstr ( bfr, RXEOL ))
            nAthMac.rxEol = atol ( &bfr [ strlen ( RXEOL )]);
          else if ( strstr ( bfr, TXURN ))
            nAthMac.txUnderRun = atol ( &bfr [ strlen ( TXURN )]);
          else if ( strstr ( bfr, TX_MGMT ))
            nAthMac.txMgmtFrames = atol ( &bfr [ strlen ( TX_MGMT )]);
          else if ( strstr ( bfr, TX_DISCARD ))
            nAthMac.txDiscards = atol ( &bfr [ strlen ( TX_DISCARD )]);
          else if ( strstr ( bfr, TX_INVALID ))
            nAthMac.txInvalid = atol ( &bfr [ strlen ( TX_INVALID )]);
          else if ( strstr ( bfr, TX_QSTOP ))
            nAthMac.txQueueStopped = atol ( &bfr [ strlen ( TX_QSTOP )]);
          else if ( strstr ( bfr, TX_ENCAP ))
            nAthMac.txEncapFailed = atol ( &bfr [ strlen ( TX_ENCAP )]);
          else if ( strstr ( bfr, TX_NONODE ))
            nAthMac.txNoNode = atol ( &bfr [ strlen ( TX_NONODE )]);
          else if ( strstr ( bfr, TX_NOBUF ))
            nAthMac.txNoNode = atol ( &bfr [ strlen ( TX_NOBUF )]);
          else if ( strstr ( bfr, TX_NOBUFMGT ))
            nAthMac.txNoMgmtBuffer = atol ( &bfr [ strlen ( TX_NOBUFMGT )]);
          else if ( strstr ( bfr, TX_XRETRIES ))
            nAthMac.txExcessiveRetries = atol ( &bfr [ strlen ( TX_XRETRIES )]);
          else if ( strstr ( bfr, TX_FIFOERR ))
            nAthMac.txFifoUnderRun = atol ( &bfr [ strlen ( TX_FIFOERR )]);
          else if ( strstr ( bfr, TX_FILTERED ))
            nAthMac.txFiltered = atol ( &bfr [ strlen ( TX_FILTERED )]);
          else if ( strstr ( bfr, TX_SHORTRETRY ))
            nAthMac.txShortRetries = atol ( &bfr [ strlen ( TX_SHORTRETRY )]);
          else if ( strstr ( bfr, TX_LONGRETRY ))
            nAthMac.txLongRetries = atol ( &bfr [ strlen ( TX_LONGRETRY )]);
          else if ( strstr ( bfr, TX_BADRATE ))
            nAthMac.txBadRate = atol ( &bfr [ strlen ( TX_BADRATE )]);
          else if ( strstr ( bfr, TX_NOACK ))
            nAthMac.txNoAck = atol ( &bfr [ strlen ( TX_NOACK )]);
          else if ( strstr ( bfr, TX_RTS ))
            nAthMac.txRts = atol ( &bfr [ strlen ( TX_RTS )]);
          else if ( strstr ( bfr, TX_CTS ))
            nAthMac.txCts = atol ( &bfr [ strlen ( TX_CTS )]);
          else if ( strstr ( bfr, TX_SHORTPRE ))
            nAthMac.txShortPreamble = atol ( &bfr [ strlen ( TX_SHORTPRE )]);
          else if ( strstr ( bfr, TX_RSSI ))
            nAthMac.txRssi = atol ( &bfr [ strlen ( TX_RSSI )]);
          else if ( strstr ( bfr, TX_RSSIDELTA ))
            nAthMac.txRssiDelta = atol ( &bfr [ strlen ( TX_RSSIDELTA )]);
          else if ( strstr ( bfr, RX_ORN ))
            nAthMac.rxOverRun = atol ( &bfr [ strlen ( RX_ORN )]);
          else if ( strstr ( bfr, RX_TOOSHORT ))
            nAthMac.rxFrameTooShort = atol ( &bfr [ strlen ( RX_TOOSHORT )]);
          else if ( strstr ( bfr, RX_CRCERR ))
            nAthMac.rxCrcError = atol ( &bfr [ strlen ( RX_CRCERR )]);
          else if ( strstr ( bfr, RX_FIFOERR ))
            nAthMac.rxFifoOverRun = atol ( &bfr [ strlen ( RX_FIFOERR )]);
          else if ( strstr ( bfr, RX_BADCRYPT ))
            nAthMac.rxCryptFailure = atol ( &bfr [ strlen ( RX_BADCRYPT )]);
          else if ( strstr ( bfr, RX_BADCRYPT ))
            nAthMac.rxCryptFailure = atol ( &bfr [ strlen ( RX_BADCRYPT )]);
          else if ( strstr ( bfr, RX_NOBUF ))
            nAthMac.rxNoSkBuffer = atol ( &bfr [ strlen ( RX_NOBUF )]);
          else if ( strstr ( bfr, RX_RSSI ))
            nAthMac.rxRssi = atol ( &bfr [ strlen ( RX_RSSI )]);
          else if ( strstr ( bfr, RX_RSSIDELTA ))
            nAthMac.rxRssiDelta = atol ( &bfr [ strlen ( RX_RSSIDELTA )]);
          else if ( strstr ( bfr, BE_NOBUF ))
            nAthMac.noBeaconBuffer = atol ( &bfr [ strlen ( BE_NOBUF )]);
          else if ( strstr ( bfr, PER_CAL ))
            nAthMac.calibrations = atol ( &bfr [ strlen ( PER_CAL )]);
          else if ( strstr ( bfr, PER_CALFAIL ))
            nAthMac.failedCalibrations = atol ( &bfr [ strlen ( PER_CALFAIL )]);
          else if ( strstr ( bfr, PER_RFGAIN ))
            nAthMac.rfGainResets = atol ( &bfr [ strlen ( PER_RFGAIN )]);
          else if ( strstr ( bfr, RATE_CALLS ))
            nAthMac.rateControlChecks = atol ( &bfr [ strlen ( RATE_CALLS )]);
          else if ( strstr ( bfr, RATE_RAISE ))
            nAthMac.rateControlRaise = atol ( &bfr [ strlen ( RATE_RAISE )]);
          else if ( strstr ( bfr, RATE_DROP ))
            nAthMac.rateControlDrop = atol ( &bfr [ strlen ( RATE_DROP )]);

          else if ( strstr ( bfr, RX_PHYERR ))
            nAthPhy.totalPhyErrors = atol ( &bfr [ strlen ( RX_PHYERR )]);
          else if ( strstr ( bfr, PHYERR_UNDERRUN ))
            nAthPhy.txUnderRun = atol ( &bfr [ strlen ( PHYERR_UNDERRUN )]);
          else if ( strstr ( bfr, PHYERR_TIMING ))
            nAthPhy.timingErrors = atol ( &bfr [ strlen ( PHYERR_TIMING )]);
          else if ( strstr ( bfr, PHYERR_PARITY ))
            nAthPhy.parityErrors = atol ( &bfr [ strlen ( PHYERR_PARITY )]);
          else if ( strstr ( bfr, PHYERR_RATE ))
            nAthPhy.invalidRate = atol ( &bfr [ strlen ( PHYERR_RATE )]);
          else if ( strstr ( bfr, PHYERR_LENGTH ))
            nAthPhy.invalidLength = atol ( &bfr [ strlen ( PHYERR_LENGTH )]);
          else if ( strstr ( bfr, PHYERR_RADAR ))
            nAthPhy.radarDetected = atol ( &bfr [ strlen ( PHYERR_RADAR )]);
          else if ( strstr ( bfr, PHYERR_SERVICE ))
            nAthPhy.invalidService = atol ( &bfr [ strlen ( PHYERR_SERVICE )]);
          else if ( strstr ( bfr, PHYERR_TOR ))
            nAthPhy.txOverrideRx = atol ( &bfr [ strlen ( PHYERR_TOR )]);
          else if ( strstr ( bfr, PHYERR_OFDM_TIMING )) 
            nAthPhy.OfdmTiming = atol ( &bfr [ strlen ( PHYERR_OFDM_TIMING )]);
          else if ( strstr ( bfr, PHYERR_OFDM_SIGNAL_PARITY ))
            nAthPhy.OfdmSignalParity = atol ( &bfr [ strlen ( PHYERR_OFDM_SIGNAL_PARITY )]);
          else if ( strstr ( bfr, PHYERR_OFDM_RATE_ILLEGAL ))
            nAthPhy.OfdmInvalidRate = atol ( &bfr [ strlen ( PHYERR_OFDM_RATE_ILLEGAL )]);
          else if ( strstr ( bfr, PHYERR_OFDM_LENGTH_ILLEGAL ))
            nAthPhy.OfdmInvalidLength = atol ( &bfr [ strlen ( PHYERR_OFDM_LENGTH_ILLEGAL )]);
          else if ( strstr ( bfr, PHYERR_OFDM_POWER_DROP ))
            nAthPhy.OfdmPowerDrop = atol ( &bfr [ strlen ( PHYERR_OFDM_POWER_DROP )]);
          else if ( strstr ( bfr, PHYERR_OFDM_SERVICE ))
            nAthPhy.OfdmService = atol ( &bfr [ strlen ( PHYERR_OFDM_SERVICE )]);
          else if ( strstr ( bfr, PHYERR_OFDM_RESTART ))
            nAthPhy.OfdmRestart = atol ( &bfr [ strlen ( PHYERR_OFDM_RESTART )]);
          else if ( strstr ( bfr, PHYERR_CCK_TIMING ))
            nAthPhy.CckTiming = atol ( &bfr [ strlen ( PHYERR_CCK_TIMING )]);
          else if ( strstr ( bfr, PHYERR_CCK_HEADER_CRC ))
            nAthPhy.CckHeaderCrc = atol ( &bfr [ strlen ( PHYERR_CCK_HEADER_CRC )]);
          else if ( strstr ( bfr, PHYERR_CCK_RATE_ILLEGAL ))
            nAthPhy.CckInvalidRate = atol ( &bfr [ strlen ( PHYERR_CCK_RATE_ILLEGAL )]);
          else if ( strstr ( bfr, PHYERR_CCK_SERVICE ))
            nAthPhy.CckService = atol ( &bfr [ strlen ( PHYERR_CCK_SERVICE )]);
          else if ( strstr ( bfr, PHYERR_CCK_RESTART ))
            nAthPhy.CckRestart = atol ( &bfr [ strlen ( PHYERR_CCK_RESTART )]);
        }
      }
      fclose ( fp );
      addList (( char * ) &athMacList, ( char * ) &nAthMac, sizeof ( nAthMac ));
      addList (( char * ) &athPhyList, ( char * ) &nAthPhy, sizeof ( nAthPhy ));
    }
//}
}

/****************************************************************************
*                                                                           *
*         loadPrismIfStruct() - load the Prism Interface structure          *
*                                                                           *
****************************************************************************/
static void 
loadPrismIfStruct ( int skfd, char *ifName, struct wireless_info *wi )
{
  int rc, ifIndex = 0;
  struct ifreq ifr;
  char path[512];
  DIR *dp;
  struct dirent *dir;

  strcpy ( ifr.ifr_name, ifName );
  rc = ioctl ( skfd, SIOCGIFHWADDR, &ifr );
  if ( rc >= 0 ) {
    ifIndex = if_nametoindex ( ifName );
    if ( !ifIndex ) {
      syslog ( LOG_ERR, "SNMP %s - %s %s\n", 
        "avc802dot11.loadPrismIfStruct()", ifName, "has no ifIndex" );
      return;
    }

    nPrIf.ifIndex = ifIndex;
    sprintf ( nPrIf.UID, "%04d\0", nPrIf.ifIndex );
    strcpy ( nPrIf.ifName, ifName );
    if ( wi->has_stats ) {
      nPrIf.signal  = wi->stats.qual.level;
      nPrIf.noise   = wi->stats.qual.noise;
      nPrIf.quality = wi->stats.qual.qual;
    }

    sprintf ( path, "%s/%s/", HOSTAPPATH, nPrIf.ifName );
    dp = opendir ( path );                    // open directory
    if ( dp ) { 
      while  ( dir = readdir ( dp )) {        // loop thru directory entries
        if ( isMACAddress ( dir->d_name ))    // name look like MAC address?
          nPrIf.assocCnt++;
      }
      closedir ( dp );                        // close directory file
    }

    addList (( char * ) &prIfList, ( char * ) &nPrIf, sizeof ( nPrIf ));
  }
}

/****************************************************************************
*                                                                           *
*                     initStructs() - initialize structures                 *
*                                                                           *
****************************************************************************/
static void initStructs()
{
  int i;

  // Prism Interface Stucture
  memset (( char * ) &nPrIf, 0, sizeof ( nPrIf ));

  // Wireless Extensions
  wepCurrentKey = 0;
  haveWepCurrentKey = FALSE;
  for ( i = 0; i < MAX_WEP_KEYS; i++ ) {
    prWep[i].len = 0;
    prWep[i].key[0] = '\0';
    prWep[i].haveKey = FALSE;
  }
}

/****************************************************************************
*                                                                           *
*                Wireless Extensions Specific Functions                     *
*                                                                           *
****************************************************************************/
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
        prWep[i].len = wrq.u.data.length;
        prWep[i].haveKey = TRUE;
        t = prWep[i].key;
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

  printf ( "========================================================\n" );
  printf ( "===> Wireless Extension IOCTL calls - avc802dot11.c <===\n" );
  printf ( "========================================================\n" );

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
    if ( prWep[i].haveKey )
      printf ( "%s prWep[%d].len: %d prWep[%d].key: %s\n", 
                "SIOCGIWENCODE", i, prWep[i].len, i, prWep[i].key );
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
  LIST_HEAD ( , avc802dot11Node ) *list;       

  // NOTE: this assumes the UID is at the begining of the 
  //       data structure and that UIDs are strings
  
  list = ( LIST_HEAD ( , avc802dot11Node ) * ) l;   // NOTE: don't know how to get 
  strcpy ( uid, data );                             //  rid of compiler warning on
                                                    //  LISTHEAD typecast
  // create a new node and the data that goes in it
  newNode = malloc ( sizeof ( struct avc802dot11Node ));
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
  LIST_INIT ( &prIfList   );
  LIST_INIT ( &prAcList   );
  LIST_INIT ( &athMacList );
  LIST_INIT ( &athPhyList );
}

/****************************************************************************
*                                                                           *
*                 flushLists() - flush all linked lists                     *
*                                                                           *
****************************************************************************/
static void flushLists()
{
  flushList (( char * ) &prIfList   );
  flushList (( char * ) &prAcList   );
  flushList (( char * ) &athMacList );
  flushList (( char * ) &athPhyList );
}

/****************************************************************************
*                                                                           *
*                   flushList() - flush a linked list                       *
*                                                                           *
****************************************************************************/
static void flushList ( char *l )
{
  LIST_HEAD ( , avc802dot11Node ) *list;
  
  list = ( LIST_HEAD ( , avc802dot11Node ) * ) l;   // NOTE: don't know how to get 
  while ( !LIST_EMPTY ( list )) {                   //  rid of compiler warning on
    np = LIST_FIRST ( list );                       //  LISTHEAD typecast
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
*        isMACAddress() - this string look kinda like a MAC address?        *
*                                                                           *
****************************************************************************/
static int isMACAddress ( char *s )
{

  if ( strlen ( s ) == 17 &&
       s[2]  == ':'       && s[5]  == ':'       &&
       s[8]  == ':'       && s[11] == ':'       &&
       s[14] == ':'       &&
       isxdigit ( s[0]  ) && isxdigit ( s[1]  ) &&
       isxdigit ( s[3]  ) && isxdigit ( s[4]  ) &&
       isxdigit ( s[6]  ) && isxdigit ( s[7]  ) &&
       isxdigit ( s[9]  ) && isxdigit ( s[10] ) &&
       isxdigit ( s[12] ) && isxdigit ( s[13] ) &&
       isxdigit ( s[15] ) && isxdigit ( s[16] ))
    return ( TRUE );
  else
    return ( FALSE );
}

/****************************************************************************
*                                                                           *
*           strtoupper() - converts a string to upper case characters       *
*                                                                           *
****************************************************************************/
static void strtoupper ( char *s )
{
  while ( *s ) {
    *s = toupper ( *s );
    *s++;
  }
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
*             htoui - converts hex string to unsigned integer               *
*                                                                           *
****************************************************************************/
static unsigned int htoui ( char *cptr )
{
  unsigned int i, j = 0;

  while (cptr && *cptr && isxdigit(*cptr)) {
    i = *cptr++ - '0';
    if (9 < i)
      i -= 7;
    j <<= 4;
    j |= (i & 0x0f);
  }
  return ( j );
}

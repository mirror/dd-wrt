#define VISUALSOURCE 1

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <cyutils.h>
#include <code_pattern.h>
#include <broadcom.h>
#include <proto/802.11.h>

#define SITE_SURVEY_DB  "/tmp/site_survey"
#define SITE_SURVEY_NUM 256

struct site_survey_list
{
    char SSID[33];
    unsigned char BSSID[18];
    uint8 channel;		/* Channel no. */
    int16 RSSI;			/* receive signal strength (in dBm) */
    int16 phy_noise;		/* noise (in dBm) */
    uint16 beacon_period;	/* units are Kusec */
    uint16 capability;		/* Capability information */
    unsigned char ENCINFO[128];	/* encryption info */
    uint rate_count;		/* # rates in this set */
    uint8 dtim_period;		/* DTIM period */
} site_survey_lists[SITE_SURVEY_NUM];

static int open_site_survey( void )
{
    FILE *fp;

    bzero( site_survey_lists, sizeof( site_survey_lists ) );

    if( ( fp = fopen( SITE_SURVEY_DB, "r" ) ) )
    {
	fread( &site_survey_lists[0], sizeof( site_survey_lists ), 1, fp );
	fclose( fp );
	return TRUE;
    }
    return FALSE;
}

#ifdef FBNFW

void ej_list_fbn( webs_t wp, int argc, char_t ** argv )
{
    int i;

    system2( "site_survey" );

    open_site_survey(  );
    for( i = 0; i < SITE_SURVEY_NUM; i++ )
    {

	if( site_survey_lists[i].SSID[0] == 0 ||
	    site_survey_lists[i].BSSID[0] == 0 ||
	    site_survey_lists[i].channel == 0 )
	    break;

	if( startswith( site_survey_lists[i].SSID, "www.fbn-dd.de" ) )
	{
	    websWrite( wp, "<option value=\"" );
	    tf_webWriteJS( wp, site_survey_lists[i].SSID );
	    websWrite( wp, "\">" );
	    tf_webWriteJS( wp, site_survey_lists[i].SSID );
	    websWrite( wp, "</option>\n" );
	}

    }
}

#endif
void ej_dump_site_survey( webs_t wp, int argc, char_t ** argv )
{
    int i;
    char buf[10] = { 0 };
    char *rates = NULL;
    char *name;

    name = websGetVar( wp, "hidden_scan", NULL );
    if( name == NULL || strlen( name ) == 0 )
	system2( "site_survey" );
    else
    {
	sysprintf( "site_survey \"%s\"", name );
    }

    open_site_survey(  );

    for( i = 0; i < SITE_SURVEY_NUM; i++ )
    {

	if( site_survey_lists[i].BSSID[0] == 0 ||
	    site_survey_lists[i].channel == 0 )
	    break;

	// fix for " in SSID
	char *tssid =
	    ( site_survey_lists[i].SSID[0] ==
	      0 ) ? "hidden" : &site_survey_lists[i].SSID[0];
	int pos = 0;
	int tpos;
	int ssidlen = strlen( tssid );

	while( pos < ssidlen )
	{
	    if( tssid[pos] == '\"' )
	    {
		for( tpos = ssidlen; tpos > pos - 1; tpos-- )
		    tssid[tpos + 1] = tssid[tpos];

		tssid[pos] = '\\';
		pos++;
		ssidlen++;
	    }
	    pos++;
	}
	// end fix for " in SSID

	if( site_survey_lists[i].rate_count == 4 )
	    rates = "4(b)";
	else if( site_survey_lists[i].rate_count == 12 )
	    rates = "12(g)";
	else
	{
	    rates = buf;
	    snprintf( rates, 9, "%d", site_survey_lists[i].rate_count );
	}

	/*
	 * #define DOT11_CAP_ESS 0x0001 #define DOT11_CAP_IBSS 0x0002 #define 
	 * DOT11_CAP_POLLABLE 0x0004 #define DOT11_CAP_POLL_RQ 0x0008 #define 
	 * DOT11_CAP_PRIVACY 0x0010 #define DOT11_CAP_SHORT 0x0020 #define
	 * DOT11_CAP_PBCC 0x0040 #define DOT11_CAP_AGILITY 0x0080 #define
	 * DOT11_CAP_SPECTRUM 0x0100 #define DOT11_CAP_SHORTSLOT 0x0400
	 * #define DOT11_CAP_CCK_OFDM 0x2000 
	 */
	char *open =
	    // (site_survey_lists[i].capability & DOT11_CAP_PRIVACY) ? "No" : 
	    // "Yes";
	    ( site_survey_lists[i].
	      capability & DOT11_CAP_PRIVACY ) ? live_translate( "share.no" )
	    : live_translate( "share.yes" );

	char *netmode;
	long netmodecap = site_survey_lists[i].capability;

	netmodecap &= ( DOT11_CAP_ESS | DOT11_CAP_IBSS );
	if( netmodecap == DOT11_CAP_ESS )
	    netmode = "AP";
	else if( netmodecap == DOT11_CAP_IBSS )
	    netmode = "AdHoc";
	else
	    netmode = live_translate( "share.unknown" );

	websWrite( wp, "%c\"", i ? ',' : ' ' );
	tf_webWriteJS( wp, tssid );
	websWrite( wp,
		   "\",\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\",\"%s\",\"%d\",\"%s\"\n",
		   netmode, site_survey_lists[i].BSSID,
		   site_survey_lists[i].channel, site_survey_lists[i].RSSI,
		   site_survey_lists[i].phy_noise,
		   site_survey_lists[i].beacon_period, open,
		   site_survey_lists[i].ENCINFO,
		   site_survey_lists[i].dtim_period, rates );

    }

    return;
}

#ifdef HAVE_WIVIZ

void ej_dump_wiviz_data( webs_t wp, int argc, char_t ** argv )	// Eko, for
								// testing
								// only
{
    FILE *f;
    char buf[256];

    killall( "autokill_wiviz", SIGTERM );
    eval( "autokill_wiviz" );
    eval( "run_wiviz" );

    if( ( f = fopen( "/tmp/wiviz2-dump", "r" ) ) != NULL )
    {
	while( fgets( buf, sizeof( buf ), f ) )
	{
	    websWrite( wp, "%s", buf );
	}
	fclose( f );
    }
    else			// dummy data - to prevent first time js
				// error
    {
	websWrite( wp, "top.hosts = new Array();\nvar hnum = 0;\nvar h;\n" );
	websWrite( wp,
		   "var wiviz_cfg = new Object();\n wiviz_cfg.channel = 6\n" );
	websWrite( wp, "top.wiviz_callback(top.hosts, wiviz_cfg);\n" );
	websWrite( wp, "function wiviz_callback(one, two) {\n" );
	websWrite( wp,
		   "alert(\'This asp is intended to run inside Wi-Viz.  You will now be redirected there.\');\n" );
	websWrite( wp, "location.replace('Wiviz_Survey.asp');\n}\n" );
    }
}

#endif

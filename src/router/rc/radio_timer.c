/*
 * Radio timer by Eko: 19.jul.2006
 * 
 * hours are represented as bits in 24 bit = xxxxxxxxxxxxxxxxxxxxxxxx from
 * GUI code scans for changes: 10 = radio off, 01 = radio on firsttime
 * change: 00 and 10 = radio off, 11 and 01 = radio on 
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <cy_conf.h>
#include <rc.h>
#include <shutils.h>
#include <syslog.h>
#include <utils.h>
#include <wlutils.h>

#define start_service(a) eval("startservice",a);
#define stop_service(a) eval("stopservice",a);

int main( int argc, char **argv )
{

    long radiotime0;		// 4 byte int number (24 bits from gui + 1

    // bit for midnight)
#ifdef HAVE_MSSID
    long radiotime1;		// 4 byte int number (24 bits from gui + 1

    // bit for midnight)
#endif

    int firsttime, needchange;

    needchange = 1;
    firsttime = 1;

    struct tm *currtime;
    long tloc;

    do
    {
	time( &tloc );		// get time in seconds since epoch
	currtime = localtime( &tloc );	// convert seconds to date structure

	if( currtime->tm_year > 100 )	// ntp time must be set
	{

	    radiotime0 = strtol( nvram_get( "radio0_on_time" ), NULL, 2 );	// convert 
										// 
	    // 
	    // binary 
	    // string 
	    // to 
	    // long 
	    // int
	    radiotime0 += ( ( radiotime0 & 1 ) << 24 );	// duplicate 23-24h
	    // bit to the start
	    // to take care of
	    // midnight
	    radiotime0 = ( radiotime0 >> ( 24 - currtime->tm_hour - 1 ) ) & 3;	// get 
										// 
	    // 
	    // pattern 
	    // only 
	    // (last 
	    // two 
	    // bits)
#ifdef HAVE_MSSID
	    radiotime1 = strtol( nvram_get( "radio1_on_time" ), NULL, 2 );	// convert 
										// 
	    // 
	    // binary 
	    // string 
	    // to 
	    // long 
	    // int
	    radiotime1 += ( ( radiotime1 & 1 ) << 24 );	// duplicate 23-24h
	    // bit to the start
	    // to take care of
	    // midnight
	    radiotime1 = ( radiotime1 >> ( 24 - currtime->tm_hour - 1 ) ) & 3;	// get 
										// 
	    // 
	    // pattern 
	    // only 
	    // (last 
	    // two 
	    // bits)
#endif

	    if( currtime->tm_min != 0 )
		needchange = 1;	// prevet to be executed more than once when
	    // min == 0

	    if( firsttime )	// first time change
	    {
		switch ( radiotime0 )
		{
		    case 3:	// 11
			radiotime0 = 1;	// 01
			break;
		    case 0:	// 00
			radiotime0 = 2;	// 10
			break;
		}
#ifdef HAVE_MSSID
		switch ( radiotime1 )
		{
		    case 3:	// 11
			radiotime1 = 1;	// 01
			break;
		    case 0:	// 00
			radiotime1 = 2;	// 10
			break;
		}
#endif
	    }

	    if( nvram_match( "radio0_timer_enable", "0" ) )
		radiotime0 = 0;
	    if( nvram_match( "radio1_timer_enable", "0" ) )
		radiotime1 = 0;

	    if( ( ( needchange ) && currtime->tm_min == 0 ) || ( firsttime ) )	// change 
										// 
		// 
		// when 
		// min 
		// = 
		// 0 
		// or 
		// firstime
	    {
		switch ( radiotime0 )
		{
		    case 0:
			break;	// do nothing, radio0 timer disabled

		    case 1:	// 01 - turn radio on
			syslog( LOG_DEBUG, "Turning radio 0 on\n" );
#ifdef HAVE_MADWIFI
			eval( "ifconfig", "ath0", "up" );
#elif HAVE_MSSID
			if( pidof( "nas" ) > 0 || pidof( "wrt-radauth" ) > 0 )
			{
			    stop_service( "nas" );
			}
			eval( "wl", "-i", get_wl_instance_name( 0 ), "radio",
			      "on" );
			eval( "startservice", "nas" );
			start_service( "guest_nas" );
#else
			eval( "wl", "radio", "on" );
#endif
			break;

		    case 2:	// 10 - turn radio off
			syslog( LOG_DEBUG, "Turning radio 0 off\n" );
#ifdef HAVE_MADWIFI
			eval( "ifconfig", "ath0", "down" );
#elif HAVE_MSSID
			if( pidof( "nas" ) > 0 || pidof( "wrt-radauth" ) > 0 )
			{
			    stop_service( "nas" );
			}
			eval( "wl", "-i", get_wl_instance_name( 0 ), "radio",
			      "off" );
#else
			eval( "wl", "radio", "off" );
#endif
			break;
		}
#ifdef HAVE_MSSID
		switch ( radiotime1 )
		{
		    case 0:
			break;	// do nothing, radio1 timer disabled

		    case 1:	// 01 - turn radio on
			syslog( LOG_DEBUG, "Turning radio 1 on\n" );
			if( pidof( "nas" ) > 0 || pidof( "wrt-radauth" ) > 0 )
			{
			    stop_service( "nas" );
			}
			eval( "wl", "-i", get_wl_instance_name( 1 ), "radio",
			      "on" );
			eval( "startservice", "nas" );
			start_service( "guest_nas" );
			break;

		    case 2:	// 10 - turn radio off
			syslog( LOG_DEBUG, "Turning radio 1 off\n" );
			if( pidof( "nas" ) > 0 || pidof( "wrt-radauth" ) > 0 )
			{
			    stop_service( "nas" );
			}
			eval( "wl", "-i", get_wl_instance_name( 1 ), "radio",
			      "off" );
			break;
		}
#endif
		needchange = 0;
		firsttime = 0;
	    }

	}
	else			// if yr < 100 (=2000) wait 5 min and try
	    // again (if ntp time is maybe set now)
	{
	    sleep( 242 );
	}

	sleep( 58 );		// loop every 58 s to be sure to catch min == 
				// 
	// 
	// 0

    }
    while( 1 );

    return 0;

}

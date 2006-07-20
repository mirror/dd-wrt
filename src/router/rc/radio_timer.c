// Radio timer by Eko: 19.jul.2006
//
// hours are represented as bits in 24 bit = 3 byte number (xxxxxxxx xxxxxxxx xxxxxxxx) from GUI 0xXXXXXX
// code scans for changes: 10 = radio off, 01 = radio on
// firsttime change: 00 and 10 = radio off, 11 and 01 = radio on


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <cy_conf.h>
#include <rc.h>
#include <shutils.h>


int
radio_timer_main (void)
{

long radiotime;  //4 byte int number (3 bytes from gui + 1 for midnight)
int firsttime, needchange;
int yr, hr, min;

needchange = 1;
firsttime = 1;

struct tm *currtime;
long tloc;


do
  {
	time (&tloc); // get time in seconds since epoch
	currtime=localtime(&tloc); // convert seconds to date structure

	yr = currtime->tm_year;
	printf("year is %d\n",yr);  //remove

#ifdef HAVE_MSSID      
	if ((yr > 100) && nvram_invmatch ("wl0_radio_mode", "disabled"))	//ntp time must be set  && radio must be on
#else
	if ((yr > 100) && nvram_invmatch ("wl_radio_mode", "disabled"))
#endif 
	{
		radiotime = (long)nvram_safe_get ("radio_on_time");  //can nvram hex be converted to int???? 
				printf("radiotime nvram = %d\n",radiotime); //remove	
		radiotime += ((radiotime & 1) << 24); //duplicate 23-24h bit to the start to take care of midnight

		hr = currtime->tm_hour;
		min = currtime->tm_min;
			printf("hour is %d\n",hr);  //remove
			printf("min is %d\n",min);	//remove

		radiotime = (radiotime >> (24 - hr - 1)) & 3;  //check pattern (last two bits)
			printf("radiotime mask %d\n",radiotime); //remove

		if (min != 0)  
			 needchange = 1;	// prevet to be executed more than once when min == 0

		if (firsttime)  //first time change
		{
		switch (radiotime)
			{
			case 1:
			case 3:
				eval ("wl", "radio", "on");
				break;
			case 0:
			case 2:
				eval ("wl", "radio", "off");
				break;
			}
		firsttime = 0;
		needchange = 0;
		}

		if ((min == 0) && (needchange))  //normal change when hour change
		{
		switch (radiotime)
			{
			case 1:
				eval ("wl", "radio", "on"); 
				break;
			case 2:
				eval ("wl", "radio", "off");
				break;
			}
		needchange = 0;
		}

	}
	else  //if yr < 100 (=2000) wait 5 min and try again (if ntp time is maybe set now)
	{
		printf("SSSS\n");
	sleep(242);

	}
		printf("sleeping\n");
	sleep(58); // loop every 58 s to be sure to catch min == 0
  }

while (1);

        return 0;

}

#include <stdio.h>
#include <stdlib.h>






char *get3GControlDevice(void)
{
	char *ttsdevice="/dev/usb/tts/0";
	FILE *modem =fopen("/sys/bus/usb/devices/1-1/idProduct","rb");
	if (!modem)
	    return ttsdevice;
	char product[32];
	fscanf(modem,"%s",product);
	fclose(modem);    
	modem =fopen("/sys/bus/usb/devices/1-1/idVendor","rb");
	if (!modem)
	    return ttsdevice;
	char vendor[32];
	fscanf(modem,"%s",vendor);
	fclose(modem);    
	int idVendor=atoi(vendor);
	int idProduct=atoi(product);
	if (idVendor==1199 && idProduct==6880)
	    {
	    //sierra wireless 
	    fprintf(stderr,"Sierra Wireless Compass 885 deteted\n");
	    ttsdevice="/dev/usb/tts/3";
	    }
return ttsdevice;
}
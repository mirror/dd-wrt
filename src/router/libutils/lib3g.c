#include <stdio.h>
#include <stdlib.h>






char *get3GControlDevice(void)
{
	char *ttsdevice="/dev/usb/tts/0";
	FILE *modem =fopen("/sys/bus/usb/devices/1-1/idProduct","rb");
	if (!modem)
	    return ttsdevice;
	int idProduct;
	int idVendor;
	fscanf(modem,"%X",&idProduct);
	fclose(modem);    
	modem =fopen("/sys/bus/usb/devices/1-1/idVendor","rb");
	if (!modem)
	    return ttsdevice;
	fscanf(modem,"%X",&idVendor);
	fclose(modem);    
	if (idVendor==0x1199 && idProduct==0x6880)
	    {
	    //sierra wireless 
	    fprintf(stderr,"Sierra Wireless Compass 885 deteted\n");
	    ttsdevice="/dev/usb/tts/3";
	    }
	if (idVendor==0x12d1 && idProduct==0x1003)
	    {
	    //huawei
	    fprintf(stderr,"HUAWEI/Option E172 detected\n");
	    ttsdevice="/dev/usb/tts/0";
	    }
return ttsdevice;
}
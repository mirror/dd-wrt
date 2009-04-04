#include <stdio.h>
#include <stdlib.h>



static int scanFor(int Vendor,int Product)
{
int count=1;
while (1)
    {
    char sysfs[64];
    sprintf(sysfs,"/sys/bus/usb/devices/%d-0:1.0/bInterfaceNumber",count);
    FILE *probe =fopen(sysfs,"rb");
    if (!probe)
	return 0;
    fclose(probe);
    sprintf(sysfs,"/sys/bus/usb/devices/%d-1/idProduct",count);
    FILE *modem =fopen(sysfs,"rb");
    if (!modem)
	{
	count++;
	continue;
	}
    int idProduct;
    int idVendor;
    fscanf(modem,"%X",&idProduct);
    fclose(modem);    
    sprintf(sysfs,"/sys/bus/usb/devices/%d-1/idVendor",count);
    modem =fopen(sysfs,"rb");
    if (!modem)
	{
	count++;
	continue;
	}
    fscanf(modem,"%X",&idVendor);
    fclose(modem);
    if (idVendor==Vendor && idProduct==Product)
	return 1;
    count++;
    }

}


char *get3GControlDevice(void)
{
	char *ttsdevice="/dev/usb/tts/0";
	if (scanFor(0x1199,0x6880))
	    {
	    //sierra wireless 
	    fprintf(stderr,"Sierra Wireless Compass 885 deteted\n");
	    return "/dev/usb/tts/3";
	    }
	if (scanFor(0x12d1,0x1003))
	    {
	    //huawei
	    fprintf(stderr,"HUAWEI/Option E172 detected\n");
	    return "/dev/usb/tts/0";
	    }
	if (scanFor(0x0af0,0x7011))
	    {
	    //huawei
	    fprintf(stderr,"HUAWEI/Option E301 HSUPA detected\n");
	    return "/dev/usb/tts/0";
	    }
return ttsdevice;
}
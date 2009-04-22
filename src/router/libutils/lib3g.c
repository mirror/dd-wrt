#include <stdio.h>
#include <stdlib.h>
#include <shutils.h>



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
	#ifdef HAVE_CAMBRIA
	    eval("gpio","enable","26");
	    eval("gpio","disable","27");
	    if (nvram_match("wan_select","1"))
		{
		eval("gpio","enable","26");
		eval("gpio","disable","27");
		}
	    if (nvram_match("wan_select","2"))
		{
		eval("gpio","disable","26");
		eval("gpio","enable","27");
		}
	    if (nvram_match("wan_select","3"))
		{
		eval("gpio","enable","26");
		eval("gpio","enable","27");
		}
	#endif
	nvram_set("3gdata","/dev/usb/tts/0");
	if (scanFor(0x1199,0x6880))
	    {
	    //sierra wireless 
	    fprintf(stderr,"Sierra Wireless Compass 885 deteted\n");
	    nvram_set("3gdata","/dev/usb/tts/4");
	    return "/dev/usb/tts/3";
	    }
	if (scanFor(0x1199,0x683C))
	    {
	    //sierra wireless 
	    fprintf(stderr,"Sierra Wireless MC8790\n");
	    nvram_set("3gdata","/dev/usb/tts/4");
	    return "/dev/usb/tts/3";
	    }
	if (scanFor(0x1199,0x683D))
	    {
	    //sierra wireless 
	    fprintf(stderr,"Sierra Wireless MC8790\n");
	    nvram_set("3gdata","/dev/usb/tts/4");
	    return "/dev/usb/tts/3";
	    }
	if (scanFor(0x1199,0x683E))
	    {
	    //sierra wireless 
	    fprintf(stderr,"Sierra Wireless MC8790\n");
	    nvram_set("3gdata","/dev/usb/tts/4");
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
	if (scanFor(0x12d1,0x1001))
	    {
	    //huawei
	    fprintf(stderr,"HUAWEI/Option E600 detected\n");
	    return "/dev/usb/tts/0";
	    }
	if (scanFor(0x12d1,0x1412))
	    {
	    //huawei
	    fprintf(stderr,"HUAWEI/Option EC168 detected\n");
	    return "/dev/usb/tts/0";
	    }
	if (scanFor(0x1199,0x6832))
	    { 
	    //sierra wireless mc 8780
	    fprintf(stderr,"Sierra Wireless MC 8780 detected\nreset card\n");
	    system("stty 115200 -echo -echoe -echok -echoctl -echoke -ignbrk -ixon </dev/usb/tts/0");
	    system("stty 115200 -echo -echoe -echok -echoctl -echoke -ignbrk -ixon </dev/usb/tts/2");
	    eval("comgt","-d","/dev/usb/tts/2","-s","/etc/comgt/reset.comgt");
	    FILE *check=NULL;
	    int count=0;
	    sleep(1);
	    while (!(check=fopen("/dev/usb/tts/2","rb")) && count<10)
		{
		sleep(1);
		count++;
		}
	    if (check)
		fclose(check);
	    else
		fprintf(stderr,"reset error\n");
	    system("stty 115200 -echo -echoe -echok -echoctl -echoke -ignbrk -ixon </dev/usb/tts/0");
	    system("stty 115200 -echo -echoe -echok -echoctl -echoke -ignbrk -ixon </dev/usb/tts/2");
	    fprintf(stderr,"wakeup card\n");
	    eval("comgt","-d","/dev/usb/tts/2","-s","/etc/comgt/wakeup.comgt");
	    sleep(5);//give extra delay for registering
	    return "/dev/usb/tts/2";
	    }
return ttsdevice;
}
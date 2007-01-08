#include <stdio.h>
#include <wlutils.h>



int main(int argc,char *argv[])
{
if (argc<2)
    {
    fprintf(stderr,"invalid argument\n");
    return 0;
    }
char *ifname="ath0";
int i;
for (i=1;i<argc;i++)
    {
    if (!strcmp(argv[i],"-i"))
	{
	ifname = argv[i++];
	continue;
	}
    if (!strcmp(argv[i],"assoclist"))
	{
	char buf[8192];
	getassoclist(ifname,buf);
	int count;
	memcpy(&count,buf,4);
	unsigned char *p=&buf[4];
	int a;
	int pos=0;
	for (a=0;a<count;a++)
	    {
	    fprintf(stdout,"assoclist %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",p[pos],p[pos+1],p[pos+2],p[pos+3],p[pos+4],p[pos+5]);
	    pos+=6;
	    }
	}
    if (!strcmp(argv[i],"rssi"))
	{
	unsigned char rmac[6];
	ether_atoe(argv[++i],rmac);
	int rssi = getRssi(ifname,rmac);
	fprintf(stdout,"rssi is %d\n",rssi);
	}
    if (!strcmp(argv[i],"noise"))
	{
	unsigned char rmac[6];
	ether_atoe(argv[++i],rmac);
	int rssi = getNoise(ifname,rmac);
	fprintf(stdout,"noise is %d\n",rssi);
	}
    }
}



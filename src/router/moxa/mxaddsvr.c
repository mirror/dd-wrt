#include "nport.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#define     ER_ARG      -10

int minor[256];
char *tmptty, *tmpcout;
int idx;
int dataport, cmdport;


char* concate(char *str, char *c, char *ret)
{
    memset(ret, '\0', 20);
    sprintf(ret, "%s%s", str, c);
    return;
}


int getMinor()
{
    int i, j;
    for (i=0; i<256; i++)
    {
        for (j=0; j<idx; j++)
        {
            if (i == minor[j])
            {
                break;
            }
        }
        if (j == idx)
        {
            minor[idx++] = i;
            return i;
        }
    }
    if (i >= 256)
    {
        return -1;
    }
}


void c_hex(int c, char *ret)
{

    if (c >= 10)
    {
        switch (c)
        {
        case 10:
            sprintf(ret, "a");
            break;
        case 11:
            sprintf(ret, "b");
            break;
        case 12:
            sprintf(ret, "c");
            break;
        case 13:
            sprintf(ret, "d");
            break;
        case 14:
            sprintf(ret, "e");
            break;
        case 15:
            sprintf(ret, "f");
            break;
        }
        return;
    }
    else
    {
        sprintf(ret, "%d", c);
        return;
    }
}


void getTty(char *ret)
{
    int i, j;
    char *x1, *x2;
    x1 = (char*)malloc(sizeof(char *));
    x2 = (char*)malloc(sizeof(char *));

    for (i=0; i<16; i++)
    {
        for (j=0; j<16; j++)
        {
            c_hex(i, x1);
            c_hex(j, x2);
            sprintf(ret, "ttyr%s%s", x1, x2);
            if (strstr(tmptty, ret) == NULL)
            {
                sprintf(tmptty, "%s[%s]", tmptty, ret);
                free (x1);
                free (x2);
                return;
            }
        }
    }
    free (x1);
    free (x2);
    return;
}



void getCout(char *ret)
{
    int i, j;
    char *x1, *x2;
    x1 = (char*)malloc(sizeof(char *));
    x2 = (char*)malloc(sizeof(char *));

    for (i=0; i<16; i++)
    {
        for (j=0; j<16; j++)
        {
            c_hex(i, x1);
            c_hex(j, x2);
            sprintf(ret, "cur%s%s", x1, x2);
            if (strstr(tmpcout, ret) == NULL)
            {
                sprintf(tmpcout, "%s[%s]", tmpcout, ret);
                free (x1);
                free (x2);
                return;
            }
        }
    }
    free (x1);
    free (x2);
    return;
}


void showMinor()
{
    int s;
    for (s=0; s<idx; s++)
    {
        printf("minor[%d] = (%d)\n", s, minor[s]);
    }
}


int check_usage(int arg, char *argv[])
{
    int i;
    char buf[10];

    if (arg == 3)
    {
        dataport = 950;
        cmdport = 966;

    }
    else if (arg == 5)
    {
        memset(buf, '\0', 10);
        strcpy(buf, argv[3]);   // data port
        for (i=0; i<strlen(buf); i++)
        {
            if (!isdigit(buf[i]))
            {
                printf("\nArgument error: [data port] is not a digital number.\n\n");
                return ER_ARG;
            }
        }
        dataport = atoi(buf);

        memset(buf, '\0', 10);
        strcpy(buf, argv[4]);   // data port
        for (i=0; i<strlen(buf); i++)
        {
            if (!isdigit(buf[i]))
            {
                printf("\nArgument error: [command port] is not a digital number.\n\n");
                return ER_ARG;
            }
        }
        cmdport = atoi(buf);

    }
    else
    {
        printf("usage: ./mxaddsvr [ip] [totalport] ([data port] [cmd port])\n");
        printf("[ip]\n");
        printf("  =NPort IP Address\n");
        printf("[totalport]\n");
        printf("  =Serial ports number of NPort\n");
        printf("  If [data port/cmd port] isn't specified, start with 1st port.\n");
        printf("[data port/cmd port]\n");
        printf(" =In general, 1st=950/966, 2nd=951/967...\n");
        printf("  This is optional argument.\n");
        printf("\n");
        printf("ex:\n");
        printf(" NPort=NPort 5210, 2 serial ports, 192.168.8.51\n");
        printf(" # ./mxaddsvr 192.168.8.51 2\n");
        printf("\n\n");
        return ER_ARG;
    }
    return 0;
}

int main(int arg, char *argv[])
{
    int i, j;
    int total, len, overwrite, fifo, mn, ssl;
    int ttymajor, calloutmajor;
    char c;
    char *tmpstr, *tmp1, *tmp2;
    char tmpm[10], tmpt[40], tmpc[40], major[40];
    char ip[40];
    struct in_addr addr;
    unsigned long ipaddr;
    FILE *f, *ft, *frc, *fos;
    char *os = "linux";

    overwrite = -1;
    system("clear");

    if (check_usage(arg, argv) != 0)
    {
        return 0;
    }

    printf("\nAdding Server...\n\n");

    memset(ip, '\0', 40);
    if(strlen(argv[1]) > 39){
    	printf("The server name length over 39!\n\n");
        return -1;
    }
    sprintf(ip, "%s", argv[1]);
    /*
        ipaddr = inet_addr(argv[1]);
        addr.s_addr = ipaddr;
        sprintf(tip, "%s", inet_ntoa(addr));
        if ((strcmp(ip, "255.255.255.255") == 0) || (ipaddr < 0)) {
            printf("Invalid IP Address !\n\n");
            return -1;
        }else if(strcmp(tip, ip) != 0){
            printf("Invalid IP Address !\n\n");
            return -1;
        }
    */
    if (strcmp(ip, "255.255.255.255") == 0)
    {
        printf("Invalid IP Address!\n\n");
        return -1;
    }

    tmpstr = (char *)malloc(256);
    len = 256;
    tmp1 = (char *)malloc(40);
    tmp2 = (char *)malloc(40);
    tmptty = (char *)malloc(2560);
    tmpcout = (char *)malloc(2560);

    memset(minor, -1, sizeof(int)*256);
    idx = 0;

    /* get OS */
    fos = fopen ("/etc/redhat-release", "r");
    if (fos != NULL)
    {
        fclose(fos);
        os = "linux";
    }
    else
    {
        fos = fopen ("/etc/SuSE-release", "r");
        if (fos != NULL)
        {
            fclose(fos);
            os = "SuSE";
        }
        else
        {
            fos = fopen ("/etc/debian_version", "r");
            if (fos != NULL)
            {
                fclose(fos);
                os = "debian";
            }
            else
            {
                fos = fopen ("/etc/gentoo-release", "r");
                if (fos != NULL)
                {
                    fclose(fos);
                    os = "gentoo";
                }
            }
        }
    }

    sprintf(tmpstr, "%s/npreal2d.cf", DRIVERPATH);
    f = fopen (tmpstr, "r");
    if (f == NULL)
    {
        printf("file open error_3\n");
        free(tmpstr);
        free(tmp1);
        free(tmp2);
        free(tmptty);
        free(tmpcout);
        return(0);
    }
    ft = fopen ("/tmp/npr_tmpfile2", "w");
    if (ft == NULL)
    {
        printf("file open error_4\n");
        free(tmpstr);
        free(tmp1);
        free(tmp2);
        free(tmptty);
        free(tmpcout);
        return(0);
    }

    for (;;)
    {
        /* end of file */
        if (getline (&tmpstr, &len, f) < 0)
        {
            break;
        }
        /* comment */
        if (strstr(tmpstr, "#") != NULL)
        {
            fputs (tmpstr, ft);
            continue;
        }

        memset(major, '\0', 20);
        sscanf(tmpstr, "%s", major);

        if (strstr(major, "ttymajor") != NULL)
        {
            ttymajor = atoi(strstr(major, "=")+1);
            fputs (tmpstr, ft);
            continue;
        }

        if (strstr(major, "calloutmajor") != NULL )
        {
            calloutmajor = atoi(strstr(major, "=")+1);
            fputs (tmpstr, ft);
            continue;
        }
        concate(ip, "\t", tmp1);
        concate(ip, " ", tmp2);
        if (strstr(tmpstr, tmp1) != NULL ||
                strstr(tmpstr, tmp2) != NULL)
        {
            if (overwrite == -1)
            {
                printf("The specified server has been configured before, \nare you sure to overwrite the settings [y/N]? ");
                scanf("%c", &c);
                if (c != 'Y' && c != 'y')
                {
                    overwrite = -2;
                }
                else
                {
                    overwrite = 1;
                }
            }
        }
        else
        {
            fputs (tmpstr, ft);
        }

        /* gather info (minor, ttyname, callout) */
        sscanf(tmpstr, "%s%s%s%s%s%s%s%s", tmpm, tmpt, tmpt, tmpt, tmpt, tmpt, tmpt, tmpc);
        sprintf(tmptty, "%s[%s]", tmptty, tmpt);
        sprintf(tmpcout, "%s[%s]", tmpcout, tmpc);
        minor[idx] = atoi(tmpm);
        idx++;
    }
    fclose(ft);
    fclose(f);

    total = atoi(argv[2]);
    if(total > 32){
    	total = 32;
    }
    if ((idx + total) > 256)
    {
        printf("The number of installed port exceeds the maxinum(256). \nPlease Check the configuration file.\n\nmxaddsvr Abort!!\n\n");
        free(tmpstr);
        free(tmp1);
        free(tmp2);
        free(tmptty);
        free(tmpcout);
        return 0;
    }

    if (overwrite == -2)
    {
        printf("\n");
        free(tmpstr);
        free(tmp1);
        free(tmp2);
        free(tmptty);
        free(tmpcout);
        return 0;
    }

    sprintf(tmpstr, "cp -f /tmp/npr_tmpfile2 %s/npreal2d.cf", DRIVERPATH);
    system(tmpstr);
    system("rm -f /tmp/npr_tmpfile2");

    sprintf(tmpstr, "%s/npreal2d.cf", DRIVERPATH);
    f = fopen (tmpstr, "a+");
    if (f == NULL)
    {
        printf("Opening configuration file error...\n");
        free(tmpstr);
        free(tmp1);
        free(tmp2);
        free(tmptty);
        free(tmpcout);
        return(0);
    }

    fifo = 1;
    ssl = 0;
    memset(tmpstr, 0, sizeof(tmpstr));

    for (i=0; i<total; i++)
    {
        mn = getMinor();
        getTty(tmp1);
        getCout(tmp2);
        sprintf(tmpt, "%s", tmp1);
        sprintf(tmpc, "%s", tmp2);
        printf("%s, %s\n", tmpt, tmpc);
        if(i > 15){
        	dataport = 966;
        	cmdport = 982;
        }
        sprintf (tmpstr, "%d\t%s\t%d\t%d\t%d\t%d %s\t%s\n", mn, ip, dataport+i, cmdport+i, fifo, ssl, tmpt, tmpc);
        fputs (tmpstr, f);
        sprintf(tmpstr, "%s/mxrmnod /dev/%s", DRIVERPATH, tmpt);
        system(tmpstr);
        sprintf(tmpstr, "%s/mxrmnod /dev/%s", DRIVERPATH, tmpc);
        system(tmpstr);

    }
    fclose(f);

    printf("Added server: %s\n\n", ip);
    sprintf(tmpstr, "%s/mxloadsvr", DRIVERPATH);
    system(tmpstr);
    if (os == "linux")
    {
        system("chmod +x /etc/rc.d/rc.local");
    }
    else if (os == "debian")
    {
        system("chmod +x /etc/init.d/npreals");
    }
    else if (os == "SuSE")
    {
        system("chmod +x /etc/rc.d/boot.local");
    }

    free(tmpstr);
    free(tmp1);
    free(tmp2);
    free(tmptty);
    free(tmpcout);
    return 0;
}



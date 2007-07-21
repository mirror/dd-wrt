#include "nport.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define     ER_ARG  -10

char svrList[256][50];
int total[256];
int idx;


int check_usage(int arg, char *argv[])
{
    if (arg > 2)
    {
        printf("mxdelsvr [ip]\n\n");
        return ER_ARG;
    }
    return 0;
}

char* GetIP(unsigned long ip, char *ret)
{
    struct in_addr ad;

    ad.s_addr = ip;
    sprintf(ret, "%s", inet_ntoa(ad));

    return;
}

int main(int arg, char *argv[])
{
    int i, j;
    int len, daemon;
    struct in_addr ad;
    char *tmpstr, *tmp, *os, c[5];
    char token[50], tty[20], cout[20], major[20], del[50];
    FILE *f, *ft;

    if (check_usage(arg, argv) != 0)
    {
        return 0;
    }

    system("clear");
    printf("\nDelete Server ...\n");

    idx = 0;
    daemon = 0;
    tmpstr = (char *)malloc(1024);
    len = 1024;
    tmp = (char *)malloc(20);

    if (arg == 2)
    {

        sprintf(del, "%s", argv[1]);
        sprintf(tmpstr, "%s/npreal2d.cf", DRIVERPATH);
        f = fopen (tmpstr, "r");
        if (f == NULL)
        {
            printf("file open error\n");
            free(tmpstr);
            free(tmp);
            return(0);
        }

        /* search the del server */
        for (;;)
        {
            if (getline (&tmpstr, &len, f) < 0)
            {
                break;
            }
            if (strstr(tmpstr, "#") != NULL)
            {
                continue;
            }
            memset(major, '\0', 20);
            sscanf(tmpstr, "%s", major);
            if (strstr(major, "ttymajor") != NULL ||
                    strstr(major, "calloutmajor") != NULL )
            {
                continue;
            }

            sscanf(tmpstr, "%s%s", token, token);
            if (strcmp(token, del) == 0)
            {
                idx = 1;
                break;
            }
        }
        fclose (f);

        if (idx == 0)
        {
            printf("The speicified ip is not installed.\n\n");
            free(tmpstr);
            free(tmp);
            return 0;
        }

    }
    else
    {

        memset(svrList, 0x0, 256*50);
        memset(total, 0x0, 256*sizeof(int));
        sprintf(tmpstr, "%s/mxcfmat", DRIVERPATH);
        system(tmpstr);

        sprintf(tmpstr, "%s/npreal2d.cf", DRIVERPATH);
        f = fopen (tmpstr, "r");
        if (f == NULL)
        {
            printf("file open error\n");
            free(tmpstr);
            free(tmp);
            return(0);
        }

        /* print the list of installed server */
        for (;;)
        {
            if (getline (&tmpstr, &len, f) < 0)
            {
                break;
            }
            if (strstr(tmpstr, "#") != NULL)
            {
                continue;
            }
            memset(major, '\0', 20);
            sscanf(tmpstr, "%s", major);
            if (strstr(major, "ttymajor") != NULL ||
                    strstr(major, "calloutmajor") != NULL )
            {
                continue;
            }

            sscanf(tmpstr, "%s%s", token, token);
            for (i=0; i<idx; i++)
            {
                if (!strcmp(svrList[i],token))
                {
                    total[i]++;
                    break;
                }
            }
            if (i == idx)
            {
                strcpy(svrList[idx], token);
                total[idx]++;
                idx++;
            }
        }
        fclose (f);

        if (idx == 0)
        {
            printf("No NPort server is installed.\n\n");
            free(tmpstr);
            free(tmp);
            return 0;
        }

        printf("\n[Index]\t[Server IP]\t[Port(s)]\n");
        for (i=0; i<idx; i++)
        {
//    	    ad.s_addr = svrList[i];
            printf("  (%d)\t%s\t  %d\n", i+1, svrList[i], total[i]);
        }

        printf("\nSelect: ");
        scanf("%s", &c);

        if (atoi(c)<=0 || atoi(c)>idx)
        {
            printf("Please run mxdelsvr again!!\n\n");
            free(tmpstr);
            free(tmp);
            return 0;
        }

        memset(tmp, '\0', 20);
//       GetIP(svrList[atoi(c)-1], tmp);
        strcpy(del, svrList[atoi(c)-1]);
    }

    sprintf(tmpstr, "%s/npreal2d.cf", DRIVERPATH);
    f = fopen (tmpstr, "r");
    if (f == NULL)
    {
        printf("file open error\n");
        free(tmpstr);
        free(tmp);
        return(0);
    }
    ft = fopen ("/tmp/nprtmp_cf", "w");
    if (ft == NULL)
    {
        printf("file open error\n");
        free(tmpstr);
        free(tmp);
        return(0);
    }

    /* delete all device file configured in npreal2d.cf */
    memset(tmpstr, '\0', 1024);
    sprintf(tmpstr, "awk '$0 !~ /#/' %s/npreal2d.cf |", DRIVERPATH);
    sprintf(tmpstr, "%s awk '$7 != \"\" ' |", tmpstr);
    sprintf(tmpstr, "%s awk '$8 != \"\" ' |", tmpstr);
    sprintf(tmpstr, "%s awk '{system(\"%s/mxrmnod \"$7); system(\"%s/mxrmnod \"$8)}'", tmpstr, DRIVERPATH, DRIVERPATH);
    system(tmpstr);

    /* Delete the server selected by user,  */
    /* and remove the relevant device files */
    for (;;)
    {
        if (getline (&tmpstr, &len, f) < 0)
        {
            break;
        }
        if (strstr(tmpstr, "#") != NULL)
        {
            fputs (tmpstr, ft);
            continue;
        }
        memset(major, '\0', 20);
        sscanf(tmpstr, "%s", major);
        if (strstr(major, "ttymajor") != NULL ||
                strstr(major, "calloutmajor") != NULL )
        {
            fputs (tmpstr, ft);
            continue;
        }

        sscanf(tmpstr, "%s%s", token, token);
        if (strcmp(token, del) != 0)
        {
            fputs (tmpstr, ft);

            /* daemon is a flag which is used to delete the */
            /* daemon start string in /etc/rc.d/rc.local */
            daemon = 1;

        }
    }

    fclose(ft);
    fclose (f);

    os = "linux";
    f = fopen ("/etc/redhat-release", "r");
    if (f != NULL)
    {
        fclose(f);
        os = "linux";
    }
    else
    {
        f = fopen ("/etc/SuSE-release", "r");
        if (f != NULL)
        {
            fclose(f);
            os = "SuSE";
        }
        else
        {
            f = fopen ("/etc/debian_version", "r");
            if (f != NULL)
            {
                os = "debian";
            } /* else {
                            printf("Your Operating System is NOT supported.\n\n");
                            free(tmpstr);
                            free(tmp);
                            return -1;
                        } */
        }
    }


    if (!daemon)
    {
        if (os == "linux")
        {
            system("grep -v mxloadsvr /etc/rc.d/rc.local > /tmp/nprtmp_rclocal");
            system("cp -f /tmp/nprtmp_rclocal /etc/rc.d/rc.local > /dev/null 2>&1");
            system("rm -f /tmp/nprtmp_rclocal");

        }
        else if (os == "debian")
        {
            system("grep -v mxloadsvr /etc/init.d/npreals > /tmp/nprtmp_rclocal");
            system("cp -f /tmp/nprtmp_rclocal /etc/init.d/npreals > /dev/null 2>&1");
            system("rm -f /tmp/nprtmp_rclocal");
            system("update-rc.d npreals defaults 90");

        }
        else if (os == "SuSE")
        {
            system("grep -v mxloadsvr /etc/rc.d/boot.local > /tmp/nprtmp_rclocal");
            system("cp -f /tmp/nprtmp_rclocal /etc/rc.d/boot.local > /dev/null 2>&1");
            system("rm -f /tmp/nprtmp_rclocal");

        }
    }

    sprintf(tmpstr, "cp -f /tmp/nprtmp_cf %s/npreal2d.cf", DRIVERPATH);
    system(tmpstr);
    system("rm -f /tmp/nprtmp_cf");

    printf("Deleted server: %s\n\n", del);
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
    free(tmp);
    return 0;
}



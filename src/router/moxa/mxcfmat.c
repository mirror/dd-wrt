#include "nport.h"
#include <string.h>
#include <stdlib.h>
#define     ER_ARG  -10

int main(int arg, char *argv[])
{
    int i, j, len, flag;
    char *tmpstr, *token, *minor_token, *chk;
    char delim[] = " \t";
    char major[20];
    FILE *f, *ft;


    tmpstr = (char *)malloc(256);
    len = 256;
    token = (char *)malloc(256);

    sprintf(tmpstr, "%s/npreal2d.cf", DRIVERPATH);
    f = fopen (tmpstr, "r");
    if (f == NULL)
    {
        printf("file open error1\n");
        free(tmpstr);
        free(token);
        return(0);
    }
    ft = fopen ("/tmp/npr_tmpfile3", "w");
    if (ft == NULL)
    {
        printf("file open error2\n");
        free(tmpstr);
        free(token);
        return(0);

    }

    for (;;)
    {
        flag = 0;
        memset(token, '\0', 256);
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
//            printf("number = [%d]\n", atoi(strstr(major, "=")+1));
            fputs (tmpstr, ft);
            continue;
        }


        for (i=0; i<8; i++)
        {
            if (i==0)
            {
                minor_token = strtok(tmpstr, delim);                  //  [minor]
                sprintf(token, "%s", minor_token);
            }
            else
            {
                chk = strtok(NULL, delim);
                if (chk == NULL)
                {
                    flag = 1;
                    break;
                }
                sprintf(token, "%s\t%s", token, chk);  //  [Nport IP]
            }
        }
        if (flag == 1)
        {
            continue;
        }
        fputs (token, ft);
    }

    fclose(ft);
    fclose(f);

    sprintf(tmpstr, "cp -f /tmp/npr_tmpfile3 %s/npreal2d.cf", DRIVERPATH);
    system(tmpstr);
    system("rm -f /tmp/npr_tmpfile3");

    free(tmpstr);
    free(token);
    return 0;
}



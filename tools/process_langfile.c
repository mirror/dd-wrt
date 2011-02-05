/* Language js file processing */
/* this program removes duplicate entries in not completly translated language files to reduce overall size */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

int filter (char *m, char *source, char *dest, int len);

int copyline(char *source, char *dest,int maxlen)
{
int c=0;
while(c<maxlen)
    {
    dest[c]=source[c];
    if (dest[c++]=='\n')
	break;
    }
dest[c++]=0;
//printf("returns %d\n",c);
return c-1;    
}
int
main (int argc, char *argv[])
{
    int i;
    char *m;
    int a;
    FILE *inFile;
    char linebuff[32762];

    if (argc <= 2)
        return 0;

    printf ("Language files processing...\n");
    FILE *refFile = fopen (argv[1], "rb");
    fseek(refFile,0,SEEK_END);
    int refsize = ftell(refFile);
    char *ref = malloc(refsize);
    fseek(refFile,0,SEEK_SET);
    fread(ref,1,refsize,refFile);
    fclose(refFile);
    for (a = 2; a < argc; a++)
    {
        if (!strcmp(argv[a], argv[1]))
            continue;

        printf ("Processing %s\n", argv[a]);
        inFile = fopen (argv[a], "rb");
        if (inFile == NULL)
            return 0;
        fseek (inFile, 0, SEEK_END);
        int len = ftell (inFile);
        m = (char *) malloc (len);
        fseek (inFile, 0, SEEK_SET);
        fread(m,1,len,inFile);
        fclose (inFile);
	int offset=0;
	
//        FILE *refFile = fopen (argv[1], "rb");
	int oinc;
        while ((oinc=copyline(&ref[offset],linebuff,refsize-offset)))
        {
    	    offset+=oinc;
            // some sanity checks
            if (linebuff[0]=='\r' || linebuff[0]=='\n')
                continue;
            if (!strstr(linebuff,"=\""))
                continue;
            if (!strstr(linebuff,"\";"))
                continue;

            len = filter (m, linebuff, "", len);
        }
//        fclose (refFile);

        printf ("Writing \n");

        inFile = fopen (argv[a], "wb");
        fwrite (m, len, 1, inFile);
        free (m);

        fclose (inFile);

    }
    return 0;
}


int
filter (char *m, char *source, char *dest, int len)
{
    int i;
    int slen = strlen(source);
    int dlen = strlen(dest);
    for (i = 0; i <= len - slen; i++)
    {
        if (strncmp ((char *) &m[i], source, slen) == 0)
        {
            memcpy (&m[i], dest, dlen);
            int delta = slen - dlen;
            memcpy (&m[i + dlen], &m[i + slen],
                    len - (i + slen));
            len -= delta;
            i += dlen;
        }
        int before = i;
        while(m[i++]!='\n' && i<(len - slen));
    }
    return len;
}



/* Language js file processing */
/* this program removes duplicate entries in not completly translated language files to reduce overall size */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

int filter (char *m, char *source, char *dest, int len);

int
main (int argc, char *argv[])
{
    int i;
    char *m;
    int a;
    FILE *inFile;
    FILE *refFile;
    char linebuff[32762];

    if (argc <= 2)
        return 0;

    printf ("Language files processing...\n");

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
        for (i = 0; i < len; i++)
            m[i] = getc (inFile);

        fclose (inFile);

        FILE *refFile = fopen (argv[1], "rb");
        while (fgets(linebuff, sizeof(linebuff), refFile))
        {
            // some sanity checks
            if (linebuff[0]=='\r' || linebuff[0]=='\n')
                continue;
            if (!strstr(linebuff,"=\""))
                continue;
            if (!strstr(linebuff,"\";"))
                continue;

            len = filter (m, linebuff, "", len);
        }
        fclose (refFile);

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
    for (i = 0; i < len - slen; i++)
    {
        if (strncmp ((char *) &m[i], source, slen) == 0)
        {
            memcpy (&m[i], dest, dlen);
            int delta = strlen (source) - dlen;
            memcpy (&m[i + dlen], &m[i + slen],
                    len - (i + slen));
            len -= delta;
            i += dlen;
        }
    }
    return len;
}



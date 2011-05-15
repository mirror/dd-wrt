/* nvramconfig file processing 
 * this program removes comments and optimizes spaces in .nvramconfig file to reduce overall size
 * (c) DD-WRT, Eko, 2010 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

int
main (int argc, char *argv[])
{
	int i;
	char *m, *out;
	int a;
	FILE *inFile;

	if (argc <= 1)
		return 0;

	printf ("Nvramconfig files processing...\n");

	for (a = 1; a < argc; a++)
	{
		printf ("Processing %s\n", argv[a]);

		inFile = fopen (argv[a], "rb");
		if (inFile == NULL)
			return 0;
		fseek (inFile, 0, SEEK_END);
		int len = ftell (inFile);
		m = (char *) malloc (len);
		fseek (inFile, 0, SEEK_SET);
		fread(m, 1, len, inFile);
		fclose (inFile);

		out = (char *) malloc (len);
		int count = 0;
		int sp_count = 0;
		int nlen = 0;


		for (i = 0; i < len; i++) {

			if (m[i] == '\n') {
				out[nlen++] = '\n';
				count = 0;
				sp_count = 0;
				continue;
			}

			if (m[i] == '\"') {
				if (count && (count % 2 == 0))
					sp_count = 0;
				count++;
				out[nlen++] = '\"';
				continue;
			}
	
			if (m[i] == ' ' && count && (count % 2 == 0)) {
				if (sp_count == 0)
					out[nlen++] = ' ';	
				sp_count++;
				continue;
			}		 	

			if (count == 3)
				continue;
			
			out[nlen++] = m[i];

		}

		printf ("Writing \n");

		inFile = fopen (argv[a], "wb");
		fwrite (out, nlen, 1, inFile);
		free (out);

		fclose (inFile);




	}
	return 0;
}


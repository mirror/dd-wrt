#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>


int main(void)
{
    int error,i,j,u;

    error = 0;
    for (i = 0; fields[i]; i++)
	for (j = i+1; fields[j]; j++)
	    if (!strcmp(fields[i],fields[j])) {
		fprintf(stderr,"collision: field \"%s\"\n",fields[i]);
		error = 1;
	    }
    for (i = 0; groups[i]; i++)
	for (j = i+1; groups[j]; j++)
	    if (!strcmp(groups[i],groups[j])) {
		fprintf(stderr,"collision: group \"%s\"\n",groups[i]);
		error = 1;
	    }
    for (u = 0; unique[u]; u++)
	for (i = 0; unique[u][i] != -1; i++)
	    for (j = i+1; unique[u][j] != -1; j++)
		if (unique[u][i] == unique[u][j]) {
		    fprintf(stderr,"collision: value %d in collection %d\n",
		      unique[u][i],u);
		    error = 1;
		}
    return error;
}

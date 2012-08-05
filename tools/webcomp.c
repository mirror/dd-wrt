/*
 * webcomp -- Compile web pages into C source
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: webcomp.c,v 1.4 2003/04/11 14:46:55 joel Exp $
 */

/******************************** Description *********************************/

/*
 *	Usage: webcomp prefix filelist >webrom.c
 *
 *	filelist is a file containing the pathnames of all web pages
 *	prefix is a path prefix to remove from all the web page pathnames
 *	webrom.c is the resulting C source file to compile and link.
 */

/********************************* Includes ***********************************/

#include <fcntl.h>
#include	"wsIntrn.h"

/**************************** Forward Declarations ****************************/

static int 	compile(char_t *fileList, char_t *prefix, FILE *cfile,FILE *www);
static void usage();

/*********************************** Code *************************************/
/*
 *	Main program for webpack test harness
 */

int gmain(int argc, char_t* argv[])
{
	char_t		*fileList, *prefix;

	fileList = NULL;

	if (argc < 3) {
		usage();
	}

	prefix = argv[1];
	fileList = argv[2];
	FILE *out=fopen(fileList,"wb");
	int i;
	for (i=3;i<argc-2;i++)
	    {
	    FILE *t=fopen(argv[i],"rb");
	    if (t==NULL)continue;
	    fclose(t);
	    fprintf(out,"%s\n",argv[i]);
	    }
	fclose(out);
	FILE *out1 = fopen(argv[argc-2],"wb");
	FILE *out2 = fopen(argv[argc-1],"wb");
	if (compile(fileList, prefix, out1,out2) < 0) {
		return -1;
	}
	fclose(out1);
	fclose(out2);
	return 0;
}

/******************************************************************************/
/*
 *	Output usage message
 */

static void usage()
{
	fprintf(stderr, "usage: webcomp prefix filelist output.c pages\n");
	exit(2);
}

/******************************************************************************/
/*
 *	Compile the web pages
 */

static int compile(char_t *fileList, char_t *prefix, FILE *stdout2, FILE *www)
{
	gstat_t			sbuf;
	gstat_t			csbuf;
	FILE			*lp;
	time_t			now;
	char_t			file[1024];
	char_t			*cp, *sl;
	char			buf[512];
	unsigned char	*p;
	int				j, i, len, fd, nFile;

/*
 *	Open list of files
 */
	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}

	time(&now);
	fprintf(stdout2, "/*\n * webrom.c -- Compiled Web Pages\n *\n");
	fprintf(stdout2, " * Compiled by GoAhead WebCompile: %s */\n\n", 
		gctime(&now));
//	fprintf(stdout, "#include \"wsIntrn.h\"\n\n");

	fprintf(stdout2, "#ifndef WEBS_PAGE_ROM\n");
	fprintf(stdout2, "websRomPageIndexType websRomPageIndex[] = {\n");
	fprintf(stdout2, "    { 0, 0 },\n};\n");
	fprintf(stdout2, "#else\n");

/*
 *	Open each input file and compile each web page
 */
	nFile = 0;
	fprintf(stdout2, "static unsigned char pages[] = {\n");
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
		if (gstat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		}
		//char cfile[1024];
		//strcpy(cfile,file);
		//strcat(cfile,".lzma");
		//char exec[1024];
		//sprintf(exec,"lzma e %s %s -fb273 -lc2 -lp0 -pb0",file,cfile);
		//fprintf(stderr,"exec %s\n",exec);
		//system(exec); 
		
		if ((fd = gopen(file, O_RDONLY | O_BINARY)) < 0) {
			fprintf(stderr, "Can't open file %s\n", file);
			return -1;
		}
		
		while ((len = read(fd, buf, sizeof(buf))) > 0) {
			p = buf;
			for (i = 0; i < len; ) {
				for (j = 0; p < &buf[len] && j < 16; j++, p++) {
					putc(*p,www);
				}
				i += j;
			}
		}

		close(fd);
		nFile++;
	}
	fprintf(stdout2, "    0 };\n\n");
	fclose(lp);

/*
 *	Now output the page index
 */
	fprintf(stdout2, "const websRomPageIndexType websRomPageIndex[] = {\n");

	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}
	nFile = 0;
	long offset=0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
/*
 *		Remove the prefix and add a leading "/" when we print the path
 */
		if (strncmp(file, prefix, gstrlen(prefix)) == 0) {
			cp = &file[gstrlen(prefix)];
		} else {
			cp = file;
		}
		while((sl = strchr(file, '\\')) != NULL) {
			*sl = '/';
		}
		if (*cp == '/') {
			cp++;
		}
/*		char_t *ncp = cp;
	        for (i=0;i<strlen(cp);i++)
		    {
		    if (cp[i]=='/')ncp=&cp[i+1];
		    } 
		cp=ncp;
*/
		if (gstat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		}
		/*char cfile[1024];
		strcpy(cfile,file);
		strcat(cfile,".lzma");
		if (gstat(cfile, &csbuf) == 0 && csbuf.st_mode & S_IFDIR) {
			continue;
		}*/
		
		fprintf(stdout2, "    { \"%s\", %d },\n", cp, 
			sbuf.st_size);
		offset+=sbuf.st_size;
		nFile++;
	}
	fclose(lp); 
	
	fprintf(stdout2, "    { 0, 0 },\n");
	fprintf(stdout2, "};\n");
	fprintf(stdout2, "#endif /* WEBS_PAGE_ROM */\n");

//	fclose(lp);
	fflush(stdout2);
	fflush(www);
	exit(0);
	return 0;
}

/******************************************************************************/

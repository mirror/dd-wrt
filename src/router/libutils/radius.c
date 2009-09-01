#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>

#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <bcmdevs.h>
#include <net/route.h>
#include <cy_conf.h>
#include <bcmdevs.h>
#include <linux/if_ether.h>
// #include <linux/mii.h>
#include <linux/sockios.h>
#include <cymac.h>
#include <broadcom.h>
#include <radiusdb.h>

static unsigned int readword(FILE * in)
{
	unsigned int value;
	value = (unsigned int)(getc(in)&0xff) << 24;
	value |= (unsigned int)(getc(in)&0xff) << 16;
	value |= (unsigned int)(getc(in)&0xff) << 8;
	value |= (unsigned int)(getc(in)&0xff);
	return value;
}

static void writeword(unsigned int value, FILE * out)
{
	putc((value >> 24) & 0xff, out);
	putc((value >> 16) & 0xff, out);
	putc((value >> 8) & 0xff, out);
	putc(value & 0xff, out);
}

struct radiusdb *loadradiusdb(void)
{
fprintf(stderr,"%s:%d\n",__func__,__LINE__);
	FILE *fp = fopen("/jffs/etc/freeradius/dd-wrtusers.db", "rb");
	if (fp == NULL)
		return NULL;
	struct radiusdb *db;
	if (feof(fp))
	    return NULL;
	db = malloc(sizeof(struct radiusdb));
	db->usercount = readword(fp);
	if (db->usercount)
	    db->users = malloc(db->usercount * sizeof(struct radiususer));
	else
	    db->users = NULL;
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		db->users[i].fieldlen = readword(fp);
		db->users[i].usersize = readword(fp);
		if (db->users[i].usersize)
		{
		db->users[i].user = malloc(db->users[i].usersize);
		fread(db->users[i].user, db->users[i].usersize, 1, fp);
		}else
		db->users[i].user = NULL;
		
		db->users[i].passwordsize = readword(fp);
		if (db->users[i].passwordsize)
		{
		db->users[i].passwd = malloc(db->users[i].passwordsize);
		fread(db->users[i].passwd, db->users[i].passwordsize, 1, fp);
		}else
		    db->users[i].passwd=NULL;
		
		db->users[i].downstream = readword(fp);
		db->users[i].upstream = readword(fp);
	}
	fclose(fp);
fprintf(stderr,"%s:%d\n",__func__,__LINE__);
	return db;
}

void writeradiusdb(struct radiusdb *db)
{
fprintf(stderr,"%s:%d\n",__func__,__LINE__);
	FILE *fp = fopen("/jffs/etc/freeradius/dd-wrtusers.db", "wb");
	if (fp == NULL)
		return;
	writeword(db->usercount, fp);
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		if (db->users[i].user)
		    db->users[i].usersize = strlen(db->users[i].user)+1;
		else
		    db->users[i].usersize = 0;

		if (db->users[i].passwd)
		    db->users[i].passwordsize = strlen(db->users[i].passwd)+1;
		else
		    db->users[i].passwordsize = 0;

		db->users[i].fieldlen=sizeof(struct radiususer)+db->users[i].usersize+db->users[i].passwordsize - 8;

		writeword(db->users[i].fieldlen, fp);
		writeword(db->users[i].usersize, fp);
		if (db->users[i].usersize)
		    fwrite(db->users[i].user, db->users[i].usersize, 1, fp);
		writeword(db->users[i].passwordsize, fp);
		if (db->users[i].passwordsize)
		    fwrite(db->users[i].passwd, db->users[i].passwordsize, 1, fp);
		writeword(db->users[i].downstream, fp);
		writeword(db->users[i].upstream, fp);
	}
fprintf(stderr,"%s:%d\n",__func__,__LINE__);
	fclose(fp);
}

void freeradiusdb(struct radiusdb *db)
{
fprintf(stderr,"%s:%d\n",__func__,__LINE__);
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		if (db->users[i].passwd && db->users[i].passwordsize)
		    free(db->users[i].passwd);
		if (db->users[i].user && db->users[i].usersize)
		    free(db->users[i].user);
	}
	if (db->users)
	    free(db->users);
	free(db);
fprintf(stderr,"%s:%d\n",__func__,__LINE__);
}

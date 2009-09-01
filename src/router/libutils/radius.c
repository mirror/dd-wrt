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
	value = getc(in) << 24;
	value |= getc(in) << 16;
	value |= getc(in) << 8;
	value |= getc(in);
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
	FILE *fp = fopen("/jffs/etc/freeradius/dd-wrtusers.db", "rb");
	if (fp == NULL)
		return NULL;
	struct radiusdb *db;
	db = malloc(sizeof(struct radiusdb));
	db->usercount = readword(fp);
	db->users = malloc(db->usercount * sizeof(struct radiususer));
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		db->users[i].fieldlen = readword(fp);
		db->users[i].usersize = readword(fp);
		db->users[i].user = malloc(db->users[i].usersize);
		fread(db->users[i].user, db->users[i].usersize, 1, fp);
		db->users[i].passwordsize = readword(fp);
		fread(db->users[i].passwd, db->users[i].passwordsize, 1, fp);
		db->users[i].downstream = readword(fp);
		db->users[i].upstream = readword(fp);
	}
	fclose(fp);
	return db;
}

void writeradiusdb(struct radiusdb *db)
{
	FILE *fp = fopen("/jffs/etc/freeradius/dd-wrtusers.db", "wb");
	if (fp == NULL)
		return;
	writeword(db->usercount, fp);
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		writeword(db->users[i].fieldlen, fp);
		writeword(db->users[i].usersize, fp);
		fwrite(db->users[i].user, db->users[i].usersize, 1, fp);
		writeword(db->users[i].passwordsize, fp);
		fwrite(db->users[i].passwd, db->users[i].passwordsize, 1, fp);
		writeword(db->users[i].downstream, fp);
		writeword(db->users[i].upstream, fp);
	}
}

void freeradiusdb(struct radiusdb *db)
{
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		free(db->users[i].passwd);
		free(db->users[i].user);
	}
	free(db->users);
	free(db);
}

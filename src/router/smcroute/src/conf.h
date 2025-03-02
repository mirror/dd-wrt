#ifndef SMCROUTE_CONF_H_
#define SMCROUTE_CONF_H_

#include "config.h"

#define EMPTY   0
#define MGROUP  1
#define MROUTE  2
#define PHYINT  3
#define INCLUDE 4

struct conf {
	const char   *file;
	unsigned int  lineno;
};

extern int conf_vrfy;

int conf_mgroup (struct conf *conf, int cmd, char *iif, char *source, char *group);
int conf_mroute (struct conf *conf, int cmd, char *iif, char *source, char *group, char *oif[], int num);
int conf_parse  (struct conf *conf, int do_vifs);

int conf_read   (char *file, int do_vifs);

#endif /* SMCROUTE_CONF_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */

#ifndef UEVENT_H
#define UEVENT_H 1

#define ACTION_ADD       0
#define ACTION_REMOVE    1
#define ACTION_UNKNOWN (-1)

#include <errno.h>
#include <libgen.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "seqnum.h"
#include "xmemutils.h"

struct env_var_t {
	char *key;
	char *value;
};

struct uevent_t {
	int action;
	event_seqnum_t seqnum;

	char *action_str;
	struct env_var_t *env_vars;
	int env_vars_c;

	char *plain;
	uint32_t plain_s;
};

void uevent_free(struct uevent_t *);
char *uevent_getvalue(const struct uevent_t *, const char *);
int uevent_add_env(struct uevent_t *, const char *);
struct uevent_t *uevent_dup(const struct uevent_t *);
struct uevent_t *uevent_deserialize(char *, int);

#endif /* ifndef UEVENT_H */


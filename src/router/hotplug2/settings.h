#ifndef SETTINGS_H
#define SETTINGS_H 1

/* Forward declaration for worker */
struct settings_t;

#include "seqnum.h"
#include "rules/ruleset.h"
#include "workers/loader.h"

struct settings_t {

/* Highest processed seqnum */
event_seqnum_t      highest_seqnum;

/* Grace time */
int                 grace;

/* We run in persitent mode or we quit once we've processed everything. */
int                 persistent;

/* Do we use flag system associated with the rules? */
int                 useflags;

/* Do we even use rules? */
int                 dumb;

/* File descriptor of the netlink socket. */
int                 netlink_socket;

/* Modprobe command to be used. If NULL, attempt autodetection.*/
char                *modprobe_command;

/* Coldplug command to be executed. If none, NULL. */
char                *coldplug_command;

/* Path to rules file. */
char                *rules_file;

/* List of rules to be used. */
struct ruleset_t    rules;

/* Worker name to be loaded */
char                *worker_name;

/* Worker loader */
struct loader_ctx_t *worker;

/* All the unprocessed arguments go here. */
char                **worker_argv;
int                 worker_argc;
};

void settings_init(struct settings_t *);
void settings_clear(struct settings_t *);

#endif /* SETTINGS_H */

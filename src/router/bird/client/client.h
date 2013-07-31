/*
 *	BIRD Client
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */


extern int init, busy, interactive;
extern int term_lns, term_cls;

/* birdc.c / birdcl.c */

void input_start_list(void);
void input_stop_list(void);

void input_init(void);
void input_notify(int prompt);
void input_read(void);

void more_begin(void);
void more_end(void);

void cleanup(void);

/* commands.c */

void cmd_build_tree(void);
void cmd_help(char *cmd, int len);
int cmd_complete(char *cmd, int len, char *buf, int again);
char *cmd_expand(char *cmd);

/* client.c */

void submit_command(char *cmd_raw);

/*
 *	BIRD Client
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/* client.c */

void cleanup(void);
void input_start_list(void);
void input_stop_list(void);

/* commands.c */

void cmd_build_tree(void);
void cmd_help(char *cmd, int len);
int cmd_complete(char *cmd, int len, char *buf, int again);
char *cmd_expand(char *cmd);

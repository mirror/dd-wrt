/***

log.h - the iptraf logging facility header file
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997

***/

char *gen_instance_logname(char *template, int instance_id);
void input_logfile(char *target, int *aborted);
void opentlog(FILE ** fd, char *logfilename);
void writelog(int logging, FILE * fd, char *msg);
void write_daemon_err(char *msg);
void rotate_logfile(FILE ** fd, char *name);
void check_rotate_flag(FILE ** fd, int logging);
void announce_rotate_prepare(FILE * fd);
void announce_rotate_complete(FILE * fd);

#ifndef IPTRAF_NG_LOG_H
#define IPTRAF_NG_LOG_H

/***

log.h - the iptraf logging facility header file

***/

#define TIME_TARGET_MAX		30

char *gen_instance_logname(char *template, int instance_id);
void input_logfile(char *target, int *aborted);
void opentlog(FILE ** fd, char *logfilename);
void writelog(int logging, FILE * fd, char *msg);
void genatime(time_t now, char *atime);
void write_daemon_err(char *msg, va_list vararg);
void rotate_logfile(FILE ** fd, char *name);
void check_rotate_flag(FILE ** fd);
void announce_rotate_prepare(FILE * fd);
void announce_rotate_complete(FILE * fd);

#endif	/* IPTRAF_NG_LOG_H */

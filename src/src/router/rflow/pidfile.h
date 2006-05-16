#ifndef	__PIDFILE_INT_H__
#define	__PIDFILE_INT_H__

int write_pid_file(int pf, pid_t pid, char *);
int make_pid_file(char *pidfile);

#endif	/* __PIDFILE_INT_H__ */

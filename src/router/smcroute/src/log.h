/* System logging API */
#ifndef SMCROUTE_LOG_H_
#define SMCROUTE_LOG_H_

#include <syslog.h>

extern int  log_level;
extern char log_message[128];

int loglvl(const char *level);
void smclog(int severity, const char *fmt, ...);

#endif /* SMCROUTE_LOG_H_ */

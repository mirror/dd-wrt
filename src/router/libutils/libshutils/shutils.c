/*
 * Shell-like utility functions
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: shutils.c,v 1.2 2005/11/11 09:26:18 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#ifdef __UCLIBC__
#include <error.h>
#endif
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <syscall.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <dirent.h>
#include <sys/mman.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || (defined(HAVE_RB600) && !defined(HAVE_WDR4900)) //special treatment

int debug_ready(void)
{
	if (!f_exists("/tmp/.nvram_done"))
		return 0;
	return 1;
}
#endif

static int console_debug(void)
{
	return debug_ready() && nvram_matchi("console_debug", 1);
}

/*
 * Reads file and returns contents
 * @param       fd      file descriptor
 * @return      contents of file or NULL if an error occurred
 */
char *fd2str(int fd)
{
	char *buf = NULL;
	size_t count = 0, n;

	do {
		buf = realloc(buf, count + 512);
		n = read(fd, buf + count, 512);
		if (n < 0) {
			free(buf);
			buf = NULL;
		}
		count += n;
	} while (n == 512);

	close(fd);
	if (buf)
		buf[count] = '\0';
	return buf;
}

/*
 * Reads file and returns contents
 * @param       path    path to file
 * @return      contents of file or NULL if an error occurred
 */
char *file2str(const char *path)
{
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1) {
		perror(path);
		return NULL;
	}

	return fd2str(fd);
}

/*
 * Waits for a file descriptor to change status or unblocked signal
 * @param       fd      file descriptor
 * @param       timeout seconds to wait before timing out or 0 for no timeout
 * @return      1 if descriptor changed status or 0 if timed out or -1 on error
 */
int waitfor(int fd, int timeout)
{
	fd_set rfds;
	struct timeval tv = { timeout, 0 };

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	return select(fd + 1, &rfds, NULL, NULL, (timeout > 0) ? &tv : NULL);
}

/*
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param       argv    argument list
 * @param       path    NULL, ">output", or ">>output"
 * @param       timeout seconds to wait before timing out or 0 for no timeout
 * @param       ppid    NULL to wait for child termination or pointer to pid
 * @return      return value of executed command or errno
 */

static void flog(const char *fmt, ...)
{
	if (nvram_matchi("flog_enabled", 1)) {
		char varbuf[256];
		va_list args;

		va_start(args, (char *)fmt);
		vsnprintf(varbuf, sizeof(varbuf), fmt, args);
		va_end(args);
		FILE *fp = fopen("/tmp/syslog.log", "ab");
		fprintf(fp, varbuf);
		fclose(fp);
	}
}

#undef system // prevent circular dependency

int dd_system(const char *command)
{
	if (debug_ready()) {
		dd_debug(DEBUG_CONSOLE, "%s:%s\n", __func__, command);
		if (nvram_match("debug_delay", "1")) {
			sleep(1);
		}
	}
	return system(command);
}

int sysprintf(const char *fmt, ...)
{
	char *varbuf;
	va_list args;

	va_start(args, (char *)fmt);
	vasprintf(&varbuf, fmt, args);
	va_end(args);
	int ret = dd_system(varbuf);
	free(varbuf);
	return ret;
}

void dd_debug(int target, const char *fmt, ...)
{
	char *varbuf;
	va_list args;
	if (!debug_ready())
		return;
	if (target == DEBUG_CONSOLE && !nvram_match("console_debug", "1"))
		return;
	if (target == DEBUG_HTTPD && !nvram_match("httpd_debug", "1"))
		return;
	if (target == DEBUG_SERVICE && !nvram_match("service_debug", "1"))
		return;

	va_start(args, (char *)fmt);
	vasprintf(&varbuf, fmt, args);
	va_end(args);
	dd_syslog(LOG_DEBUG, varbuf);
	fprintf(stderr, varbuf);
	free(varbuf);
	return;
}

#ifdef HAVE_SYSLOG
void dd_loginfo(const char *servicename, const char *fmt, ...)
{
	char *str;
	va_list args;
	va_start(args, (char *)fmt);
	vasprintf(&str, fmt, args);
	va_end(args);
	fprintf(stdout, "[%s] : %s", servicename, str);
	dd_syslog(LOG_INFO, "[%s] : %s", servicename, str);
	free(str);
}

void dd_logdebug(const char *servicename, const char *fmt, ...)
{
	char *str;
	int service = DEBUG_SERVICE;
	va_list args;
	va_start(args, (char *)fmt);
	vasprintf(&str, fmt, args);
	va_end(args);
	if (!strcmp(servicename, "httpd"))
		service = DEBUG_HTTPD;

#ifdef SYS_gettid
	unsigned int thread = syscall(SYS_gettid);
	dd_debug(service, "[%s][%d] : %s", servicename, thread, str);
#else
	dd_debug(service, "[%s] : %s", servicename, str);
#endif
	free(str);
}

void dd_logerror(const char *servicename, const char *fmt, ...)
{
	char *str;
	va_list args;
	va_start(args, (char *)fmt);
	vasprintf(&str, fmt, args);
	va_end(args);
	fprintf(stderr, "[%s] : %s", servicename, str);
	dd_syslog(LOG_ERR, "[%s] : %s", servicename, str);
	free(str);
}

void dd_logstart(const char *servicename, int retcode)
{
	if (retcode)
		dd_loginfo(servicename, "Error on startup, returncode %d", retcode);
	else
		dd_loginfo(servicename, "successfully started");
}
#endif

static int internal_eval_va(int silence, int space, const char *cmd, va_list args)
{
	const char *s_args[128];
	int i = 1;
	s_args[0] = cmd;
	while (1) {
		const char *arg = va_arg(args, const char *);
		char word[128];
		char *next;
		if (!arg || !space)
			s_args[i++] = arg;
		else {
			char *c = strdup(arg);
			foreach(word, c, next)
			{
				s_args[i++] = strdup(word);
			}
			free(c);
		}
		if (!arg)
			break;
	}
	int ret = _evalpid((char *const *)s_args, silence ? NULL : ">/dev/console", 0, NULL);
	i = 1;
	while (1) {
		if (!space || !s_args[i])
			break;
		free((void *)s_args[i++]);
	}
	return ret;
}

int eval_va(const char *cmd, ...)
{
	va_list args;
	va_start(args, (char *)cmd);
	int ret = internal_eval_va(0, 0, cmd, args);
	va_end(args);
	return ret;
}

int log_eval_va(const char *cmd, ...)
{
	va_list args;
	va_start(args, (char *)cmd);
	int ret = internal_eval_va(0, 0, cmd, args);
	va_end(args);
	dd_logstart(cmd, ret);
	return ret;
}

int eval_va_silence(const char *cmd, ...)
{
	va_list args;
	va_start(args, (char *)cmd);
	int ret = internal_eval_va(1, 0, cmd, args);
	va_end(args);
	return ret;
}

int eval_va_space(const char *cmd, ...)
{
	va_list args;
	va_start(args, (char *)cmd);
	int ret = internal_eval_va(0, 1, cmd, args);
	va_end(args);
	return ret;
}

int eval_va_silence_space(const char *cmd, ...)
{
	va_list args;
	va_start(args, (char *)cmd);
	int ret = internal_eval_va(1, 1, cmd, args);
	va_end(args);
	return ret;
}

int _log_evalpid(char *const argv[], char *path, int timeout, int *ppid)
{
	int ret = _evalpid(argv, path, timeout, ppid);
	dd_logstart(argv[0], ret);
	return ret;
}

// FILE *debugfp=NULL;
int _evalpid(char *const argv[], char *path, int timeout, int *ppid)
{
	pid_t pid;
	int status;
	int fd;
	int flags;
	int sig;

	// if (debugfp==NULL)
	// debugfp = fopen("/tmp/evallog.log","wb");
	// char buf[254] = "";

	if (console_debug()) {
		int i = 0;
		char buf[256] = { 0 };
		if (argv[i])
			while (argv[i] != NULL) {
				fprintf(stderr, "%s ", argv[i]);
				dd_snprintf(buf, sizeof(buf), "%s%s ", buf, argv[i++]);
				flog("%s ", argv[i - 1]);
			}
		dd_syslog(LOG_INFO, "%s:%s", __func__, buf);
		fprintf(stderr, "\n");
		flog("\n");
		if (nvram_match("debug_delay", "1")) {
			sleep(1);
		}
	}
#ifndef HAVE_SILENCE
	int i = 0;

	while (argv[i] != NULL) {
		fprintf(stderr, "%s ", argv[i++]);
	}
	fprintf(stderr, "\n");

#endif

	switch (pid = fork()) {
	case -1: /* error */
		perror("fork");
		return errno;
	case 0: /* child */
		/*
		 * Reset signal handlers set for parent process 
		 */
		for (sig = 0; sig < (_NSIG - 1); sig++)
			signal(sig, SIG_DFL);

		/*
		 * Clean up 
		 */
		ioctl(0, TIOCNOTTY, 0);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		setsid();

		/*
		 * We want to check the board if exist UART? , add by honor
		 * 2003-12-04 
		 */
		if ((fd = open("/dev/console", O_RDWR)) < 0) {
			(void)open("/dev/null", O_RDONLY);
			(void)open("/dev/null", O_WRONLY);
			(void)open("/dev/null", O_WRONLY);
		} else {
			close(fd);
			(void)open("/dev/console", O_RDONLY);
			(void)open("/dev/console", O_WRONLY);
			(void)open("/dev/console", O_WRONLY);
		}

		/*
		 * Redirect stdout to <path> 
		 */
		if (path) {
			flags = O_WRONLY | O_CREAT;
			if (!strncmp(path, ">>", 2)) {
				/*
				 * append to <path> 
				 */
				flags |= O_APPEND;
				path += 2;
			} else if (!strncmp(path, ">", 1)) {
				/*
				 * overwrite <path> 
				 */
				flags |= O_TRUNC;
				path += 1;
			}
			if ((fd = open(path, flags, 0644)) < 0)
				perror(path);
			else {
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
			}
		}

		/*
		 * execute command 
		 */
		setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
		alarm(timeout);
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(errno);
	default: /* parent */
		if (ppid) {
			*ppid = pid;
			return 0;
		} else {
			waitpid(pid, &status, 0);
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}

int _eval(char *const argv[])
{
	return _evalpid(argv, ">/dev/console", 0, NULL);
}

/*
 * Kills process whose PID is stored in plaintext in pidfile
 * @param       pidfile PID file
 * @return      0 on success and errno on failure
 */
int kill_pidfile(char *pidfile)
{
	FILE *fp = fopen(pidfile, "r");
	char buf[256];

	if (fp && fgets(buf, sizeof(buf), fp)) {
		pid_t pid = strtoul(buf, NULL, 0);

		fclose(fp);
		return kill(pid, SIGTERM);
	} else
		return errno;
}

int check_pid(int pid, char *name)
{
	char checkname[32];
	sprintf(checkname, "/proc/%d/cmdline", pid);
	FILE *file = fopen(checkname, "rb");
	if (file) {
		char line[64];
		fgets(line, sizeof(line), file);
		fclose(file);
		if (!strncmp(line, name, strlen(name)))
			return 1;
	}
	return 0;
}

int check_pidfromfile(char *pidfile, char *name)
{
	FILE *file = fopen(pidfile, "rb");
	if (file) {
		int p;
		fscanf(file, "%d", &p);
		fclose(file);
		return check_pid(p, name);
	}
	return 0;
}

/*
 * fread() with automatic retry on syscall interrupt
 * @param       ptr     location to store to
 * @param       size    size of each element of data
 * @param       nmemb   number of elements
 * @param       stream  file stream
 * @return      number of items successfully read
 */
int safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fread((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

/*
 * fwrite() with automatic retry on syscall interrupt
 * @param       ptr     location to read from
 * @param       size    size of each element of data
 * @param       nmemb   number of elements
 * @param       stream  file stream
 * @return      number of items successfully written
 */
int safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fwrite((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

/*
 * Convert Ethernet address string representation to binary data
 * @param       a       string in xx:xx:xx:xx:xx:xx notation
 * @param       e       binary data
 * @return      TRUE if conversion was successful and FALSE otherwise
 */
int ether_atoe(const char *a, char *e)
{
	char *c = (char *)a;
	int i = 0;

	bzero(e, ETHER_ADDR_LEN);
	for (;;) {
		e[i++] = (unsigned char)strtoul(c, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
	}
	return (i == ETHER_ADDR_LEN);
}

/*
 * Convert Ethernet address binary data to string representation
 * @param       e       binary data
 * @param       a       string in xx:xx:xx:xx:xx:xx notation
 * @return      a
 */
char *ether_etoa(const char *e, char *a)
{
	char *c = a;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

/*
 * Search a string backwards for a set of characters
 * This is the reverse version of strspn()
 *
 * @param       s       string to search backwards
 * @param       accept  set of chars for which to search
 * @return      number of characters in the trailing segment of s 
 *              which consist only of characters from accept.
 */
static size_t sh_strrspn(const char *s, const char *accept)
{
	const char *p;
	size_t accept_len = strlen(accept);
	int i;

	if (s[0] == '\0')
		return 0;

	p = s + (strlen(s) - 1);
	i = 0;

	do {
		if (memchr(accept, *p, accept_len) == NULL)
			break;
		p--;
		i++;
	} while (p != s);

	return i;
}

int get_ifname_unit(const char *ifname, int *unit, int *subunit)
{
	const char digits[] = "0123456789";
	char str[64];
	char *p;
	size_t ifname_len = strlen(ifname);
	size_t len;
	long val;

	if (unit)
		*unit = -1;
	if (subunit)
		*subunit = -1;

	if (ifname_len + 1 > sizeof(str))
		return -1;

	strcpy(str, ifname);

	/*
	 * find the trailing digit chars 
	 */
	len = sh_strrspn(str, digits);

	/*
	 * fail if there were no trailing digits 
	 */
	if (len == 0)
		return -1;

	/*
	 * point to the beginning of the last integer and convert 
	 */
	p = str + (ifname_len - len);
	val = strtol(p, NULL, 10);

	/*
	 * if we are at the beginning of the string, or the previous character is 
	 * not a '.', then we have the unit number and we are done parsing 
	 */
	if (p == str || p[-1] != '.') {
		if (unit)
			*unit = val;
		return 0;
	} else {
		if (subunit)
			*subunit = val;
	}

	/*
	 * chop off the '.NNN' and get the unit number 
	 */
	p--;
	p[0] = '\0';

	/*
	 * find the trailing digit chars 
	 */
	len = sh_strrspn(str, digits);

	/*
	 * fail if there were no trailing digits 
	 */
	if (len == 0)
		return -1;

	/*
	 * point to the beginning of the last integer and convert 
	 */
	p = p - len;
	val = strtol(p, NULL, 10);

	/*
	 * save the unit number 
	 */
	if (unit)
		*unit = val;

	return 0;
}

#define WLMBSS_DEV_NAME "wlmbss"
#define WL_DEV_NAME "wl"
#define WDS_DEV_NAME "wds"

#if defined(linux)

/**
	 nvifname_to_osifname() 
	 The intent here is to provide a conversion between the OS interface name
	 and the device name that we keep in NVRAM.  
	 This should eventually be placed in a Linux specific file with other 
	 OS abstraction functions.

	 @param nvifname pointer to ifname to be converted
	 @param osifname_buf storage for the converted osifname
	 @param osifname_buf_len length of storage for osifname_buf
*/
int nvifname_to_osifname(const char *nvifname, char *osifname_buf, int osifname_buf_len)
{
	char varname[NVRAM_MAX_PARAM_LEN];
	char *ptr;

	bzero(osifname_buf, osifname_buf_len);

	/*
	 * Bail if we get a NULL or empty string 
	 */
	if ((!nvifname) || (!*nvifname) || (!osifname_buf)) {
		return -1;
	}

	if (strstr(nvifname, "eth") || strstr(nvifname, ".")) {
		strncpy(osifname_buf, nvifname, osifname_buf_len);
		return 0;
	}

	snprintf(varname, sizeof(varname), "%s_ifname", nvifname);
	ptr = nvram_safe_get(varname);
	if (*ptr) {
		/*
		 * Bail if the string is empty 
		 */
		strncpy(osifname_buf, ptr, osifname_buf_len);
		return 0;
	}

	return -1;
}

/*
 * osifname_to_nvifname()
 * 
 * Convert the OS interface name to the name we use
 * internally(NVRAM,GUI,etc.)
 * 
 * This is the Linux version of this function
 * 
 * @param osifname pointer to osifname to be converted @param nvifname_buf
 * storage for the converted ifname @param nvifname_buf_len length of storage 
 * for nvifname_buf 
 */

int osifname_to_nvifname(const char *osifname, char *nvifname_buf, int nvifname_buf_len)
{
	char varname[NVRAM_MAX_PARAM_LEN];
	int pri, sec;

	/*
	 * Bail if we get a NULL or empty string 
	 */

	if ((!osifname) || (!*osifname) || (!nvifname_buf)) {
		return -1;
	}

	bzero(nvifname_buf, nvifname_buf_len);

	if (strstr(osifname, "wl")) {
		strncpy(nvifname_buf, osifname, nvifname_buf_len);
		return 0;
	}

	/*
	 * look for interface name on the primary interfaces first 
	 */
	for (pri = 0; pri < MAX_NVPARSE; pri++) {
		snprintf(varname, sizeof(varname), "wl%d_ifname", pri);
		if (nvram_match(varname, (char *)osifname)) {
			snprintf(nvifname_buf, nvifname_buf_len, "wl%d", pri);
			return 0;
		}
	}

	/*
	 * look for interface name on the multi-instance interfaces 
	 */
	for (pri = 0; pri < MAX_NVPARSE; pri++)
		for (sec = 0; sec < MAX_NVPARSE; sec++) {
			snprintf(varname, sizeof(varname), "wl%d.%d_ifname", pri, sec);
			if (nvram_match(varname, (char *)osifname)) {
				snprintf(nvifname_buf, nvifname_buf_len, "wl%d.%d", pri, sec);
				return 0;
			}
		}

	return -1;
}

#endif

int strhas(char *list, char *value)
{
	char *next, word[32];

	if (!list)
		return 0;
	foreach(word, list, next)
	{
		if (!strcmp(word, value))
			return 1;
	}
	return 0;
}

int isListed(char *listname, char *value)
{
	return strhas(nvram_safe_get(listname), value);
}

void addList(char *listname, char *value)
{
	int listlen = 0;

	if (isListed(listname, value))
		return;
	char *list = nvram_safe_get(listname);
	char *newlist;

	listlen = strlen(list);
	newlist = safe_malloc(strlen(value) + listlen + 2);
	if (!newlist)
		return;
	if (*list) {
		sprintf(newlist, "%s %s", list, value);
	} else {
		strcpy(newlist, value);
	}
	nvram_set(listname, newlist);
	free(newlist);
}

size_t strlcpy_compat(register char *dst, register const char *src, size_t n)
{
	const char *src0 = src;
	char dummy[1];

	if (!n) {
		dst = dummy;
	} else {
		--n;
	}

	while ((*dst = *src) != 0) {
		if (n) {
			--n;
			++dst;
		}
		++src;
	}

	return src - src0;
}

int f_read_string(const char *path, char *buffer, int max);

char *psname(int pid, char *buffer, int maxlen)
{
	char buf[512];
	char path[64];
	char *p;

	if (maxlen <= 0)
		return NULL;
	*buffer = 0;
	sprintf(path, "/proc/%d/stat", pid);
	if ((f_read_string(path, buf, sizeof(buf)) > 4) && ((p = strrchr(buf, ')')) != NULL)) {
		*p = 0;
		if (((p = strchr(buf, '(')) != NULL) && (atoi(buf) == pid)) {
			strlcpy_compat(buffer, p + 1, maxlen);
		}
	}
	return buffer;
}

int f_exists(const char *path) // note: anything but a directory
{
	struct stat st;
	bzero(&st, sizeof(struct stat));

	return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode));
}

int f_read(const char *path, void *buffer, int max)
{
	int f;
	int n;

	if ((f = open(path, O_RDONLY)) < 0)
		return -1;
	n = read(f, buffer, max);
	close(f);
	return n;
}

int f_read_string(const char *path, char *buffer, int max)
{
	if (max <= 0)
		return -1;
	int n = f_read(path, buffer, max - 1);

	buffer[(n > 0) ? n : 0] = 0;
	return n;
}

int wait_file_exists(const char *name, int max, int invert)
{
	while (max-- > 0) {
		if (f_exists(name) ^ invert)
			return 1;
		sleep(1);
	}
	return 0;
}

int check_action(void)
{
	char buf[80] = "";

	if (file_to_buf(ACTION_FILE, buf, sizeof(buf))) {
		if (!strcmp(buf, "ACT_WEB_UPGRADE")) {
			fprintf(stderr, "Upgrading from web (http) now ...\n");
			return ACT_WEB_UPGRADE;
		}
#ifdef HAVE_HTTPS
		else if (!strcmp(buf, "ACT_WEBS_UPGRADE")) {
			fprintf(stderr, "Upgrading from web (https) now ...\n");
			return ACT_WEBS_UPGRADE;
		}
#endif
		else if (!strcmp(buf, "ACT_SW_RESTORE")) {
			fprintf(stderr, "Receiving restore command from web ...\n");
			return ACT_SW_RESTORE;
		} else if (!strcmp(buf, "ACT_HW_RESTORE")) {
			fprintf(stderr, "Receiving restore command from resetbutton ...\n");
			return ACT_HW_RESTORE;
		} else if (!strcmp(buf, "ACT_NVRAM_COMMIT")) {
			fprintf(stderr, "Committing nvram now ...\n");
			return ACT_NVRAM_COMMIT;
		} else if (!strcmp(buf, "ACT_ERASE_NVRAM")) {
			fprintf(stderr, "Erasing nvram now ...\n");
			return ACT_ERASE_NVRAM;
		}
	}
	// fprintf(stderr, "Waiting for upgrading....\n");
	return ACT_IDLE;
}

int file_to_buf(char *path, char *buf, int len)
{
	FILE *fp;

	bzero(buf, len);

	if ((fp = fopen(path, "r"))) {
		fgets(buf, len, fp);
		fclose(fp);
		return 1;
	}

	return 0;
}

int ishexit(char c)
{
	if (strchr("01234567890abcdefABCDEF", c) != (char *)0)
		return 1;

	return 0;
}

static int _pidof(const char *name, pid_t **pids)
{
	const char *p;
	char *e;
	DIR *dir;
	struct dirent *de;
	pid_t i;
	int count;
	char buf[256];

	count = 0;
	if (pids)
		*pids = NULL;
	if ((p = strchr(name, '/')) != NULL)
		name = p + 1;
	if ((dir = opendir("/proc")) != NULL) {
		while ((de = readdir(dir)) != NULL) {
			i = strtol(de->d_name, &e, 10);
			if (*e != 0)
				continue;
			if (strncmp(name, psname(i, buf, sizeof(buf)), 15) == 0) {
				if (pids) {
					if ((*pids = realloc(*pids, sizeof(pid_t) * (count + 1))) == NULL) {
						closedir(dir);
						return -1;
					}
					(*pids)[count++] = i;
				} else {
					closedir(dir);
					return i;
				}
			}
		}
	}
	closedir(dir);
	return count;
}

int pidof(const char *name)
{
	pid_t p;
	if (!name)
		return -1;
	if ((p = _pidof(name, NULL)) > 0) {
		return p;
	}
	return -1;
}

int killall(const char *name, int sig)
{
	pid_t *pids;
	int i;
	int r;

	if ((i = _pidof(name, &pids)) > 0) {
		r = 0;
		do {
			r |= kill(pids[--i], sig);
		} while (i > 0);
		free(pids);
		return r;
	}
	return -2;
}

#undef sprintf
#undef snprintf

int dd_snprintf(char *str, int len, const char *fmt, ...)
{
	va_list ap;
	int n;
	char *dest;
	if (len < 1)
		return 0;
	va_start(ap, fmt);
	n = vasprintf(&dest, fmt, ap);
	va_end(ap);
	strncpy(str, dest, len - 1);
	str[len - 1] = '\0';
	free(dest);
	return n;
}
#undef strcat

char *dd_strncat(char *dst, size_t len, const char *src)
{
	return ((len - 1) - strlen(dst)) > 0 ? strncat(dst, src, ((len - 1) - strlen(dst))) : dst;
}

#undef strcat_r
char *strcat_r(const char *s1, const char *s2, char *buf)
{
	strcpy(buf, s1);
	strcat(buf, s2);
	return buf;
}

char *strlcat_r(const char *s1, const char *s2, char *buf, size_t len)
{
	strncpy(buf, s1, len - 1);
	return dd_strncat(buf, len, s2);
}

u_int64_t freediskSpace(char *path)
{
	struct statfs sizefs;

	if ((statfs(path, &sizefs) != 0) || (sizefs.f_type == 0x73717368) || (sizefs.f_type == 0x74717368) ||
	    (sizefs.f_type == 0x68737174)) {
		bzero(&sizefs, sizeof(sizefs));
	}

	return (u_int64_t)sizefs.f_bsize * (u_int64_t)sizefs.f_bfree;
}

int jffs_mounted(void)
{
#if defined(HAVE_X86) || defined(HAVE_VENTANA) || defined(HAVE_RAMBUTAN) || defined(HAVE_OCTEON) || defined(HAVE_NEWPORT) || \
	(defined(HAVE_RB600) && !defined(HAVE_WDR4900))
	return 1;
#endif
	int ret = nvram_matchi("jffs_mounted", 1);
	if (!ret)
		ret = freediskSpace("/jffs") > 65536;
	return ret;
}

int getMTD(char *name)
{
	char buf[32];
	int device = -1;
	char dev[32];
	char size[32];
	char esize[32];
	char n[32];
	char line[128];
	sprintf(buf, "\"%s\"", name);
	FILE *fp = fopen("/proc/mtd", "rb");
	if (!fp)
		return -1;
	while (!feof(fp)) {
		fgets(line, 127, fp);
		if (sscanf(line, "%s %s %s %s", dev, size, esize, n) < 4)
			break;
		if (!strcmp(n, buf)) {
			if (dev[4] == ':') {
				device = dev[3] - '0';
			} else {
				device = ((dev[3] - '0') * 10) + (dev[4] - '0');
			}

			break;
		}
	}
	fclose(fp);
	return device;
}

char *strattach(char *src, char *attach, char *delimiter)
{
	if (!src || !delimiter)
		return NULL;
	if (!*src) {
		strcpy(src, attach);
	} else {
		strcat(src, delimiter);
		strcat(src, attach);
	}
}

char *strspcattach(char *src, char *attach)
{
	return strattach(src, attach, " ");
}

#undef malloc

void *dd_malloc(size_t len)
{
	return len ? malloc(len) : NULL;
}

int dd_sprintf(char *str, const char *fmt, ...)
{
	va_list ap;
	int n;
	char *dest;
	if (!str)
		return 0;
	va_start(ap, fmt);
	n = vasprintf(&dest, fmt, ap);
	va_end(ap);
	strcpy(str, dest);
	free(dest);

	return n;
}

static void strcpyto(char *dest, char *src, char *delim, size_t max)
{
	int len = strlen(src);
	char *to = strpbrk(src, delim);
	if (to)
		len = to - src;
	if (max != sizeof(long) && max < len + 1) {
		dd_logerror("internal", "foreach is used in a improper way, target word is too small");
		len = max - 1;
	}
	memcpy(dest, src, len);
	dest[len] = '\0';
}

char *chomp(char *s)
{
	char *c = (s) + strlen((s)) - 1;
	while ((c > (s)) && (*c == '\n' || *c == '\r' || *c == ' '))
		*c-- = '\0';
	return s;
}

char *foreach_first(char *foreachwordlist, char *word, char *delimiters, size_t len)
{
	char *next = &foreachwordlist[strspn(foreachwordlist, delimiters)];
	strcpyto(word, next, delimiters, len);
	next = strpbrk(next, delimiters);
	return next;
}

char *foreach_last(char *next, char *word, char *delimiters, size_t len)
{
	next = next ? &next[strspn(next, delimiters)] : "";
	strcpyto(word, next, delimiters, len);
	next = strpbrk(next, delimiters);
	return next;
}

char *getentrybyidx_d(char *buf, char *list, int idx, char *delimiters_short, char *delimiters)
{
	if (!list || !buf)
		return NULL;
	char *newlist = NULL;
	int i = 0;
	while (1) {
		if (!i)
			newlist = strpbrk(list, delimiters_short);
		else
			newlist = strpbrk(list, delimiters);
		if (!newlist) {
			newlist = list + strlen(list);
			if (newlist == list)
				return NULL;
			if (i == idx)
				break;
			else
				return NULL;
		}
		i++;
		if (i > idx)
			break;
		list = newlist + 1;
	}
	int len = newlist - list;
	if (!len)
		return NULL;
	strncpy(buf, list, len);
	buf[len] = 0;
	return buf;
}

char *getentrybyidx(char *buf, char *list, int idx)
{
	return getentrybyidx_d(buf, list, idx, "><:,", "><:-,");
}

#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600) || defined(HAVE_EROUTER) && !defined(HAVE_WDR4900)
static int rootdetect(char *fname)
{
	FILE *in = fopen(fname, "rb");

	if (in == NULL)
		return -1;
	// exist, skipping
	char buf[4];

	fread(buf, 4, 1, in);
	fclose(in);
	if (!memcmp(&buf[0], "tqsh", 4) || !memcmp(&buf[0], "hsqt", 4) || !memcmp(&buf[0], "hsqs", 4)) {
		return 1;
	}
	return 0;
}

char *getdisc(void) // works only for squashfs
{
	int i, a, n;
	int nocache = 0;
	char *ret;
#ifndef HAVE_EROUTER
	FILE *in = fopen("/usr/local/nvram/nvram.bin", "rb");
	if (in) {
		fclose(in);
#endif
		char *cache = nvram_safe_get("root_disc");
		if (cache[0]) {
			char tmp[32];
			if (!strncmp(cache, "mmcblk", 6))
				sprintf(tmp, "/dev/%sp2", cache);
			else if (!strncmp(cache, "nvme", 4))
				sprintf(tmp, "/dev/%sp2", cache);
			else
				sprintf(tmp, "/dev/%s2", cache);
			if (rootdetect(tmp) > 0) {
				return strdup(cache);
			}
		}
#ifndef HAVE_EROUTER
	} else
		nocache = 1;
#endif
	for (i = 'a'; i <= 'z'; i++) {
		char dev[64];
		sprintf(dev, "/dev/sd%c2", i);
		int detect = rootdetect(dev);
		if (detect < 0)
			continue;
		if (detect) {
			asprintf(&ret, "sd%c", i);
			if (!nocache) {
				nvram_set("root_disc", ret);
				nvram_async_commit();
			}
			return ret;
		}
	}
	for (i = 'a'; i <= 'z'; i++) {
		char dev[64];
		sprintf(dev, "/dev/hd%c2", i);
		int detect = rootdetect(dev);
		if (detect < 0)
			continue;
		if (detect) {
			asprintf(&ret, "hd%c", i);
			if (!nocache) {
				nvram_set("root_disc", ret);
				nvram_async_commit();
			}
			return ret;
		}
	}
	for (a = '1'; a <= '2'; a++) {
		for (i = '0'; i <= '9'; i++) {
			char dev[64];
			sprintf(dev, "/dev/mmcblk%cp%c", i, a);
			int detect = rootdetect(dev);
			if (detect < 0)
				continue;
			if (detect) {
				asprintf(&ret, "mmcblk%c", i);
				if (!nocache) {
					nvram_set("root_disc", ret);
					nvram_async_commit();
				}
				return ret;
			}
		}
	}
	for (a = '1'; a <= '2'; a++) {
		for (n = '1'; n <= '5'; n++) {
			for (i = '0'; i <= '9'; i++) {
				char dev[64];
				sprintf(dev, "/dev/nvme%cn%cp%c", i, n, a);
				int detect = rootdetect(dev);
				if (detect < 0)
					continue;
				if (detect) {
					asprintf(&ret, "nvme%cn%c", i, n);
					if (!nocache) {
						nvram_set("root_disc", ret);
						nvram_async_commit();
					}
					return ret;
				}
			}
		}
	}
	return NULL;
}
#endif

static void precommit(void)
{
#if defined(HAVE_WZRHPG300NH) || defined(HAVE_WHRHPGN) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_DIR825) || \
	defined(HAVE_TEW632BRP) || defined(HAVE_TG2521) || defined(HAVE_WR1043) || defined(HAVE_WRT400) ||      \
	defined(HAVE_WZRHPAG300NH) || defined(HAVE_WZRG450) || defined(HAVE_DANUBE) || defined(HAVE_WR741) ||   \
	defined(HAVE_NORTHSTAR) || defined(HAVE_DIR615I) || defined(HAVE_WDR4900) || defined(HAVE_VENTANA) ||   \
	defined(HAVE_UBNTM) || defined(DHAVE_IPQ806X) || defined(DHAVE_IPQ6018)
	eval("ledtool", "1");
#elif HAVE_LSX
	//nothing
#elif HAVE_XSCALE
	//nothing
#else
	eval("ledtool", "1");
#endif
}

int nvram_commit(void)
{
	precommit();
	return _nvram_commit();
}

int nvram_async_commit(void)
{
	precommit();
	eval("async_commit");
}

int writeproc(char *path, char *value)
{
	int fd;
	fd = open(path, O_WRONLY);
	if (fd == -1) {
		fprintf(stderr, "cannot open %s\n", path);
		return -1;
	}
	write(fd, value, strlen(value));
	close(fd);
	return 0;
}

int writeprocsysnet(char *path, char *value)
{
	char syspath[128];
	snprintf(syspath, sizeof(syspath), "/proc/sys/net/%s", path);
	return writeproc(syspath, value);
}

int writeprocsys(char *path, char *value)
{
	char syspath[128];
	snprintf(syspath, sizeof(syspath), "/proc/sys/%s", path);
	return writeproc(syspath, value);
}

int writevaproc(char *value, char *fmt, ...)
{
	char varbuf[256];
	va_list args;

	va_start(args, (char *)fmt);
	vsnprintf(varbuf, sizeof(varbuf), fmt, args);
	va_end(args);
	return writeproc(varbuf, value);
}

char *get_ipfromsock(int socket, char *ip)
{
	struct sockaddr_storage addr;
	socklen_t len = sizeof(addr);
	getpeername(socket, (struct sockaddr *)&addr, &len);
	if (addr.ss_family == AF_INET) {
		struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
		inet_ntop(AF_INET, &sa->sin_addr, ip, INET_ADDRSTRLEN);
	} else {
		struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
		inet_ntop(AF_INET6, &sa->sin6_addr, ip, INET6_ADDRSTRLEN);
	}
	return ip;
}

unsigned char *get_ether_hwaddr(const char *name, unsigned char *hwaddr)
{
	struct ifreq ifr;
	unsigned char *ret = NULL;
	int s;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return NULL;
	}

	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
		memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
		ret = hwaddr;
	}

	close(s);
	return ret;
}

int set_ether_hwaddr(const char *name, unsigned char *hwaddr)
{
	struct ifreq ifr;
	int ret = 0;
	int s;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr); // must read to update struct
	memcpy(ifr.ifr_hwaddr.sa_data, hwaddr, ETHER_ADDR_LEN);
	ioctl(s, SIOCSIFHWADDR, &ifr); // rewrite with updated mac
	close(s);
	return ret;
}

int set_hwaddr(const char *name, char *hwaddr)
{
	unsigned char mac[6];
	if (ether_atoe(hwaddr, mac)) {
		return set_ether_hwaddr(name, mac);
	}
	return -1;
}

char *get_hwaddr(const char *name, char *eabuf)
{
	unsigned char buf[6];
	unsigned char *mac = get_ether_hwaddr(name, buf);
	if (mac) {
		if (ether_etoa(mac, eabuf)) {
			return eabuf;
		}
	}
	return NULL;
}

#ifdef HAVE_MICRO
void add_blocklist(const char *service, char *ip)
{
}

void add_blocklist_sock(const char *service, int sock)
{
}

int check_blocklist(const char *service, char *ip)
{
	return 0;
}

int check_blocklist_sock(const char *service, int sock)
{
	return 0;
}

#else
#include <pthread.h>

struct blocklist {
	char ip[INET6_ADDRSTRLEN];
	time_t end;
	int count;
	int attempts;
	struct blocklist *next;
} __attribute__((packed));

#define BLOCKTIME 5
static struct blocklist blocklist_root;
static pthread_mutex_t mutex_block = PTHREAD_MUTEX_INITIALIZER;

static void dump_blocklist(void)
{
	struct blocklist *entry = blocklist_root.next;
	FILE *fp = NULL;
	fp = fopen("/tmp/blocklist", "wb");
	if (fp) {
		while (entry) {
			fwrite(entry, sizeof(struct blocklist) - sizeof(void *), 1, fp);
			entry = entry->next;
		}
		fclose(fp);
	}
}

static void init_blocklist(void)
{
	struct blocklist *entry = blocklist_root.next;
	struct blocklist *last = &blocklist_root;
	if (entry) {
		while (entry) {
			last = entry;
			entry = entry->next;
			free(last);
		}
		blocklist_root.next = NULL;
		entry = blocklist_root.next;
		last = &blocklist_root;
	}

	FILE *fp = fopen("/tmp/blocklist", "rb");
	if (fp) {
		while (!feof(fp)) {
			last->next = malloc(sizeof(*entry));
			int elems = fread(last->next, sizeof(struct blocklist) - sizeof(void *), 1, fp);
			if (elems < 1) {
				free(last->next);
				last->next = NULL;
				break;
			}
			last = last->next;
		}
		fclose(fp);
	}
}

void add_blocklist(const char *service, char *ip)
{
	if (ip == NULL)
		return;
	pthread_mutex_lock(&mutex_block);
	init_blocklist();
	struct blocklist *entry = blocklist_root.next;
	struct blocklist *last = &blocklist_root;
	while (entry) {
		if (!strcmp(ip, &entry->ip[0])) {
			entry->count++;
			if (entry->count > 4) {
				entry->end = time(NULL) + BLOCKTIME * 60;
				entry->attempts = 1;
				dd_loginfo(service, "5 failed login attempts reached. block client %s for %d minutes", ip,
					   BLOCKTIME);
			}
			goto end;
		}
		last = entry;
		entry = entry->next;
	}
	last->next = malloc(sizeof(*last));
	if (!last->next) {
		goto end;
	}
	memset(last->next, 0, sizeof(*last));
	strcpy(&last->next->ip[0], ip);
	last->next->end = 0;
	last->next->count = 0;
	last->next->attempts = 0;
	last->next->next = NULL;
end:;
	dump_blocklist();
	pthread_mutex_unlock(&mutex_block);
}

void add_blocklist_sock(const char *service, int conn_fd)
{
	char ip[INET6_ADDRSTRLEN];
	get_ipfromsock(conn_fd, ip);
	add_blocklist(service, ip);
}

int check_blocklist(const char *service, char *ip)
{
	int ret = 0;
	if (ip == NULL)
		return 0;
	pthread_mutex_lock(&mutex_block);
	init_blocklist();
	int change = 0;
	time_t cur = time(NULL);
	struct blocklist *entry = blocklist_root.next;
	struct blocklist *last = &blocklist_root;
	while (entry) {
		if (!strcmp(ip, &entry->ip[0])) {
			if (entry->end > cur) {
				// each try from a block client extends by another 5 minutes;
				entry->attempts++;
				entry->end = time(NULL) + (BLOCKTIME * entry->attempts) * 60;
				dd_loginfo(service, "client %s is blocked, terminate connection, set new blocktime to %d minutes\n",
					   ip, (BLOCKTIME * entry->attempts));
				ret = -1;
				change = 1;
				goto end;
			}
			//time over, free entry
			if (entry->count > 4) {
				dd_loginfo(service, "time is over for client %s, so free it\n", &entry->ip[0]);
				last->next = entry->next;
				free(entry);
				change = 1;
			}
			goto end;
		}
		//time over, free entry
		if (entry->end && entry->end < cur) {
			dd_loginfo(service, "time is over for client %s, so free it\n", &entry->ip[0]);
			last->next = entry->next;
			free(entry);
			entry = last->next;
			change = 1;
			continue;
		}
		last = entry;
		entry = entry->next;
	}
end:;
	if (change)
		dump_blocklist();
	pthread_mutex_unlock(&mutex_block);
	if (nvram_match("ignore_blocklist", "1"))
		return 0;
	return ret;
}

int check_blocklist_sock(const char *service, int conn_fd)
{
	char ip[INET6_ADDRSTRLEN];
	get_ipfromsock(conn_fd, ip);
	return check_blocklist(service, ip);
}
#endif
#ifdef MEMDEBUG
/* some special code for memory leak tracking */

typedef struct MEMENTRY {
	void *reference;
	int size;
	char *func;
	int line;
	struct MEMENTRY *next;
	struct MEMENTRY *last;
};

static struct MEMENTRY root;
static struct MEMENTRY *current = NULL;
void *mymalloc(int size, char *func, int line)
{
	if (!current) {
		current = &root;
		current->last = NULL;
	}
	current->size = size;
	void *ref = malloc(size);
	current->reference = ref;
	current->func = strdup(func);
	current->line = line;
	current->next = malloc(sizeof(struct MEMENTRY));
	memset(current->next, 0, sizeof(struct MEMENTRY));
	current->next->last = current;
	current = current->next;
	return ref;
}

#undef free
void myfree(void *ref, char *func, int line)
{
	int i;
	struct MEMENTRY *c_current = &root;
	while (c_current) {
		if (c_current->reference == ref)
			break;
		c_current = c_current->next;
	}
	free(ref);
	// remove list entry if not root
	if (c_current && c_current->last) {
		c_current->last->next = c_current->next;
		free(c_current->func);
		free(c_current);
		c_current = NULL;
	} else {
		if (!c_current) {
			fprintf(stderr, "function %s line %d does free a untracked pointer", func, line);
		}
	}
	if (c_current) {
		c_current->ref = NULL;
	}
}

void showmemdebugstat(void)
{
	int i;
	struct MEMENTRY *c_current = &root;
	while (c_current) {
		if (c_current->ref && c_current->func)
			fprintf(stderr, "%s line %d leaks %d bytes\n", currrent->func, c_current->line, c_current->size);
		c_current = c_current->next;
	}
}

#endif

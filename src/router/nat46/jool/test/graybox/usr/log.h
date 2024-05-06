#ifndef TEST_GRAYBOX_USR_LOG_H_
#define TEST_GRAYBOX_USR_LOG_H_

#if __GNUC__
#define CHECK_FORMAT(str, args) __attribute__((format(printf, str, args)))
#else
#define CHECK_FORMAT(str, args) /* Nothing */
#endif

void pr_err(const char *fmt, ...) CHECK_FORMAT(1, 2);

#endif /* TEST_GRAYBOX_USR_LOG_H_ */

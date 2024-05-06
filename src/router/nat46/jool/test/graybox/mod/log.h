#ifndef TEST_GRAYBOX_MOD_LOG_H_
#define TEST_GRAYBOX_MOD_LOG_H_

#include <linux/printk.h>

#define log_debug(text, ...) pr_debug("Graybox: " text "\n", ##__VA_ARGS__)
#define log_info(text, ...) pr_info("Graybox: " text "\n", ##__VA_ARGS__)
#define log_err(text, ...) pr_err("Graybox: " text "\n", ##__VA_ARGS__)

#endif /* TEST_GRAYBOX_MOD_LOG_H_ */

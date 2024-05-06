#ifndef TEST_GRAYBOX_COMMON_GRAYBOX_TYPES_H_
#define TEST_GRAYBOX_COMMON_GRAYBOX_TYPES_H_

#include <linux/types.h>

enum graybox_command {
	COMMAND_EXPECT_ADD = 1,
	COMMAND_EXPECT_FLUSH,
	COMMAND_SEND,
	COMMAND_STATS_DISPLAY,
	COMMAND_STATS_FLUSH,
};

enum graybox_attribute {
	/* Command fields */
	ATTR_FILENAME = 1,
	ATTR_PKT,
	ATTR_EXCEPTIONS,

	/* Response fields */
	ATTR_ERROR_CODE,
	ATTR_STATS,

	__ATTR_MAX,
};

struct graybox_proto_stats {
	__u32 successes;
	__u32 failures;
	__u32 queued;
};

struct graybox_stats {
	struct graybox_proto_stats ipv6;
	struct graybox_proto_stats ipv4;
};

#endif /* TEST_GRAYBOX_COMMON_GRAYBOX_TYPES_H_ */

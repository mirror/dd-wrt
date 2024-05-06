#include "mod/common/log.h"

#include <linux/interrupt.h>
#include "mod/common/error_pool.h"
#include "mod/common/wkmalloc.h"
#include "mod/common/translation_state.h"

/*
 * From the logging module's point of view, Jool functions in three different
 * contexts:
 *
 * 1. Packet context (ie. while translating a packet).
 *    It's a softirq, and should never print errors unless they're critical.
 *    (This is because normal errors should be catched in the other contexts.)
 * 2. Userspace client command handlers.
 *    It's user context. It should print errors in the kernel ring buffer, and
 *    also send them to userspace so the client will print them. This is needed
 *    because the console might not be listening to kernel messages, and our
 *    users are not kernel devs.
 * 3. Module insertion and removal (ie. modprobe).
 *    It's the same as 2, except the userspace client is not present, and so
 *    the error message cannot be sent to userspace.
 */

static bool is_packet_context(void)
{
	return in_softirq();
}

void log_err(const char *format, ...)
{
	char *msg;
	va_list args;

	msg = __wkmalloc("error_code.msg", 256, GFP_ATOMIC);
	if (!msg) {
		/* Fall back to shitty print */
		va_start(args, format);
		vprintk(format, args);
		va_end(args);
		return;
	}

	/* Want to avoid vprintk because I can't append a level string to it. */
	va_start(args, format);
	vsnprintf(msg, 256, format, args);
	va_end(args);

	/*
	 * Errors should be catched during user context, so a log_err()
	 * during packet context should be upgraded to critical.
	 */
	if (WARN(is_packet_context(), "%s", msg)) {
		__wkfree("error_code.msg", msg);
		return;
	}

	pr_err("Jool error: %s\n", msg);
	error_pool_add_message(msg);
}

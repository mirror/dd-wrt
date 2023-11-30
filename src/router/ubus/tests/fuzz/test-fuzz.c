#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#include <libubox/blob.h>
#include <libubox/blobmsg.h>

#include "ubusmsg.h"
#include "libubus.h"
#include "libubus-internal.h"

static void _ubus_validate_hdr(const uint8_t *data, size_t size)
{
	if (size > sizeof(struct ubus_msghdr))
		return;

	ubus_validate_hdr((struct ubus_msghdr *) data);
}

static void _ubus_parse_msg(const uint8_t *data, size_t size)
{
	struct blob_attr *attr = (struct blob_attr *) data;

	if (size < sizeof(struct blob_attr *))
		return;

	if (blob_pad_len(attr) > UBUS_MAX_MSGLEN)
		return;

	ubus_parse_msg(attr, size);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	_ubus_validate_hdr(data, size);
	_ubus_parse_msg(data, size);

	return 0;
}

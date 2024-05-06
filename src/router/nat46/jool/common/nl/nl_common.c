#include "mod/common/nl/nl_common.h"

#include "common/types.h"
#include "mod/common/init.h"
#include "mod/common/log.h"
#include "mod/common/nl/nl_core.h"

char *get_iname(struct genl_info *info)
{
	struct joolnlhdr *hdr;
	hdr = get_jool_hdr(info);
	return (hdr->iname[0] != 0) ? hdr->iname : INAME_DEFAULT;
}

struct joolnlhdr *get_jool_hdr(struct genl_info *info)
{
	return (struct joolnlhdr *)((u8 *)info->genlhdr + GENL_HDRLEN);
}

static int validate_magic(struct joolnlhdr *hdr)
{
	if (memcmp(hdr->magic, JOOLNL_HDR_MAGIC, JOOLNL_HDR_MAGIC_LEN) == 0)
		return 0;

	log_err("Don't know what to do: The packet I just received does not follow Jool's protocol.");
	return -EINVAL;
}

static int validate_stateness(struct joolnlhdr *hdr)
{
	switch (hdr->xt) {
	case XT_SIIT:
		if (is_siit_enabled())
			return 0;
		log_err("SIIT Jool has not been modprobed. (Try `modprobe jool_siit`)");
		return -EINVAL;
	case XT_NAT64:
		if (is_nat64_enabled())
			return 0;
		log_err("NAT64 Jool has not been modprobed. (Try `modprobe jool`)");
		return -EINVAL;
	}

	log_err(XT_VALIDATE_ERRMSG);
	return -EINVAL;
}

static int validate_version(struct joolnlhdr *hdr)
{
	__u32 hdr_version = ntohl(hdr->version);

	if ((xlat_version() & 0xFFFF0000u) == (hdr_version & 0xFFFF0000u))
		return 0;

	log_err("Version mismatch. The userspace client's version is %u.%u.%u.%u,\n"
			"but the kernel module is %u.%u.%u.%u.\n"
			"Please update the %s.",
			hdr_version >> 24, (hdr_version >> 16) & 0xFFU,
			(hdr_version >> 8) & 0xFFU, hdr_version & 0xFFU,
			JOOL_VERSION_MAJOR, JOOL_VERSION_MINOR,
			JOOL_VERSION_REV, JOOL_VERSION_DEV,
			(xlat_version() > hdr_version)
					? "userspace client"
					: "kernel module");
	return -EINVAL;
}

int request_handle_start(struct genl_info *info, xlator_type xt,
		struct xlator *jool, bool require_net_admin)
{
	struct joolnlhdr *hdr;
	int error;

	if (require_net_admin && !capable(CAP_NET_ADMIN)) {
		log_err("CAP_NET_ADMIN capability required. (Maybe try su or sudo?)");
		return -EPERM;
	}

	if (!info->attrs) {
		log_err("Userspace request lacks Netlink attributes.");
		return -EINVAL;
	}

	hdr = get_jool_hdr(info);
	if (!hdr) {
		log_err("Userspace request lacks a Jool header.");
		return -EINVAL;
	}
	error = validate_magic(hdr);
	if (error)
		return error;
	error = validate_stateness(hdr);
	if (error)
		return error;
	error = validate_version(hdr);
	if (error)
		return error;

	if (!(hdr->xt & xt)) {
		log_err("Command unsupported by %s translators.", xt2str(hdr->xt));
		return -EINVAL;
	}

	if (jool) {
		error = xlator_find_current(get_iname(info), XF_ANY | hdr->xt, jool);
		if (error == -ESRCH)
			log_err("This namespace lacks an instance named '%s'.", get_iname(info));
		if (error)
			return error;
	}

	return 0;
}

void request_handle_end(struct xlator *jool)
{
	if (jool)
		xlator_put(jool);
}

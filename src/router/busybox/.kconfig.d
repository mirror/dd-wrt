deps_config := \
	sysklogd/Config.in \
	shell/Config.in \
	selinux/Config.in \
	runit/Config.in \
	procps/Config.in \
	mailutils/Config.in \
	printutils/Config.in \
	networking/udhcp/Config.in \
	networking/Config.in \
	miscutils/Config.in \
	util-linux/Config.in \
	modutils/Config.in \
	e2fsprogs/Config.in \
	loginutils/Config.in \
	init/Config.in \
	findutils/Config.in \
	editors/Config.in \
	debianutils/Config.in \
	console-tools/Config.in \
	coreutils/Config.in \
	archival/Config.in \
	libbb/Config.in \
	Config.in

.config include/autoconf.h: $(deps_config)

include/autoconf.h: .config

$(deps_config):

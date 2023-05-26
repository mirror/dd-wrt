ACLOCAL_AMFLAGS = -I m4

sbin_PROGRAMS = mstpd mstpctl

mstpd_SOURCES = \
	main.c epoll_loop.c epoll_loop.h clock_gettime.h brmon.c \
	bridge_track.c bridge_track.h driver.h bridge_ctl.h libnetlink.c \
	libnetlink.h mstp.c mstp.h packet.c packet.h netif_utils.c \
	netif_utils.h ctl_socket_server.c ctl_socket_server.h hmac_md5.c \
	list.h log.h driver_deps.c

mstpctl_SOURCES = \
	ctl_main.c ctl_socket_client.c ctl_socket_client.h ctl_functions.h

mstpd_CFLAGS = \
	-Os -Wall -D_REENTRANT -D__LINUX__ -I. \
	-D_GNU_SOURCE
if ENABLE_DEVEL
  mstpd_CFLAGS += -g3 -O0 -Werror
endif
mstpctl_CFLAGS = $(mstpd_CFLAGS)

EXTRA_DIST = bridge-stp.in utils/ifupdown.sh.in utils/mstp_config_bridge.in \
	utils/mstpd.service.in utils/bash_completion utils/nm-dispatcher.in \
	README.md README.VLANs.md mstpd.spec autogen.sh

CLEANFILES = bridge-stp utils/ifupdown.sh utils/mstp_config_bridge \
	utils/mstpd.service utils/nm-dispatcher
dist_utilsexec_SCRIPTS = utils/ifquery
dist_doc_DATA = LICENSE
utilsexec_SCRIPTS = utils/ifupdown.sh utils/mstp_config_bridge
dist_utilsexec_DATA = utils/mstpctl-utils-functions.sh
sbin_SCRIPTS = bridge-stp
dist_sysconf_DATA = bridge-stp.conf
dist_man_MANS = utils/mstpctl.8 utils/mstpctl-utils-interfaces.5

AM_DISTCHECK_CONFIGURE_FLAGS = --exec_prefix=$$dc_install_base \
	--libexecdir=$$dc_install_base/lib \
	--with-systemdunitdir=$$dc_install_base/usr/lib/systemd/system

all-local: bridge-stp utils/ifupdown.sh utils/mstp_config_bridge \
	utils/mstpd.service utils/nm-dispatcher

utilsexecdir=$(libexecdir)/mstpctl-utils
nmlibdir=$(prefix)/lib/NetworkManager/dispatcher.d

mstpdfile=$(sbindir)/mstpd
mstpctlfile=$(sbindir)/mstpctl
bridgestpfile=$(sbindir)/bridge-stp
mstpdpidfile=$(localstatedir)/run/mstpd.pid
bridgestpconffile=$(sysconfdir)/bridge-stp.conf
ifupdownfile=$(utilsexecdir)/ifupdown.sh
utilsfuncfile=$(utilsexecdir)/mstpctl-utils-functions.sh
configbridgefile=$(utilsexecdir)/mstp_config_bridge
ifqueryfile=$(utilsexecdir)/ifquery

mstpd_CFLAGS += -DMSTPD_PID_FILE='"$(mstpdpidfile)"'

# See https://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.69/html_node/Installation-Directory-Variables.html#index-sysconfdir-188
populate_template = sed \
	-e 's|@mstpdfile[@]|$(mstpdfile)|g' \
	-e 's|@bridgestpfile[@]|$(bridgestpfile)|g' \
	-e 's|@mstpdpidfile[@]|$(mstpdpidfile)|g' \
	-e 's|@mstpctlfile[@]|$(mstpctlfile)|g' \
	-e 's|@bridgestpconffile[@]|$(bridgestpconffile)|g' \
	-e 's|@ifupdownfile[@]|$(ifupdownfile)|g' \
	-e 's|@utilsfuncfile[@]|$(utilsfuncfile)|g' \
	-e 's|@configbridgefile[@]|$(configbridgefile)|g' \
	-e 's|@ifqueryfile[@]|$(ifqueryfile)|g'
bridge-stp utils/ifupdown.sh utils/mstp_config_bridge utils/mstpd.service utils/nm-dispatcher: Makefile
	rm -f $@ $@.tmp
	mkdir -p $(@D)
	srcdir=''; test -f ./$@.in || srcdir="$(srcdir)/"; \
	$(populate_template) $${srcdir}$@.in > $@.tmp
	chmod 755 $@.tmp
	mv $@.tmp $@
bridge-stp: $(srcdir)/bridge-stp.in
utils/ifupdown.sh: $(srcdir)/utils/ifupdown.sh.in
utils/mstp_config_bridge: $(srcdir)/utils/mstp_config_bridge.in
utils/nm-dispatcher: $(srcdir)/utils/nm-dispatcher.in

sbindest=$(DESTDIR)$(sbindir)
utilsdest=$(DESTDIR)$(utilsexecdir)
etcdest=$(DESTDIR)$(sysconfdir)
docdest=$(DESTDIR)$(docdir)
bashcompdest=$(DESTDIR)$(bashcompletiondir)
systemdunitdest=$(DESTDIR)$(systemdunitdir)
nmlibdest=$(DESTDIR)$(nmlibdir)

if ENABLE_INSTALL_IFUPDOWN_SCRIPTS
IFUPDOWN_SCRIPT_INSTALL_CMDS = \
	$(MKDIR_P) $(etcdest)/network/if-pre-up.d ; \
	ln -sf $(utilsexecdir)/ifupdown.sh $(etcdest)/network/if-pre-up.d/mstpctl ; \
	$(MKDIR_P) $(etcdest)/network/if-post-down.d ; \
	ln -sf $(utilsexecdir)/ifupdown.sh $(etcdest)/network/if-post-down.d/mstpctl
endif

install-exec-hook:
	ln -sf $(sbindir)/bridge-stp $(sbindest)/mstp_restart
	ln -sf $(sbindir)/bridge-stp $(utilsdest)/mstpctl_restart_config
	$(IFUPDOWN_SCRIPT_INSTALL_CMDS)
	$(MKDIR_P) $(bashcompdest)
	$(INSTALL_DATA) $(srcdir)/utils/bash_completion $(bashcompdest)/mstpctl
	if [ -n "$(systemdunitdir)" ]; then \
	  $(MKDIR_P) $(systemdunitdest) ; \
	  $(INSTALL_DATA) $(top_builddir)/utils/mstpd.service $(systemdunitdest) ; \
	fi
	if [ -d $(nmlibdir) ]; then \
	  $(MKDIR_P) $(nmlibdest) ; \
	  $(INSTALL_SCRIPT) $(top_builddir)/utils/nm-dispatcher $(nmlibdest)/45-mstpd ; \
	fi
	$(MKDIR_P) $(docdest)
	$(INSTALL_DATA) $(srcdir)/README.md $(docdest)/README
	$(INSTALL_DATA) $(srcdir)/README.VLANs.md $(docdest)/README.VLANs

uninstall-hook:
	-rm -f $(sbindest)/mstp_restart
	-rm -f $(utilsdest)/mstpctl_restart_config
	-rmdir $(utilsdest)
	-rm -f $(bashcompdest)/mstpctl
	-if [ -d $(etcdest)/network/if-pre-up.d ]; then \
	  rm -f $(etcdest)/network/if-pre-up.d/mstpctl ; \
	fi
	-if [ -d $(etcdest)/network/if-post-down.d ]; then \
	  rm -f $(etcdest)/network/if-post-down.d/mstpctl ; \
	fi
	-if [ -n "$(systemdunitdir)" ]; then \
	  rm -f $(systemdunitdest)/mstpd.service ; \
	fi
	-if [ -d $(nmlibdest) ]; then \
	  rm -f $(nmlibdest)/45-mstpd ; \
	fi
	-rm -f $(docdest)/README
	-rm -f $(docdest)/README.VLANs
	-rmdir $(docdest)

rpm: mstpd.spec
	Version="$$(perl -n -e 'if(/AC_INIT[^,]+,\s+\[([^]]+)\]/) \
	{ $$V=$$1; $$V=~s/-/_/g; print $$V; exit; }' configure.ac)" ; \
	[ -n "$$Version" ] || exit 3 ; SrcDir="$$(pwd)" ; \
	mkdir -p "../rpmbuild/SOURCES" && cd "../rpmbuild/SOURCES" && \
	git clone "$$SrcDir" "mstpd-$$Version" && \
	rm -rf "mstpd-$$Version/.git" && \
	tar -cvzf "mstpd-$$Version.tar.gz" "mstpd-$$Version" && \
	rm -rf "mstpd-$$Version" && \
	cd .. && mkdir -p SPECS && \
	cp "$$SrcDir/mstpd.spec" SPECS/mstpd.spec && \
	rpmbuild --define "_topdir $$(pwd)" -ba SPECS/mstpd.spec

mstpd.spec: configure.ac
	Version="$$(perl -n -e 'if(/AC_INIT[^,]+,\s+\[([^]]+)\]/) \
	{ $$V=$$1; $$V=~s/-/_/g; print $$V; exit; }' configure.ac)" ; \
	[ -n "$$Version" ] || exit 3 ; \
	perl -i -p -e 's/^(\s*Version:\s+).*$$/$${1}'"$$Version/" mstpd.spec

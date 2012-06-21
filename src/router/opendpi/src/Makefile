


OPENDPI_PATH2 := ${TOP}/opendpi/opendpi
OPENDPI_SRC := opendpi
OPENDPI_PRO := ${OPENDPI_SRC}/lib/protocols

ccflags-y += -I${src}/${OPENDPI_SRC}/include -I${src}/${OPENDPI_SRC}/lib -DOPENDPI_NETFILTER_MODULE -DHAVE_NTOP

EXTRA_CFLAGS := $(ccflags-y)
MODULES_DIR := $(INSTALLDIR)/opendpi/lib/opendpi
KERNEL_DIR := ${LINUXDIR}

obj-m := xt_opendpi.o
xt_opendpi-y := main.o ${OPENDPI_SRC}/lib/ipq_main.o \
                ${OPENDPI_PRO}/afp.o ${OPENDPI_PRO}/dns.o ${OPENDPI_PRO}/halflife2_and_mods.o \
                ${OPENDPI_PRO}/kontiki.o ${OPENDPI_PRO}/msn.o ${OPENDPI_PRO}/pcanywhere.o \
                ${OPENDPI_PRO}/rtsp.o ${OPENDPI_PRO}/ssdp.o ${OPENDPI_PRO}/tftp.o \
                ${OPENDPI_PRO}/xbox.o ${OPENDPI_PRO}/aimini.o ${OPENDPI_PRO}/edonkey.o \
                ${OPENDPI_PRO}/http.o ${OPENDPI_PRO}/mail_imap.o ${OPENDPI_PRO}/mysql.o \
                ${OPENDPI_PRO}/popo.o ${OPENDPI_PRO}/secondlife.o ${OPENDPI_PRO}/ssh.o \
                ${OPENDPI_PRO}/thunder.o ${OPENDPI_PRO}/xdmcp.o ${OPENDPI_PRO}/applejuice.o \
                ${OPENDPI_PRO}/fasttrack.o ${OPENDPI_PRO}/i23v5.o ${OPENDPI_PRO}/mail_pop.o \
                ${OPENDPI_PRO}/netbios.o ${OPENDPI_PRO}/postgres.o ${OPENDPI_PRO}/shoutcast.o \
                ${OPENDPI_PRO}/ssl.o ${OPENDPI_PRO}/tvants.o ${OPENDPI_PRO}/yahoo.o \
                ${OPENDPI_PRO}/battlefield.o ${OPENDPI_PRO}/feidian.o ${OPENDPI_PRO}/iax.o \
                ${OPENDPI_PRO}/mail_smtp.o ${OPENDPI_PRO}/nfs.o ${OPENDPI_PRO}/pplive.o \
                ${OPENDPI_PRO}/sip.o ${OPENDPI_PRO}/stealthnet.o ${OPENDPI_PRO}/tvuplayer.o \
                ${OPENDPI_PRO}/zattoo.o ${OPENDPI_PRO}/bgp.o ${OPENDPI_PRO}/filetopia.o \
                ${OPENDPI_PRO}/icecast.o ${OPENDPI_PRO}/manolito.o ${OPENDPI_PRO}/non_tcp_udp.o \
                ${OPENDPI_PRO}/ppstream.o ${OPENDPI_PRO}/smb.o ${OPENDPI_PRO}/steam.o \
                ${OPENDPI_PRO}/usenet.o ${OPENDPI_PRO}/bittorrent.o ${OPENDPI_PRO}/flash.o \
                ${OPENDPI_PRO}/imesh.o ${OPENDPI_PRO}/mdns.o ${OPENDPI_PRO}/ntp.o \
                ${OPENDPI_PRO}/qq.o ${OPENDPI_PRO}/snmp.o ${OPENDPI_PRO}/stun.o \
                ${OPENDPI_PRO}/veohtv.o ${OPENDPI_PRO}/dhcp.o ${OPENDPI_PRO}/ftp.o \
                ${OPENDPI_PRO}/ipp.o ${OPENDPI_PRO}/meebo.o ${OPENDPI_PRO}/openft.o \
                ${OPENDPI_PRO}/quake.o ${OPENDPI_PRO}/socrates.o ${OPENDPI_PRO}/syslog.o \
                ${OPENDPI_PRO}/vnc.o ${OPENDPI_PRO}/directconnect.o ${OPENDPI_PRO}/gadu_gadu.o \
                ${OPENDPI_PRO}/irc.o ${OPENDPI_PRO}/mgcp.o ${OPENDPI_PRO}/oscar.o \
                ${OPENDPI_PRO}/rdp.o ${OPENDPI_PRO}/sopcast.o ${OPENDPI_PRO}/tds.o \
                ${OPENDPI_PRO}/winmx.o ${OPENDPI_PRO}/directdownloadlink.o ${OPENDPI_PRO}/gnutella.o \
                ${OPENDPI_PRO}/jabber.o ${OPENDPI_PRO}/mms.o ${OPENDPI_PRO}/pando.o \
                ${OPENDPI_PRO}/rtp.o ${OPENDPI_PRO}/soulseek.o ${OPENDPI_PRO}/telnet.o \
                ${OPENDPI_PRO}/world_of_warcraft.o ${OPENDPI_PRO}/maplestory.o \
                ${OPENDPI_PRO}/ldap.o ${OPENDPI_PRO}/dhcpv6.o \
                ${OPENDPI_PRO}/dofus.o ${OPENDPI_PRO}/kerberos.o \
                ${OPENDPI_PRO}/pptp.o ${OPENDPI_PRO}/warcraft3.o \
                ${OPENDPI_PRO}/http_activesync.o ${OPENDPI_PRO}/armagetron.o \
                ${OPENDPI_PRO}/florensia.o ${OPENDPI_PRO}/world_of_kung_fu.o \
                ${OPENDPI_PRO}/mssql.o ${OPENDPI_PRO}/guildwars.o \
                ${OPENDPI_PRO}/crossfire.o ${OPENDPI_PRO}/fiesta.o \
		${OPENDPI_PRO}/citrix.o ${OPENDPI_PRO}/netflow.o \
		${OPENDPI_PRO}/sflow.o ${OPENDPI_PRO}/skype.o \
		${OPENDPI_PRO}/dcerpc.o ${OPENDPI_PRO}/dropbox.o \
		${OPENDPI_PRO}/radius.o

all:
	if test -d ${OPENDPI_SRC}; then \
		cp ${OPENDPI_PATH2}/* ${OPENDPI_SRC} -R; \
	else \
		mkdir ${OPENDPI_SRC}; \
		cp ${OPENDPI_PATH2}/* ${OPENDPI_SRC} -R; \
	fi
	make -C ${KERNEL_DIR} CROSS_COMPILE="ccache $(CROSS_COMPILE)" M=$$PWD;
	rm -r ${OPENDPI_SRC}
modules:
	make -C ${KERNEL_DIR} M=$$PWD $@;
modules_install:
	install -D xt_opendpi.ko $(INSTALLDIR)/lib/opendpi/xt_opendpi.ko
#	make -C ${KERNEL_DIR} M=$$PWD $@;
#	depmod -a;
clean:
	make -C ${KERNEL_DIR} M=$$PWD $@;
	rm -rf modules.order ${OPENDPI_SRC}

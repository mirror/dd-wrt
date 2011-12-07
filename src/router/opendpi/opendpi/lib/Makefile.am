AM_CPPFLAGS = -I$(top_srcdir)/src/include/

lib_LTLIBRARIES = libopendpi.la

noinst_HEADERS = ipq_main.h \
			ipq_protocol_history.h \
			ipq_protocols.h \
			ipq_structs.h \
			ipq_utils.h \
			linux_compat.h


# this will create a library 1.3.0
LIB_AC=1
LIB_REV=3
LIB_ANC=0

libopendpi_la_LDFLAGS=-version-info ${LIB_AC}:${LIB_REV}:${LIB_ANC}

libopendpi_la_SOURCES = ipq_main.c \
			protocols/afp.c \
			protocols/aimini.c \
			protocols/applejuice.c \
			protocols/armagetron.c \
			protocols/battlefield.c \
			protocols/bgp.c \
			protocols/bittorrent.c \
			protocols/crossfire.c \
			protocols/dhcp.c \
			protocols/dhcpv6.c \
			protocols/directconnect.c \
			protocols/directdownloadlink.c \
			protocols/dns.c \
			protocols/dofus.c \
			protocols/edonkey.c \
			protocols/fasttrack.c \
			protocols/feidian.c \
			protocols/fiesta.c \
			protocols/filetopia.c \
			protocols/flash.c \
			protocols/florensia.c \
			protocols/ftp.c \
			protocols/gadu_gadu.c \
			protocols/gnutella.c \
			protocols/guildwars.c \
			protocols/halflife2_and_mods.c \
			protocols/http_activesync.c \
			protocols/http.c \
			protocols/i23v5.c \
			protocols/iax.c \
			protocols/icecast.c \
			protocols/imesh.c \
			protocols/ipp.c \
			protocols/irc.c \
			protocols/jabber.c \
			protocols/kerberos.c \
			protocols/kontiki.c \
			protocols/ldap.c \
			protocols/mail_imap.c \
			protocols/mail_pop.c \
			protocols/mail_smtp.c \
			protocols/manolito.c \
			protocols/maplestory.c \
			protocols/mdns.c \
			protocols/meebo.c \
			protocols/mgcp.c \
			protocols/mms.c \
			protocols/msn.c \
			protocols/mssql.c \
			protocols/mysql.c \
			protocols/netbios.c \
			protocols/nfs.c \
			protocols/non_tcp_udp.c \
			protocols/ntp.c \
			protocols/openft.c \
			protocols/oscar.c \
			protocols/pando.c \
			protocols/pcanywhere.c \
			protocols/popo.c \
			protocols/postgres.c \
			protocols/pplive.c \
			protocols/ppstream.c \
			protocols/pptp.c \
			protocols/qq.c \
			protocols/quake.c \
			protocols/rdp.c \
			protocols/rtp.c \
			protocols/rtsp.c \
			protocols/secondlife.c \
			protocols/shoutcast.c \
			protocols/sip.c \
			protocols/smb.c \
			protocols/snmp.c \
			protocols/socrates.c \
			protocols/sopcast.c \
			protocols/soulseek.c \
			protocols/ssdp.c \
			protocols/ssh.c \
			protocols/ssl.c \
			protocols/stealthnet.c \
			protocols/steam.c \
			protocols/stun.c \
			protocols/syslog.c \
			protocols/tds.c \
			protocols/telnet.c \
			protocols/tftp.c \
			protocols/thunder.c \
			protocols/tvants.c \
			protocols/tvuplayer.c \
			protocols/usenet.c \
			protocols/veohtv.c \
			protocols/vnc.c \
			protocols/warcraft3.c \
			protocols/winmx.c \
			protocols/world_of_kung_fu.c \
			protocols/world_of_warcraft.c \
			protocols/xbox.c \
			protocols/xdmcp.c \
			protocols/yahoo.c \
			protocols/zattoo.c


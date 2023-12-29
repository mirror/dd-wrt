#bin_PROGRAMS = ndpi_network_list_compile
AM_CPPFLAGS = -I$(top_srcdir)/src/include/  -I$(top_srcdir)/src/lib/third_party/include/

CFLAGS += -fPIC -DPIC -DNDPI_LIB_COMPILATION # --coverage

libndpi_a_includedir = $(includedir)/libndpi-@VERSION@/libndpi

#BUILT_SOURCES = ndpi_network_list.c.inc

libndpi_a_include_HEADERS = ../include/ndpi_api.h \
			     ../include/ndpi_define.h \
			     ../include/ndpi_main.h \
			     ../include/ndpi_includes.h \
			     ../include/ndpi_config.h       \
			     ../include/ndpi_protocol_ids.h \
			     ../include/ndpi_typedefs.h     \
			     third_party/include/ndpi_md5.h \
			     third_party/include/ndpi_sha1.h \
			     third_party/include/gcrypt_light.h \
			     third_party/include/hll.h \
			     third_party/include/libcache.h

libndpi_a_SOURCES = ndpi_content_match.c.inc \
		     ndpi_main.c \
		     ndpi_utils.c \
		     ndpi_bitmap.c \
		     ndpi_analyze.c \
		     ndpi_classify.c \
		     ndpi_serializer.c \
		     ndpi_community_id.c \
		     ndpi_domain_classify.c \
		     ndpi_filter.c \
		     ndpi_geoip.c \
		     ndpi_binary_bitmap.c \
		     ndpi_domain_classify.c \
		     ndpi_bitmap64.c \
		     ndpi_binary_bitmap.c \
		     ndpi_hash.c \
		     protocols/activision.c \
		     protocols/afp.c \
		     protocols/alicloud.c \
		     protocols/ajp.c \
		     protocols/among_us.c \
		     protocols/amazon_video.c \
		     protocols/apple_push.c \
		     protocols/armagetron.c \
		     protocols/avast.c \
		     protocols/avast_securedns.c \
		     protocols/amqp.c \
		     protocols/bacnet.c \
		     protocols/beckhoff_ads.c \
		     protocols/bgp.c \
		     protocols/bitcoin.c \
		     protocols/bittorrent.c \
		     protocols/bjnp.c \
		     protocols/can.c \
		     protocols/capwap.c \
		     protocols/cassandra.c \
		     protocols/checkmk.c \
		     protocols/ciscovpn.c \
		     protocols/citrix.c \
		     protocols/cpha.c \
		     protocols/coap.c \
		     protocols/collectd.c \
		     protocols/corba.c \
		     protocols/crossfire.c \
		     protocols/crynet.c \
		     protocols/csgo.c \
		     protocols/dcerpc.c \
		     protocols/dhcp.c \
		     protocols/dhcpv6.c \
		     protocols/diameter.c \
		     protocols/discord.c \
		     protocols/dns.c \
		     protocols/dnscrypt.c \
		     protocols/dnp3.c \
		     protocols/dofus.c \
		     protocols/drda.c \
		     protocols/dropbox.c \
		     protocols/eaq.c \
		     protocols/edonkey.c \
		     protocols/elastic_search.c \
		     protocols/epicgames.c \
		     protocols/ethereum.c \
		     protocols/ethersio.c \
		     protocols/ethernet_ip.c \
		     protocols/fastcgi.c \
		     protocols/fins.c \
		     protocols/fix.c \
		     protocols/ftp_control.c \
		     protocols/ftp_data.c \
		     protocols/genshin_impact.c \
		     protocols/git.c \
		     protocols/gnutella.c \
		     protocols/gtp.c \
		     protocols/guildwars.c \
		     protocols/h323.c \
		     protocols/halflife2_and_mods.c \
		     protocols/haproxy.c \
		     protocols/hart-ip.c \
		     protocols/hislip.c \
		     protocols/hots.c \
		     protocols/hpvirtgrp.c \
		     protocols/hsrp.c \
		     protocols/http.c \
		     protocols/http2.c \
		     protocols/iax.c \
		     protocols/icecast.c \
		     protocols/i3d.c \
		     protocols/imo.c \
		     protocols/ieee-c37118.c \
		     protocols/iec60870-5-104.c \
		     protocols/ipsec.c \
		     protocols/ipp.c \
		     protocols/irc.c \
		     protocols/iso9506-1-mms.c \
		     protocols/ethersbus.c \
		     protocols/jabber.c \
		     protocols/kakaotalk_voice.c \
		     protocols/kerberos.c \
		     protocols/kismet.c \
		     protocols/kontiki.c \
		     protocols/ldap.c \
		     protocols/lotus_notes.c \
		     protocols/line.c \
		     protocols/lisp.c \
		     protocols/mail_imap.c \
		     protocols/mail_pop.c \
		     protocols/mail_smtp.c \
		     protocols/maplestory.c \
		     protocols/megaco.c \
		     protocols/memcached.c \
		     protocols/merakicloud.c \
		     protocols/mining.c \
		     protocols/mgcp.c \
		     protocols/modbus.c \
		     protocols/monero.c \
		     protocols/mongodb.c \
		     protocols/mpegts.c \
		     protocols/mpegdash.c \
		     protocols/mqtt.c \
		     protocols/mssql_tds.c \
		     protocols/munin.c \
		     protocols/mysql.c \
		     protocols/natpmp.c \
		     protocols/nats.c \
		     protocols/nest_log_sink.c \
		     protocols/netbios.c \
		     protocols/netflow.c \
		     protocols/nintendo.c \
		     protocols/nfs.c \
		     protocols/noe.c \
		     protocols/non_tcp_udp.c \
		     protocols/ntp.c \
		     protocols/oicq.c \
		     protocols/opc-ua.c \
		     protocols/openvpn.c \
		     protocols/ookla.c \
		     protocols/oracle.c \
		     protocols/postgres.c \
		     protocols/ppstream.c \
		     protocols/pptp.c \
		     protocols/profinet_io.c \
		     protocols/protobuf.c \
		     protocols/ptpv2.c \
		     protocols/qq.c \
		     protocols/quic.c \
		     protocols/radius.c \
		     protocols/raknet.c \
		     protocols/rdp.c \
		     protocols/redis_net.c \
		     protocols/rmcp.c \
		     protocols/rsh.c \
		     protocols/rsync.c \
		     protocols/rtcp.c \
		     protocols/rtmp.c \
		     protocols/rtp.c \
		     protocols/rtps.c \
		     protocols/rtsp.c \
		     protocols/rx.c \
		     protocols/s7comm.c \
		     protocols/sd_rtn.c \
		     protocols/sflow.c \
		     protocols/sip.c \
		     protocols/skinny.c \
		     protocols/skype.c \
		     protocols/slp.c \
		     protocols/smb.c \
		     protocols/snmp_proto.c \
		     protocols/soap.c \
		     protocols/socks45.c \
		     protocols/softether.c \
		     protocols/source_engine.c \
		     protocols/spotify.c \
		     protocols/ssdp.c \
		     protocols/ssh.c \
		     protocols/starcraft.c \
		     protocols/steam.c \
		     protocols/stun.c \
		     protocols/syncthing.c \
		     protocols/syslog.c \
		     protocols/tailscale.c \
		     protocols/tcp_udp.c \
		     protocols/teamspeak.c \
		     protocols/teamviewer.c \
		     protocols/telegram.c \
		     protocols/telnet.c \
		     protocols/tftp.c \
		     protocols/thrift.c \
		     protocols/tls.c \
		     protocols/threema.c \
		     protocols/tplink_shp.c \
		     protocols/tuya_lp.c \
		     protocols/teredo.c \
		     protocols/riotgames.c \
		     protocols/tivoconnect.c \
		     protocols/tvuplayer.c \
		     protocols/tocaboca.c \
		     protocols/ubntac2.c \
		     protocols/ultrasurf.c \
		     protocols/usenet.c \
		     protocols/viber.c \
		     protocols/vhua.c \
		     protocols/vmware.c \
		     protocols/vnc.c \
		     protocols/vxlan.c \
		     protocols/warcraft3.c \
		     protocols/websocket.c \
		     protocols/whoisdas.c \
		     protocols/world_of_kung_fu.c \
		     protocols/world_of_warcraft.c \
		     protocols/wsd.c \
		     protocols/xbox.c \
		     protocols/xdmcp.c \
		     protocols/xiaomi.c \
		     protocols/zattoo.c \
		     protocols/zeromq.c \
		     protocols/smpp.c \
		     protocols/tinc.c \
		     protocols/someip.c \
		     protocols/whatsapp.c \
		     protocols/wireguard.c \
		     protocols/z3950.c \
		     protocols/zabbix.c \
		     third_party/include/ahocorasick.h \
		     third_party/include/ndpi_patricia.h \
		     third_party/include/libcache.h \
		     third_party/src/ahocorasick.c \
		     third_party/src/ndpi_md5.c \
		     third_party/src/ndpi_sha1.c \
		     third_party/src/libinjection_html5.c \
		     third_party/src/libinjection_sqli.c \
		     third_party/src/libinjection_xss.c \
		     third_party/src/libcache.c \
		     third_party/src/btlib.c \
		     third_party/src/ndpi_patricia.c \
		     third_party/src/hll/MurmurHash3.c \
		     third_party/src/hll/hll.c \
		     third_party/src/gcrypt_light.c

lib_LIBRARIES = libndpi.a

libndpi_a_CPPFLAGS = -I$(top_srcdir)/src/include/  -I$(top_srcdir)/src/lib/third_party/include/ 
#@HS_INC@

#EXTRA_DIST = third_party/src/ndpi_patricia.c

#PROTO_CFG_FILES = $(wildcard ndpi_network_list_*.yaml)
#
#ndpi_network_list.c.inc: $(PROTO_CFG_FILES) ndpi_network_list_compile
#		./ndpi_network_list_compile -o $@ $(PROTO_CFG_FILES)


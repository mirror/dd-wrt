The l2tpv3tun utility is a command line utility for configuring static
L2TPv3 ethernet pseudowires. It is planned to merge it into the
iproute2 ip utility, though to do so will require some work to
reimplement the netlink interface to not use libnl.

To use l2tpv3tun, you will need libnl installed and a kernel with new
L2TPv3 drivers (2.6.35 or later).

Static (unmanaged) L2TPv3 tunnels are those where there is no L2TP
control protocol; tunnels are configured at each side manually. No
userspace L2TP daemon is needed.

To create an L2TPv3 ethernet pseudowire between local host 192.168.1.1
and peer 192.168.1.2, using IP addresses 10.5.1.1 and 10.5.1.2 for the
tunnel endpoints:-

# modprobe l2tp_eth
# modprobe l2tp_netlink

# l2tpv3tun help
Usage: l2tpv3tun add tunnel
          remote ADDR local ADDR
          tunnel_id ID peer_tunnel_id ID
          [ encap { ip | udp } ]
          [ udp_sport PORT ] [ udp_dport PORT ]
       l2tpv3tun add session
          tunnel_id ID
          session_id ID peer_session_id ID
          [ cookie HEXSTR ] [ peer_cookie HEXSTR ]
          [ offset OFFSET ] [ peer_offset OFFSET ]
       l2tpv3tun del tunnel tunnel_id ID
       l2tpv3tun del session tunnel_id ID session_id ID
       l2tpv3tun show tunnel [ tunnel_id ID ]
       l2tpv3tun show session [ tunnel_id ID ] [ session_id ID ]

# l2tpv3tun add tunnel tunnel_id 1 peer_tunnel_id 1 udp_sport 5000 \
  udp_dport 5000 encap udp local 192.168.1.1 remote 192.168.1.2
# l2tpv3tun add session tunnel_id 1 session_id 1 peer_session_id 1
# ifconfig -a
# ip addr add 10.5.1.2/32 peer 10.5.1.1/32 dev l2tpeth0
# ifconfig l2tpeth0 up

Choose IP addresses to be the address of a local IP interface and that
of the remote system. The IP addresses of the l2tpeth0 interface can be
anything suitable.

Repeat the above at the peer, with ports, tunnel/session ids and IP
addresses reversed.  The tunnel and session IDs can be any non-zero
32-bit number, but the values must be reversed at the peer.

Host 1                         Host2
udp_sport=5000                 udp_sport=5001
udp_dport=5001                 udp_dport=5000
tunnel_id=42                   tunnel_id=45
peer_tunnel_id=45              peer_tunnel_id=42
session_id=128                 session_id=5196755
peer_session_id=5196755        peer_session_id=128

When done at both ends of the tunnel, it should be possible to send
data over the network. e.g.

# ping 10.5.1.1

L2TP tunnels and sessions may be listed using l2tpv3tun.

# l2tpv3tun show tunnel
Tunnel 1, encap UDP
  From 192.168.1.1 to 192.168.1.2
  Peer tunnel 1
  UDP source / dest ports: 5000/5001

# l2tpv3tun show session
Session 1 in tunnel 1
  Peer session 1, tunnel 1
  interface name: l2tpeth0
  offset 0, peer offset 0

Acknowledgements
================

l2tpv3tun was written for the OpenL2TP project.
http://www.openl2tp.org

Katalix Systems Ltd develops and supports OpenL2TP, a GPL-licensed
L2TP server.

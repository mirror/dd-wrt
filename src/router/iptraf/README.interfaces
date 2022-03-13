============================================================================
Supported Interface Information			(Needs update)
as of version 2.8, July 2002
----------------------------------------------------------------------------

IPTraf has been slowly improving with its interface support since its
first release.  IPTraf currently supports the following types of links:

Local loopback
Ethernet (10 and 100 Mbps)
SLIP and variants
Asynchronous PPP over analog telephone lines
FDDI (now includes Ethernet-emulating interfces)
Frame Relay FRAD/DLCI interfaces (new as of IPTraf 2.5.0)
PLIP (Parallel Line IP)
DVB satellite-receive interfaces
SBNI long-range modem interfaces
Wireless LAN interfaces
Free s/WAN logical interfaces
IPsec logical interfaces
Some tunnelling interfaces
Some bridging interfaces
- add 802.1ad and QinQ VLAN handling
- SIT tunnels support
- support GRE-over-IP tunnels

ADDITIONAL INTERFACE SUPPORT

As much as I would like to support every concievable interface in
existence, we know that's just not possible.  I myself do not have a lot
of interface types.  However, that does not mean I'm unwilling to support
more.

So here's the deal.  If you'd like me to include support for a new type of
interface I will need this information as much as possible:

* Resulting link type in spkt_family after a recvfrom() on a (PF_PACKET,
  SOCK_RAW) socket (ARPHRD_ETHER, ARPHRD_PPP, etc).
* Standard interface name for the type of network medium (eth0, eth1,
  ppp0, etc) after the recvfrom() mentioned above.
* Packet structure.  How many bytes are there in its data-link header
  (with Ethernet, there are 14, with FDDI, 21) as returned by recvfrom on
  a (PF_PACKET, SOCK_RAW) socket?
* Pointers to other sources of information if possible.

Then finally, if you come up with a request for support for a new
interface, I'd really like an offer to have it tested, obviously, since I
do not have the interface myself If I do not receive an offer to test, then
support cannot be included.

Patches, even quick-and-dirty ones, are very much welcome.

All information and patches will be fully credited in the CHANGES file.

Looking forward to serving you.

-- Gerard <riker@seul.org>
-- Phil <pcameron@redhat.com>

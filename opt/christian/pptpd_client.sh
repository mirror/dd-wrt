#!/bin/sh
USER=$(/usr/sbin/nvram get pptpd_client_srvuser)
PASS=$(/usr/sbin/nvram get pptpd_client_srvpass)
IPPARAM=$(/usr/sbin/nvram get pptpd_client_ipparam)
MTU=$(/usr/sbin/nvram get pptpd_client_mtu)
MRU=$(/usr/sbin/nvram get pptpd_client_mru)
#warum 30 sekunden warten?, warum ueberhaupt warten? 
sleep 3
mkdir /tmp/pptpd_client
mkdir /tmp/ppp
cp /etc/config/pptpd_client.options /tmp/pptpd_client/options.vpn
echo -n "name "  >> /tmp/pptpd_client/options.vpn
echo "$USER" >> /tmp/pptpd_client/options.vpn
echo -n "password "  >> /tmp/pptpd_client/options.vpn
echo "$PASS" >> /tmp/pptpd_client/options.vpn
echo -n "ipparam "  >> /tmp/pptpd_client/options.vpn
echo ${IPPARAM} >> /tmp/pptpd_client/options.vpn
echo -n "mtu " >> /tmp/pptpd_client/options.vpn
echo ${MTU} >> /tmp/pptpd_client/options.vpn
echo -n "mru " >> /tmp/pptpd_client/options.vpn
echo ${MRU} >> /tmp/pptpd_client/options.vpn
#/usr/sbin/nvram get pptpd_client_eoptions >> /tmp/pptpd_client/options.vpn
cp /etc/config/pptpd_client.vpn /tmp/pptpd_client/vpn
chmod +x /tmp/pptpd_client/vpn
cp /etc/config/pptpd_client.ip-up /tmp/pptpd_client/ip-up
chmod +x /tmp/pptpd_client/ip-up
cp /etc/config/pptpd_client.ip-down /tmp/pptpd_client/ip-down
chmod +x /tmp/pptpd_client/ip-down
pidof vpn || /tmp/pptpd_client/vpn start >/tmp/pptpd_client/log 2>&1

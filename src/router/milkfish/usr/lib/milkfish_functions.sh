#!/bin/sh
######################################################################
# This program is free software; you can redistribute it and/or      #
# modify it under the terms of the GNU General Public License as     #
# published by the Free Software Foundation; either version 3 of the #
# License, or (at your option) any later version.                    #
#                                                                    #
# This program is distributed in the hope that it will be useful,    #
# but WITHOUT ANY WARRANTY; without even the implied warranty of     #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      #
# GNU General Public License for more details.                       #
#                                                                    #
# You should have received a copy of the GNU General Public License  #
# along with this program; if not, write to the                      #
# Free Software Foundation, Inc.,                                    #
# 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA             #
######################################################################
# This file is part of a Milkfish Embedded OpenSER SIP Router Setup  #
# More information can be found at http://www.milkfish.org           #
#                                                                    #
# The Milkfish Router Services - Shell Function Library              #
#                                                                    #
# Built/Version:  20071230                                           #
# Author/Contact: Franz Streibl <fronce@sipwerk.com>                 #
# Copyright (C) 2007 by sipwerk - All rights reserved.               #
#                                                                    #
# Please note that this software is under development and comes with #
# absolutely no warranty, to the extend permitted by applicable law. #
######################################################################

#echo $0

mf_rw () {
    cp $1 $1.cp
    rm $1
    mv $1.cp $1
}

mf_feedback () {
    echo "The Milkfish Router Services"
    echo ${1:-200 OK}
}

mf_logging () {
    $LOGGER -t $BASENAME $1
}


mf_printdaemon () {
    mf_feedback "#####################################################################"
    echo "# Note: The Milkfish Router Services depend on an online internet   #"
    echo "#       connection or valid route to the internet using a gateway.  #"
    echo "#       If no internet connection is available at start time the    #"
    echo "#       service process will background and retry later and so on.  #"
    echo "#       To stop the daemon you can execute the following command:   #"
    echo "#       milkfish_services stop                                      #" 
    echo "#################################### Copyright (C) 2007 by sipwerk ##"
    echo " "
    echo "Starting and daemonising..."
}


mf_settings_check () {
    CHECK=${2:-NULL}
    if [ $CHECK = "NULL" ]; then
     mf_feedback "ERROR: Necessary service setting not found: $1 - aborting." 
     exit 1
    fi
}


mf_switch_check () {
    case "$2" in
	on)
	    true
	;;
	off)
	    mf_feedback "INFO: $3 service is disabled - Current setting: $1=off - aborting."
	    [ $4 != "noexit" ] && exit 1
	;;
	*)
	    mf_feedback "ERROR: $3 service setting not found or invalid: $1 is not set 'on' nor 'off' - aborting." 
	    [ $4 != "noexit" ] && exit 1
	;;
    esac
}


mf_load_setting () {
    echo $(nvram get $1)
}


mf_save_setting () {
    nvram set $1
}


mf_ser_procs () {
    echo $(ps | grep openser | wc -l | awk '{print $1}')
}


mf_services_check () {
    if [ $(which curl) ] ; then
        mf_feedback "Checking router status at milkfish.org (https)..."
	curl -k -s https://$USERNAME:$PASSWORD@milkfish.org/register?system=all\&action=status\&routerid=$(nvram get milkfish_routerid)
 	
    else
        mf_feedback "Checking router status at milkfish.org (http)..."
	wget -O - http://$USERNAME:$PASSWORD@milkfish.org/register?system=all\&action=status\&routerid=$(nvram get milkfish_routerid)
 	
    fi
}

mf_router_status () {
    mf_feedback "Checking the local Router Status..."
    echo ""
    echo "SIP Transaction Statistics:"
    echo "$(openserctl fifo t_stats)"
    echo ""
    echo "NVRAM:"
    echo "$(nvram get milkfish_fromdomain | awk '{print "SIP_DOMAIN: " $1}';)"
    echo "$( nvram get openser_cfg | awk '{print "OPENSER_CFG: " $1}';)"
    echo ""
    echo "Processes (first receiver process per IP:port):"
    echo "$(openserctl ps | grep child=0\ sock=; openserctl ps | grep child=0\ sock=1; openserctl ps | grep child=0\ sock=2;)"
    echo ""
    echo "RTPproxy:"
    echo "$(ps | grep 'rtpproxy -l' | grep -v 'grep' | cut -f2 -d'l' | awk 'sub(" ","") {print $1 "  " $2}'; )"
    echo ""
    echo "IP addresses:"
    echo "$(ifconfig $(nvram get lan_ifname)|awk 'sub("inet addr:","") {print "Configured LAN: " $1}';)"

    if [ $(nvram get wan_proto) = pppoe ] ; then
	if [ "'ifconfig | grep ppp0'" ]; then
	    echo $(ifconfig ppp0 | awk 'sub("inet addr:","") {print "Configured WAN: " $1}')
	else echo "ppp0 interface not up"
        fi
    else
    	echo $(ifconfig $(nvram get wan_ifname)|awk 'sub("inet addr:","") {print "Configured WAN: " $1}')
    fi
    echo "Effective WAN: $(wget -O - http://checkip.sipwerk.com|sed s/[^0-9.]//g)"

    echo ""
    echo "SER Uptime:"
    echo "$(openserctl fifo uptime)"
    echo ""
}

mf_audit_activate () {
    if [ $(which curl) ] ; then
        mf_feedback "Activating the Milkfish Audit service (https)..."
	curl -k -s https://$USERNAME:$PASSWORD@milkfish.org/register?system=audit\&action=activate\&routerid=$(nvram get milkfish_routerid)
 	
    else
        mf_feedback "Activating the Milkfish Audit service (http)..."
	wget -O - http://$USERNAME:$PASSWORD@milkfish.org/register?system=audit\&action=activate\&routerid=$(nvram get milkfish_routerid)
 	
    fi
}

mf_audit_deactivate () {
    if [ $(which curl) ] ; then
        mf_feedback "DeActivating the Milkfish Audit service (https)..."
	curl -k -s https://$USERNAME:$PASSWORD@milkfish.org/register?system=audit\&action=deactivate\&routerid=$(nvram get milkfish_routerid)
 	
    else
        mf_feedback "DeActivating the Milkfish Audit service (http)..."
	wget -O - http://$USERNAME:$PASSWORD@milkfish.org/register?system=audit\&action=deactivate\&routerid=$(nvram get milkfish_routerid)
 	
    fi
}

mf_audit_results () {

    mf_feedback "Requesting Milkfish Audit results (http)..."
    wget -O - http://www.milkfish.org/audit/results/$( echo $(nvram get milkfish_routerid) | cut -c1-16 )-results.txt
}


mf_fwtest_curl () {
    mf_feedback "Starting Firewall Test Utility (https)..."
    curl -k -s https://firewalltest.milkfish.org/embedded_tcpscan
}


mf_fwtest_wget () {
    mf_feedback "Starting Firewall Test Utility (http)..."
    wget -O - http://firewalltest.milkfish.org/embedded_tcpscan
}


mf_echotest_curl () {
    mf_feedback "Starting Signaling Test Utility (https)..."
    curl -k -s https://echotest.milkfish.org/embedded_echotest
}


mf_echotest_wget () {
    mf_feedback "Starting Signaling Test Utility (http)..."
    wget -O - http://echotest.milkfish.org/embedded_echotest
}


mf_dynsip_update () {
   
   if [ $(nvram get wan_proto) = pppoe ] ; then
	if [ "'ifconfig | grep ppp0'" ]; then
	    CURRENTIP=$(ifconfig ppp0 | awk 'sub("inet addr:","") {print $1}')
	else CURRENTIP=$DYNSIPIP #echo "ppp0 interface not up"
        fi
   else
    	CURRENTIP=$(ifconfig $(nvram get wan_ifname)|awk 'sub("inet addr:","") {print $1}')
   fi

   #CURRENTIP=$(wget -O - http://checkip.sipwerk.com|sed s/[^0-9.]//g)

   case $1 in
    verbose)
      [ "$CURRENTIP" != "$DYNSIPIP" ] && {                           
       if [ $(which curl) ] ; then
        mf_feedback "Updating dynamic SIP service (https)..."
#	curl -k -s https://$USERNAME:$PASSWORD@dynsip.milkfish.org/nic/update?hostname=$DOMAIN
 	curl -k -s https://${DSUSERNAME:-$USERNAME}:${DSPASSWORD:-$PASSWORD}@${DYNSIPURL:-dynsip.milkfish.org/nic/update}?hostname=$DOMAIN
       else
        mf_feedback "Updating dynamic SIP service (http)..."
#	wget -O - http://$USERNAME:$PASSWORD@dynsip.milkfish.org/nic/update?hostname=$DOMAIN
 	wget -O - http://${DSUSERNAME:-$USERNAME}:${DSPASSWORD:-$PASSWORD}@${DYNSIPURL:-dynsip.milkfish.org/nic/update}?hostname=$DOMAIN
       fi
       DYNSIPIP=$CURRENTIP
      }
      ;;
    *)
      [ "$CURRENTIP" != "$DYNSIPIP" ] && {                           
       if [ $(which curl) ] ; then
        mf_logging "Updating dynamic SIP service (https)..."
 #	curl -k -s https://$USERNAME:$PASSWORD@dynsip.milkfish.org/nic/update?hostname=$DOMAIN > /dev/null 2>&1
	curl -k -s https://${DSUSERNAME:-$USERNAME}:${DSPASSWORD:-$PASSWORD}@${DYNSIPURL:-dynsip.milkfish.org/nic/update}?hostname=$DOMAIN > /dev/null 2>&1
       else
        mf_logging "Updating dynamic SIP service (http)..."
 #	wget -O - http://$USERNAME:$PASSWORD@dynsip.milkfish.org/nic/update?hostname=$DOMAIN > /dev/null 2>&1
	wget -O - http://${DSUSERNAME:-$USERNAME}:${DSPASSWORD:-$PASSWORD}@${DYNSIPURL:-dynsip.milkfish.org/nic/update}?hostname=$DOMAIN > /dev/null 2>&1
       fi
       DYNSIPIP=$CURRENTIP
      }
      ;;
    esac
}


mf_ppptime () {

    NOWTIME=$(date +'%H')
    PPPTIME=$(mf_load_setting milkfish_ppptime)
    SYNCHRONE=FALSE
    if [ $PPPTIME ] && [ $PPPTIME != "off" ]; then
	#echo "Last: $TIMESTAMP"
	#echo "Now: $NOWTIME"
	#echo "Preset: $PPPTIME"
	if [ $NOWTIME -eq $PPPTIME ] ; then
	
    	    if [ -r $TIMESTAMPFILE ] ; then
		TIMESTAMP=$(tail -1 $TIMESTAMPFILE  | awk -F: '{print $1}')
        	if [ $TIMESTAMP -eq $PPPTIME ] ; then
        	    #echo "Last reconnect was in the same hour of day..."
            	    date +'%H:%M:%S %y-%m-%d - reconnect cancelled' >> $TIMESTAMPFILE
		    SYNCHRONE=TRUE
		else
		    date +'%H:%M:%S %y-%m-%d - commencing reconnect' >> $TIMESTAMPFILE
		fi
    	    fi
	    
	    if [ $SYNCHRONE = "FALSE" ] ; then
        	#echo "It is the right hour for a reconnect..."
		mf_feedback "INFO: Commencing PPPoE reconnect..."
        	milkfish_services audit router && \
        	if [ -r $OPENSERPIDFILE ] ; then
	    	    milkfish_services audit openser && \
    	    	    openserctl stop
        	fi			
        	ifdown wan && \
		sleep 5 && \
    	        ifup wan && \
		sleep 30
	    fi
        else
	    mf_feedback "INFO: Not the right hour for a PPPoE reconnect."
	fi
    else
        mf_feedback "ERROR: Automatic PPPoE reconnect disabled or no preset reconnection time found."
    fi
}

mf_routeraudit_curl () {
    if [ -z $1 ] ; then
	mf_feedback "Updating router audit (https)..."
    fi
    PUPTIME=$(( $(cat /proc/uptime | awk -F . '{print $1}';) / 3600 ))
    DOTPUPTIME=$(( $(( $(cat /proc/uptime | awk -F . '{print $1}';) - $(( $PUPTIME * 3600 )) )) / 36 ))
    curl -k -s https://$USERNAME:$PASSWORD@audit.milkfish.org/audits?uptime=\
$(cat /proc/uptime| awk 'sub(" ", "_") {print}';)\&btime=$(cat /proc/stat | grep btime | awk '{print $2}';)\
_$(date +%s)\&puptime=$PUPTIME.$DOTPUPTIME\
\&routerid=$(nvram get milkfish_username)_$(nvram get milkfish_routerid)$(md5sum $0 | awk '{print $1}')
}


mf_routeraudit_wget () {
    if [ -z $1 ] ; then
	mf_feedback "Updating router audit (http)..."
    fi
    PUPTIME=$(( $(cat /proc/uptime | awk -F . '{print $1}';) / 3600 ))
    DOTPUPTIME=$(( $(( $(cat /proc/uptime | awk -F . '{print $1}';) - $(( $PUPTIME * 3600 )) )) / 36 ))
    wget -O - http://$USERNAME:$PASSWORD@audit.milkfish.org/audits?uptime=\
$(cat /proc/uptime| awk 'sub(" ", "_") {print}';)\&btime=$(cat /proc/stat | grep btime | awk '{print $2}';)\
_$(date +%s)\&puptime=$PUPTIME.$DOTPUPTIME\
\&routerid=$(nvram get milkfish_username)_$(nvram get milkfish_routerid)$(md5sum $0 | awk '{print $1}')
}


mf_seraudit_curl () {
    AUDITPRCS=$(ps | grep openser | grep -v "milkfish_services" | wc -l | awk '{print $1}' )
    #echo $AUDITPRCS
    if [ $AUDITPRCS -gt 4 ]; then
        mf_feedback "Updating $SERNAME audit (https)..."
	ROUTERUPTIME=$(cat /proc/uptime| awk '{print $1}' | awk -F. '{print $1}';)
	SERFIFOUPTIME=$(openserctl fifo uptime | grep "Up time" | awk '{print $3}')
	if [ $SERFIFOUPTIME -gt $ROUTERUPTIME ] ; then
	    SERFIFOUPTIME=$ROUTERUPTIME
	fi
	SERPUPTIME=$(( $SERFIFOUPTIME / 3600 ))
	DOTSERPUPTIME=$(( $(( $SERFIFOUPTIME - $(( $SERPUPTIME * 3600 )) )) / 36 ))
	curl -k -s https://$USERNAME:$PASSWORD@audit.milkfish.org/audits?seruptime=\
$(openserctl fifo uptime | grep "Up time" | awk '{print $3}')_$(cat /proc/uptime| awk '{print $1}';)\&serstats=\
$(openserctl fifo t_stats | grep status | awk -Fs '{print $3}' | awk -F, '{print $1 ": " $2 ": " $3 ": " $4 ": " $5}' |\
awk -F": " '{print $2 "_" $4 "_" $6 "_" $8 "_" $10}' | awk '{print $1}')_$(date +%s)\&\
serpuptime=$SERPUPTIME.$DOTSERPUPTIME\&routerid=$(nvram get milkfish_username)\
_$(nvram get milkfish_routerid)$(md5sum $0 | awk '{print $1}')
    else
	mf_feedback "ERROR: No running $SERNAME found - no $SERNAME auditing possible - abort"
	#exit 1
    fi
}


mf_seraudit_wget () {
    AUDITPRCS=$(ps | grep openser | grep -v "milkfish_services" | wc -l | awk '{print $1}' )
    if [ $AUDITPRCS -gt 4 ]; then
        mf_feedback "Updating $SERNAME audit (http)..."
	ROUTERUPTIME=$(cat /proc/uptime| awk '{print $1}' | awk -F. '{print $1}';)
	SERFIFOUPTIME=$(openserctl fifo uptime | grep "Up time" | awk '{print $3}')
	if [ $SERFIFOUPTIME -gt $ROUTERUPTIME ] ; then
	    SERFIFOUPTIME=$ROUTERUPTIME
	fi
	SERPUPTIME=$(( $SERFIFOUPTIME / 3600 ))
	DOTSERPUPTIME=$(( $(( $SERFIFOUPTIME - $(( $SERPUPTIME * 3600 )) )) / 36 ))
	wget -O - http://$USERNAME:$PASSWORD@audit.milkfish.org/audits?seruptime=\
$(openserctl fifo uptime | grep "Up time" | awk '{print $3}')_$ROUTERUPTIME\&serstats=\
$(openserctl fifo t_stats | grep status | awk -Fs '{print $3}' | awk -F, '{print $1 ": " $2 ": " $3 ": " $4 ": " $5}' |\
awk -F": " '{print $2 "_" $4 "_" $6 "_" $8 "_" $10}' | awk '{print $1}')_$(date +%s)\&\
serpuptime=$SERPUPTIME.$DOTSERPUPTIME\&routerid=$(nvram get milkfish_username)\
_$(nvram get milkfish_routerid)$(md5sum $0 | awk '{print $1}')
    else
	mf_feedback "ERROR: No running $SERNAME found - no $SERNAME auditing"
	#exit 1
    fi
}

mf_ddns_wget () {
  #Credits: Based on a script by mbm of openwrt.org
  DDNSACACHE=$(wget -O - http://checkip.sipwerk.com|sed s/[^0-9.]//g)
   [ "$DDNSDCACHE" != "$DDNSACACHE" ] && {                           
      wget -O /dev/null http://$DDNSUSERNM:$DDNSPASSWD@$DDNSSERVER/nic/update?hostname=$DDNSHOSTNM &&
      DDNSDCACHE=$DDNSACACHE
   }                     
}

mf_backup () {

    milkfish_services audit router && \
    if [ -r $OPENSERPIDFILE ] ; then
        milkfish_services audit openser noexit && \
        openserctl stop
    fi			
    mkdir -p /jffs
    mount /dev/mtdblock/4 /jffs
    cd /jffs
    tar -czvf /tmp/backup.tar.gz . && \
    echo "Done." && \
    cd / && \
    umount /jffs && \
    sleep 30 && \
    openserctl start && \
    sleep 10 && \
    milkfish_services audit openser noexit
    
}

mf_phonebook () {
    mf_feedback "=============================="
    echo "LOCAL CONTACT LIST" &&\
    openserctl ul show | grep 'aor   :' | awk '{print $3}' | sed -e "s/^\('\)/==============================\n\1/" | awk 'sub("","") {print}' | sed -e "s/'//" | sed -e "s/'//" | awk '{print $1}' &&\
    echo "==============================" &&\
    echo &&\
    echo "==============================" &&\
    echo "DETAILED LOCAL PHONEBOOK" &&\
    openserctl ul show | grep "^domain    :\|^aor       :\|^Contact\|^Expires\|^Call-ID\|^User-Agent\|^State" | sed -e "s/^\(domain \)/==============================\n\1/" | awk 'sub("","") {print}' &&\
    echo "==============================" &&\
    echo " "
}

mf_phonebook_html () {
    echo "<h3>Local contact list</h3>" &&\
    openserctl ul show | grep 'aor   :' | awk '{print $3}' | sed -e "s/^\('\)/==============================<br>\1/" | awk 'sub("","<br>") {print}' | sed -e "s/'//" | sed -e "s/'//" | awk '{print $1}' &&\
    echo "<br>==============================<br><br><br>" &&\
    echo "<h3>Detailed local phonebook</h3>" &&\
    openserctl ul show | grep "^domain    :\|^aor       :\|^Contact\|^Expires\|^Call-ID\|^User-Agent\|^State" | sed -e "s/^\(domain \)/==============================<br>\1/" | awk 'sub("","<br>") {print}' &&\
    echo "<br>==============================<br>"
}

mf_phonebook_htmltable () {
    echo "<table border=1><tr><td><br><h3>Local Contact List</h3>" &&\
    openserctl ul show | grep 'aor   :' | awk '{print $3}' | sed -e "s/^\('\)/<\/td><\/tr><tr><td>\1/" | awk 'sub("","<br>") {print}' | sed -e "s/'//" | sed -e "s/'//" | awk '{print $1}' &&\
    echo "</td></tr></table><br><br><br>" &&\
    echo "<table border=1><tr><td><br><h3>Detailed Local Phonebook</h3>" &&\
    openserctl ul show | grep "^domain    :\|^aor       :\|^Contact\|^Expires\|^Call-ID\|^User-Agent\|^State" | sed -e "s/^\(domain \)/<\/td><\/tr><tr><td>\1/" | awk 'sub("","<br>") {print}' &&\
    echo "</td></tr></table><br>"
}

mf_sipdb_adduser () {
    dbtextctl add $1 $2 $(nvram get milkfish_fromdomain) &&\
    milkfish_services audit openser noexit &&\
    openserctl stop && sleep 3 && openserctl start
}

mf_sipdb_rmuser () {
    dbtextctl rm $1 | grep USER &&\
    milkfish_services audit openser noexit &&\
    openserctl stop && sleep 3 && openserctl start
}

#exists /var/openser/dbtext/aliases  && ALIASES_FILE=/var/openser/dbtext/aliases || ALIASES_FILE=/etc/openser/dbtext/aliases
#exists /var/openser/dbtext/location  && LOCATION_FILE=/var/openser/dbtext/location || LOCATION_FILE=/etc/openser/dbtext/location
#exists /var/openser/dbtext/subscriber  && SUBSCRIBER_FILE=/var/openser/dbtext/subscriber || SUBSCRIBER_FILE=/etc/openser/dbtext/subscriber

mf_sipdb_addalias () {
    [ -e /var/openser/dbtext/aliases ] && ALIASES_FILE=/var/openser/dbtext/aliases || ALIASES_FILE=/etc/openser/dbtext/aliases
    echo "$1::$2::0:1.00:Milkfish-Alias:42::0:0:128:Milkfish-Router:" >> "$ALIASES_FILE"
    milkfish_services audit openser noexit &&\
    openserctl stop && sleep 3 && openserctl start
}

mf_sipdb_rmalias () {
    [ -e /var/openser/dbtext/aliases ] && ALIASES_FILE=/var/openser/dbtext/aliases || ALIASES_FILE=/etc/openser/dbtext/aliases
    touch "$ALIASES_FILE".tmp
    grep -v ^"$1" "$ALIASES_FILE" >> "$ALIASES_FILE".tmp
    mv "$ALIASES_FILE".tmp "$ALIASES_FILE"
    milkfish_services audit openser noexit &&\
    openserctl stop && sleep 3 && openserctl start
}

mf_sipdb_rmlocation () {
    openserctl ul rm $1 &&\
    milkfish_services audit openser noexit &&\
    openserctl stop && sleep 3 && openserctl start
}

mf_sipdb_flush () {
    [ -d /var/openser ] &&\
    milkfish_services openserctl stop &&\
    mf_feedback "Flushing volatile SIP database..." &&\
    cp /etc/openser/dbtext/uri.empty /var/openser/dbtext/uri &&\
    cp /etc/openser/dbtext/grp.empty /var/openser/dbtext/grp &&\
    cp /etc/openser/dbtext/aliases.empty /var/openser/dbtext/aliases &&\
    cp /etc/openser/dbtext/location.empty /var/openser/dbtext/location &&\
    cp /etc/openser/dbtext/subscriber.empty /var/openser/dbtext/subscriber &&\
    #cp /etc/openser/dbtext/version.empty /var/openser/dbtext/version &&\
    sleep 5 && openserctl start
}

mf_sipdb_restore () {
    [ -d /var/openser ] &&\
    milkfish_services openserctl stop &&\
    mf_feedback "Restoring persistent SIP database..." &&\
    cp /etc/openser/dbtext/uri /var/openser/dbtext/ &&\
    cp /etc/openser/dbtext/grp /var/openser/dbtext/ &&\
    cp /etc/openser/dbtext/aliases /var/openser/dbtext/ &&\
    cp /etc/openser/dbtext/location /var/openser/dbtext/ &&\
    cp /etc/openser/dbtext/subscriber /var/openser/dbtext/ &&\
    cp /etc/openser/dbtext/version /var/openser/dbtext/ &&\
    sleep 5 && openserctl start
}

mf_sipdb_store () {
    [ -d /var/openser ] && mf_feedback "Storing volatile SIP DB to Flash..." && cp -r /var/openser /etc/
}

mf_sipdb_flushnv () {
    mf_feedback "Flushing SIP subscriber database in NVRAM..."
    nvram set milkfish_subscriber= && echo "Done."
    mf_feedback "Flushing SIP aliases database in NVRAM..."
    nvram set milkfish_aliases= && echo "Done."
}

mf_sipdb_restorenvdd () {
    [ -d /var/openser/dbtext ] &&\
    mf_feedback "Restoring SIP ddsubscriber database from NVRAM..."
    if [ ! -z "$(nvram get milkfish_ddsubscribers)" ]; then 
	echo "#!/bin/sh" > /tmp/restorenvdd.sh
	nvram get milkfish_ddsubscribers | tr ' ' '\n' | awk -F : '{print "dbtextctl add " $1 " " $2 " " "nomail"}'
	nvram get milkfish_ddsubscribers | tr ' ' '\n' | awk -F : '{print "dbtextctl add " $1 " " $2 " " "nomail"}' >> /tmp/restorenvdd.sh
	[ -e /tmp/restorenvdd.sh ] && chmod +x /tmp/restorenvdd.sh && /tmp/restorenvdd.sh && rm /tmp/restorenvdd.sh
	echo "Done."
    else
	echo "Empty."
    fi
    mf_feedback "Restoring SIP ddaliases database from NVRAM..."
    if [ ! -z "$(nvram get milkfish_ddaliases)" ]; then 
	echo "#!/bin/sh" > /tmp/restorenvdd.sh
	[ -e /var/openser/dbtext/aliases ] && nvram get milkfish_ddaliases | tr ' ' '\n' | awk -F : '{print "[ -z \"$(grep " $1 ": /var/openser/dbtext/aliases)\" ] && echo \"" $1 "::sip\\:" $2 "::0:1.00:Milkfish-Alias:42::0:0:128:Milkfish-Router:\" >> /var/openser/dbtext/aliases" }'
	[ -e /var/openser/dbtext/aliases ] && nvram get milkfish_ddaliases | tr ' ' '\n' | awk -F : '{print "[ -z \"$(grep " $1 ": /var/openser/dbtext/aliases)\" ] && echo \"" $1 "::sip\\:" $2 "::0:1.00:Milkfish-Alias:42::0:0:128:Milkfish-Router:\" >> /var/openser/dbtext/aliases" }' >> /tmp/restorenvdd.sh
	[ -e /tmp/restorenvdd.sh ] && chmod +x /tmp/restorenvdd.sh && /tmp/restorenvdd.sh && rm /tmp/restorenvdd.sh
	echo "Done."
    else
	echo "Empty."
    fi
    #milkfish_services openserctl restart
}

mf_sipdb_restorenv () {
    [ -d /var/openser/dbtext ] &&\
    mf_feedback "Restoring SIP subscriber database from NVRAM..."
    if [ ! -z "$(nvram get milkfish_subscriber)" ]; then 
	nvram get milkfish_subscriber | sed -e 's/<+>/\n/g;s/<->/ /g' > /var/openser/dbtext/subscriber
	echo "Done."
    else
	echo "Empty."
    fi
    mf_feedback "Restoring SIP aliases database from NVRAM..."
    if [ ! -z "$(nvram get milkfish_aliases)" ]; then 
	nvram get milkfish_aliases | sed -e 's/<+>/\n/g;s/<->/ /g' > /var/openser/dbtext/aliases
	echo "Done."
    else
	echo "Empty."
    fi
    #milkfish_services openserctl restart
}

mf_sipdb_storenv () {
    [ -e /var/openser/dbtext/subscriber ] && mf_feedback "Storing volatile SIP DB subscribers to NVRAM..." &&\
    nvram set milkfish_subscriber=$(cat /var/openser/dbtext/subscriber | head -n11 | sed -e ':a;N;$!ba;s/\n/<+>/g;s/ /<->/g') &&\
    echo "Done."
    [ -e /var/openser/dbtext/aliases ] && mf_feedback "Storing volatile SIP DB aliases to NVRAM..." &&\
    nvram set milkfish_aliases=$(cat /var/openser/dbtext/aliases | head -n11 | sed -e ':a;N;$!ba;s/\n/<+>/g;s/ /<->/g') &&\
    echo "Done."
}

mf_ddactive () {
    DDACTIVEFILE=/tmp/ddactive.txt
    DDACTIVEFILE2=/tmp/ddactive2.txt
    DDACTIVEFILE3=/tmp/ddactive3.txt
    [ -e $DDACTIVEFILE ] && rm $DDACTIVEFILE
    [ -e $DDACTIVEFILE2 ] && rm $DDACTIVEFILE2
    [ -e $DDACTIVEFILE3 ] && rm $DDACTIVEFILE3
    openserctl ul show | grep "^aor       :\|^Contact\|^User-Agent" | sed -e "s/sip://" | awk -F : '{print $2 " " $3}' | awk -F "'" '{print $2}' | sed -e "s/ /_/g;s/;/\\\;/g" > $DDACTIVEFILE
    LINES=$(wc -l $DDACTIVEFILE | awk '{print $1}')
    CONTACTS=$(( $LINES / 3 ))
    #echo $CONTACTS
    #cat $DDACTIVEFILE
    while [ $CONTACTS -gt 0 ];
    do {
	#echo $CONTACTS
	#echo $(( $CONTACTS *3 - 2 ))
	SIPUSER=$(head -n$(( $CONTACTS * 3 - 2 )) $DDACTIVEFILE | tail -n1)
	SIPCONTACT=$(head -n$(( $CONTACTS * 3 - 1 )) $DDACTIVEFILE | tail -n1)
	SIPAGENT=$(head -n$(( $CONTACTS * 3 - 0 )) $DDACTIVEFILE | tail -n1)
	echo $SIPUSER:$SIPCONTACT:$SIPAGENT >> $DDACTIVEFILE2
	#echo $SIPCONTACT
	#echo $SIPAGENT
	#echo $CONTACTS
	let "CONTACTS--"
	}
    done;
    #cat $DDACTIVEFILE2
    cat $DDACTIVEFILE2 | grep -v Milkfish-Router >> $DDACTIVEFILE3
    #cat $DDACTIVEFILE3 | wc -l
    #cat $DDACTIVEFILE3
    NVRAMNUM=$(cat $DDACTIVEFILE3 | wc -l)
    NVRAMSTRING=$(cat $DDACTIVEFILE3 | tr '\n' ' ')
    #echo $NVRAMNUM
    #echo $NVRAMSTRING
    #NVRAMSTRING2=$NVRAMSTRING
    # | sed -e "s/ /\\\ /g")
    nvram set milkfish_ddactive="$NVRAMSTRING"
    nvram set milkfish_ddactivenum=$NVRAMNUM
    [ -e $DDACTIVEFILE ] && rm $DDACTIVEFILE
    [ -e $DDACTIVEFILE2 ] && rm $DDACTIVEFILE2
    [ -e $DDACTIVEFILE3 ] && rm $DDACTIVEFILE3
}


mf_imdb_store () {
    [ -d /var/jabberd ] && mf_feedback "Storing volatile IM DB to Flash..." && cp -r /var/jabberd /etc/
}

mf_reboot () {
    #milkfish_services store im &&\
    milkfish_services audit router &&\
    milkfish_services audit openser noexit &&\
    reboot
}

mf_sip_restart () {
    #milkfish_services store im &&\
    milkfish_services audit openser noexit &&\
    $SIPSTARTUPSCRIPT restart
}

mf_sip_stop () {
    #milkfish_services store im &&\
    milkfish_services audit openser noexit &&\
    $SIPSTARTUPSCRIPT stop
}

mf_sip_start () {
    #milkfish_services store im &&\
    milkfish_services audit openser noexit &&\
    $SIPSTARTUPSCRIPT start
}

mf_openserctl () {
    openserctl $1
}



mf_simpledd () {

    echo ":t_uac_dlg:openser_fifo_replies
MESSAGE
$1
.
From: sip:mf@$(nvram get milkfish_fromdomain)
To: $1
foo: bar_special_header
x: y
p_header: p_value
Contact: <sip:devnull@$(nvram get milkfish_fromdomain):9>
Content-Type: text/plain; charset=UTF-8" > /tmp/msg;
    echo "." >> /tmp/msg;
    cat /tmp/sipmessage >> /tmp/msg
    echo "." >> /tmp/msg;
    echo "EOF" >> /tmp/msg;
    #cat /tmp/msg;
    mkfifo -m 666 /tmp/openser_fifo_replies;
    cat /tmp/msg > /tmp/openser_fifo;
    sleep 2;
    #cat /tmp/openser_fifo_replies # > /dev/null;
    rm /tmp/openser_fifo_replies;

}

mf_simple () {

    echo ":t_uac_dlg:openser_fifo_replies
MESSAGE
$1
.
From: sip:mf@$(nvram get sip_domain)
To: $1
foo: bar_special_header
x: y
p_header: p_value
Contact: <sip:devnull@$(nvram get sip_domain):9>
Content-Type: text/plain; charset=UTF-8" > /tmp/msg;
    echo "." >> /tmp/msg;
    echo "$2 $3 $4 $5 $6 $7 $8 $9 $10 $11 $13 $14 $15" >> /tmp/msg;
    echo "." >> /tmp/msg;
    echo "EOF" >> /tmp/msg;
    #cat /tmp/msg;
    mkfifo -m 666 /tmp/openser_fifo_replies;
    cat /tmp/msg > /tmp/openser_fifo;
    sleep 2;
    #cat /tmp/openser_fifo_replies # > /dev/null;
    rm /tmp/openser_fifo_replies;

}


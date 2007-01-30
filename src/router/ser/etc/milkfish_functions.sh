#!/bin/sh
# The Milkfish Router Services - Functions Library
# Author: fronce@users.berlios.de
# Website: milkfish.org
# License: GPL
# Date: 061005

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
    echo "#####################################################################"
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


mf_services_status () {
    if [ $(which curl) ] ; then
        mf_feedback "Checking router status at milkfish.org (https)..."
	curl -k -s https://$USERNAME:$PASSWORD@milkfish.org/register?system=all\&action=status\&routerid=$(nvram get milkfish_routerid)
 	
    else
        mf_feedback "Checking router status at milkfish.org (http)..."
	wget -O - http://$USERNAME:$PASSWORD@milkfish.org/register?system=all\&action=status\&routerid=$(nvram get milkfish_routerid)
 	
    fi
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


mf_dynsip_update () {
   CURRENTIP=$(wget -O - http://checkip.dyndns.org|sed s/[^0-9.]//g)
   case $1 in
    verbose)
      [ "$CURRENTIP" != "$DYNSIPIP" ] && {                           
       if [ $(which curl) ] ; then
        mf_feedback "Updating the homesip.net dynamic SIP service (https)..."
	curl -k -s https://$USERNAME:$PASSWORD@dynsip.milkfish.org/nic/update?hostname=$DOMAIN
 	
       else
        mf_feedback "Updating the homesip.net dynamic SIP service (http)..."
	wget -O - http://$USERNAME:$PASSWORD@dynsip.milkfish.org/nic/update?hostname=$DOMAIN
 	
       fi
       DYNSIPIP=$CURRENTIP
      }
      ;;
    *)
      [ "$CURRENTIP" != "$DYNSIPIP" ] && {                           
       if [ $(which curl) ] ; then
        mf_logging "Updating the homesip.net dynamic SIP service (https)..."
 	curl -k -s https://$USERNAME:$PASSWORD@dynsip.milkfish.org/nic/update?hostname=$DOMAIN > /dev/null 2>&1
       else
        mf_logging "Updating the homesip.net dynamic SIP service (http)..."
 	wget -O - http://$USERNAME:$PASSWORD@dynsip.milkfish.org/nic/update?hostname=$DOMAIN > /dev/null 2>&1
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
  DDNSACACHE=$(wget -O - http://checkip.dyndns.org|sed s/[^0-9.]//g)
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


mf_sipdb_adduser () {
    dbtextctl add $1 $2 $(nvram get sip_domain) &&\
    milkfish_services audit openser noexit &&\
    openserctl stop && sleep 3 && openserctl start
}

mf_sipdb_rmuser () {
    dbtextctl rm $1 | grep USER &&\
    milkfish_services audit openser noexit &&\
    openserctl stop && sleep 3 && openserctl start
}

#exists /tmp/openser/dbtext/aliases  && ALIASES_FILE=/tmp/openser/dbtext/aliases || ALIASES_FILE=/etc/openser/dbtext/aliases
#exists /tmp/openser/dbtext/location  && LOCATION_FILE=/tmp/openser/dbtext/location || LOCATION_FILE=/etc/openser/dbtext/location
#exists /tmp/openser/dbtext/subscriber  && SUBSCRIBER_FILE=/tmp/openser/dbtext/subscriber || SUBSCRIBER_FILE=/etc/openser/dbtext/subscriber

mf_sipdb_addalias () {
    [ -e /tmp/openser/dbtext/aliases ] && ALIASES_FILE=/tmp/openser/dbtext/aliases || ALIASES_FILE=/etc/openser/dbtext/aliases
    echo "$1::$2::0:1.00:Milkfish-Alias:42::0:0:128:Milkfish-Router:" >> "$ALIASES_FILE"
    milkfish_services audit openser noexit &&\
    openserctl stop && sleep 3 && openserctl start
}

mf_sipdb_rmalias () {
    [ -e /tmp/openser/dbtext/aliases ] && ALIASES_FILE=/tmp/openser/dbtext/aliases || ALIASES_FILE=/etc/openser/dbtext/aliases
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
    cp /etc/openser/dbtext/version.empty /var/openser/dbtext/version &&\
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
    [ -d /var/openser ] && mf_feedback "Storing volatile SIP DB to flash..." && cp -r /var/openser /etc/
}

mf_imdb_store () {
    [ -d /var/jabberd ] && mf_feedback "Storing volatile IM DB to flash..." && cp -r /var/jabberd /etc/
}

mf_reboot () {
    #milkfish_services store im &&\
    milkfish_services audit router &&\
    milkfish_services audit openser noexit &&\
    reboot
}

mf_openserctl () {
    openserctl $1
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

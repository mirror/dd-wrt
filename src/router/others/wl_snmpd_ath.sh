#!/bin/sh

place=".1.3.6.1.4.1.2021.255"

refresh() {

  id=1
  lastid=0
  rm -rf /tmp/snmp_cache  
  for mac in $(wl_atheros assoclist | cut -d" " -f2)
  do
    if test $lastid -eq 0
    then
      getnext_1361412021255="$place.3.54.1.3.32.1.1.1"
      getnext_1361412021255354133211="$place.3.54.1.3.32.1.1.1"
      getnext_1361412021255354133214="$place.3.54.1.3.32.1.4.1"
      getnext_13614120212553541332113="$place.3.54.1.3.32.1.13.1"
      getnext_13614120212553541332126="$place.3.54.1.3.32.1.26.1"
      getnext_13614120212553541332127="$place.3.54.1.3.32.1.27.1"
      getnext_13614120212553541332128="$place.3.54.1.3.32.1.28.1"
      getnext_13614120212553541332129="$place.3.54.1.3.32.1.29.1"
      getnext_13614120212553541332130="$place.3.54.1.3.32.1.30.1"
      getnext_13614120212553541332131="$place.3.54.1.3.32.1.31.1"
      getnext_13614120212553541332132="$place.3.54.1.3.32.1.32.1"
    else
      eval getnext_1361412021255354133211${lastid}="$place.3.54.1.3.32.1.1.$id"
      eval getnext_1361412021255354133214${lastid}="$place.3.54.1.3.32.1.4.$id"
      eval getnext_13614120212553541332113${lastid}="$place.3.54.1.3.32.1.13.$id"
      eval getnext_13614120212553541332126${lastid}="$place.3.54.1.3.32.1.26.$id"
      eval getnext_13614120212553541332127${lastid}="$place.3.54.1.3.32.1.27.$id"
      eval getnext_13614120212553541332128${lastid}="$place.3.54.1.3.32.1.28.$id"
      eval getnext_13614120212553541332129${lastid}="$place.3.54.1.3.32.1.29.$id"
      eval getnext_13614120212553541332130${lastid}="$place.3.54.1.3.32.1.30.$id"
      eval getnext_13614120212553541332131${lastid}="$place.3.54.1.3.32.1.31.$id"
      eval getnext_13614120212553541332132${lastid}="$place.3.54.1.3.32.1.32.$id"
    fi
  
    rssi=$(wl_atheros rssi $mac | cut -d" " -f3)
    noise_reference=$(wl_atheros noise $mac | cut -d" " -f3)
    uptime=$(wl_atheros uptime $mac | cut -d" " -f3)
    ifname=$(wl_atheros ifname $mac | cut -d" " -f3)
    rxrate=$(wl_atheros rxrate $mac | cut -d" " -f3)
    txrate=$(wl_atheros txrate $mac | cut -d" " -f3)
    uptimestr=$(wl_atheros uptimestr $mac | cut -d" " -f3)
    eap_identity="NONE";
    if test $rssi -eq 0
    then
      snr=0
    else
      let snr=-1*$noise_reference+$rssi
    fi
    if test -e "/tmp/eap_identities/$mac"
    then
	eap_identity=`cat /tmp/eap_identities/$mac`
    fi
    mac=$(echo $mac | tr : ' ')
    eval value_1361412021255354133211${id}=$id;
    eval type_1361412021255354133211${id}='integer';
    eval value_1361412021255354133214${id}='$mac';
    eval type_1361412021255354133214${id}='string';
    eval value_13614120212553541332113${id}=$noise_reference;
    eval type_13614120212553541332113${id}='integer';
    eval value_13614120212553541332126${id}=$snr;
    eval type_13614120212553541332126${id}='integer';
    eval value_13614120212553541332127${id}=$uptime;
    eval type_13614120212553541332127${id}='integer';
    eval value_13614120212553541332128${id}=$ifname;
    eval type_13614120212553541332128${id}='string';
    eval value_13614120212553541332129${id}=$eap_identity;
    eval type_13614120212553541332129${id}='string';
    eval value_13614120212553541332130${id}=$rxrate;
    eval type_13614120212553541332130${id}='integer';
    eval value_13614120212553541332131${id}=$txrate;
    eval type_13614120212553541332131${id}='integer';
    eval value_13614120212553541332132${id}=$uptimestr;
    eval type_13614120212553541332132${id}='string';

    lastid=$id
    let id=$id+1
  
  done

  if test $lastid -ne 0
  then
    eval getnext_1361412021255354133211${lastid}="$place.3.54.1.3.32.1.4.1"
    eval getnext_1361412021255354133214${lastid}="$place.3.54.1.3.32.1.13.1"
    eval getnext_13614120212553541332113${lastid}="$place.3.54.1.3.32.1.26.1"
    eval getnext_13614120212553541332126${lastid}="$place.3.54.1.3.32.1.27.1"
    eval getnext_13614120212553541332127${lastid}="$place.3.54.1.3.32.1.28.1"
    eval getnext_13614120212553541332128${lastid}="$place.3.54.1.3.32.1.29.1"
    eval getnext_13614120212553541332129${lastid}="$place.3.54.1.3.32.1.30.1"
    eval getnext_13614120212553541332130${lastid}="$place.3.54.1.3.32.1.31.1"
    eval getnext_13614120212553541332131${lastid}="$place.3.54.1.3.32.1.32.1"
    eval getnext_13614120212553541332132${lastid}="NONE"
  fi
} 

LASTREFRESH=0

while read CMD
do
  case "$CMD" in
    PING)
      echo PONG
      continue 
      ;;
    getnext)
      read REQ
      let REFRESH=$(date +%s)-$LASTREFRESH
      if test $REFRESH -gt 30
      then
        LASTREFRESH=$(date +%s)
        refresh
      fi
      
      oid=$(echo $REQ | tr -d .) 
      eval ret=\$getnext_${oid}
      if test "x$ret" = "xNONE"
      then
        echo NONE
        continue 
      fi 
      ;;
    *)
      read REQ
      if test "x$REQ" = "x$place"
      then
        echo NONE
        continue 
      else
        ret=$REQ
      fi
      ;;
  esac

  oid=$(echo $ret | tr -d .) 
  if eval test "x\$type_${oid}" != "x"
  then
    echo $ret
    eval echo "\$type_${oid}"
    eval echo "\$value_${oid}"
  else
    echo NONE
  fi

done

#!/bin/sh

place=".1.3.6.1.4.1.2021.255.3.54.1.3.32.1"

refresh() {

  id=1
  lastid=0
  
  for mac in $(wl_atheros assoclist | cut -d" " -f2)
  do
    if test $lastid -eq 0
    then
      getnext_1361412021255="$place.1.1"
      getnext_1="$place.1.1"
      getnext_4="$place.4.1"
      getnext_13="$place.13.1"
      getnext_26="$place.26.1"
      getnext_27="$place.27.1"
      getnext_28="$place.28.1"
      getnext_29="$place.29.1"
      getnext_30="$place.30.1"
      getnext_31="$place.31.1"
      getnext_32="$place.32.1"
    else
      eval getnext_1${lastid}="$place.1.$id"
      eval getnext_4${lastid}="$place.4.$id"
      eval getnext_13${lastid}="$place.13.$id"
      eval getnext_26${lastid}="$place.26.$id"
      eval getnext_27${lastid}="$place.27.$id"
      eval getnext_28${lastid}="$place.28.$id"
      eval getnext_29${lastid}="$place.29.$id"
      eval getnext_30${lastid}="$place.30.$id"
      eval getnext_31${lastid}="$place.31.$id"
      eval getnext_32${lastid}="$place.32.$id"
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
    mac=$(echo $mac | tr : ' ')
    if test -e "/tmp/eap_identities/$mac"
    then
	eap_identity=`cat /tmp/eap_identities/$mac`
    fi
  
    eval value_1${id}=$id;
    eval type_1${id}='integer';
    eval value_4${id}='$mac';
    eval type_4${id}='octet';
    eval value_13${id}=$noise_reference;
    eval type_13${id}='integer';
    eval value_26${id}=$snr;
    eval type_26${id}='integer';
    eval value_27${id}=$uptime;
    eval type_27${id}='integer';
    eval value_28${id}=$ifname;
    eval type_28${id}='octet';
    eval value_29${id}=$eap_identity;
    eval type_29${id}='octet';
    eval value_30${id}=$rxrate;
    eval type_30${id}='integer';
    eval value_31${id}=$txrate;
    eval type_31${id}='integer';
    eval value_32${id}=$uptimestr;
    eval type_32${id}='octet';

    lastid=$id
    let id=$id+1
  
  done

  if test $lastid -ne 0
  then
    eval getnext_1${lastid}="$place.4.1"
    eval getnext_4${lastid}="$place.13.1"
    eval getnext_13${lastid}="$place.26.1"
    eval getnext_26${lastid}="$place.27.1"
    eval getnext_27${lastid}="NONE"
    eval getnext_28${lastid}="NONE"
    eval getnext_29${lastid}="NONE"
    eval getnext_30${lastid}="NONE"
    eval getnext_31${lastid}="NONE"
    eval getnext_32${lastid}="NONE"
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

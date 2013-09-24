#!/bin/sh

# Have a look at /usr/share/doc/bridge-utils/README.Debian if you want
# more info about the way on wich a bridge is set up on Debian.
# Author: Satish Ashok, <sashok@cumulusnetworks.com>

if [ ! -x /sbin/mstpctl ]
then
  exit 0
fi

. /lib/mstpctl-utils/mstpctl-utils.sh

case "$IF_MSTPCTL_PORTS" in
    "")
	exit 0
	;;
    none)
	INTERFACES=""
	;;
    *)
	INTERFACES="$IF_MSTPCTL_PORTS"
	;;
esac


# Previous work (create the interface)
if [ "$MODE" = "start" ] && [ ! -d /sys/class/net/$IFACE ]; then
  brctl addbr $IFACE || exit 1
# Previous work (stop the interface)
elif [ "$MODE" = "stop" ];  then
  ifconfig $IFACE down || exit 1
fi

all_interfaces= &&
unset all_interfaces &&
mstpctl_parse_ports $INTERFACES | while read i
do
  for port in $i
  do
    # We attach and configure each port of the bridge
    if [ "$MODE" = "start" ] && [ ! -d /sys/class/net/$IFACE/brif/$port ]; then
      if [ -x /etc/network/if-pre-up.d/vlan ]; then
        env IFACE=$port /etc/network/if-pre-up.d/vlan
      fi
      if [ "$IF_BRIDGE_HW" ]
      then
         ifconfig $port down; ifconfig $port hw ether $IF_BRIDGE_HW
      fi
      if [ -f /proc/sys/net/ipv6/conf/$port/disable_ipv6 ]
      then
        echo 1 > /proc/sys/net/ipv6/conf/$port/disable_ipv6
      fi
      brctl addif $IFACE $port && ifconfig $port 0.0.0.0 up
    # We detach each port of the bridge
    elif [ "$MODE" = "stop" ] && [ -d /sys/class/net/$IFACE/brif/$port ];  then
      ifconfig $port down && brctl delif $IFACE $port && \
        if [ -x /etc/network/if-post-down.d/vlan ]; then
          env IFACE=$port /etc/network/if-post-down.d/vlan
        fi
      if [ -f /proc/sys/net/ipv6/conf/$port/disable_ipv6 ]
      then
        echo 0 > /proc/sys/net/ipv6/conf/$port/disable_ipv6
      fi
    fi
  done
done

# We finish setting up the bridge
if [ "$MODE" = "start" ] ; then

  if [ "$IF_MSTPCTL_STP" ]
  then
    brctl stp $IFACE $IF_MSTPCTL_STP
  fi

  if [ "$IF_MSTPCTL_MAXAGE" ]
  then
    mstpctl setmaxage $IFACE $IF_MSTPCTL_MAXAGE
  fi

  if [ "$IF_MSTPCTL_FDELAY" ]
  then
    mstpctl setfdelay $IFACE $IF_MSTPCTL_FDELAY
  fi

  if [ "$IF_MSTPCTL_MAXHOPS" ]
  then
    mstpctl setmaxhops $IFACE $IF_MSTPCTL_MAXHOPS
  fi

  if [ "$IF_MSTPCTL_TXHOLDCOUNT" ]
  then
    mstpctl settxholdcount $IFACE $IF_MSTPCTL_TXHOLDCOUNT
  fi

  if [ "$IF_MSTPCTL_FORCEVERS" ]
  then
    mstpctl setforcevers $IFACE $IF_MSTPCTL_FORCEVERS
  fi

  if [ "$IF_MSTPCTL_TREEPRIO" ]
  then
    mstpctl settreeprio $IFACE 0 $IF_MSTPCTL_TREEPRIO
  fi


  if [ "$IF_MSTPCTL_PORTPATHCOST" ]
  then
    portpathcosts=$(echo "$IF_MSTPCTL_PORTPATHCOST" | tr '\n' ' ' | tr -s ' ')
    for portpathcost in $portpathcosts
    do
       port=$(echo $portpathcost | cut -d '=' -f1)
       pathcost=$(echo $portpathcost | cut -d '=' -f2)
       if [ -n $port -a -n $pathcost ]
       then
          mstpctl setportpathcost $IFACE $port $pathcost
       fi
    done
  fi

  if [ "$IF_MSTPCTL_PORTADMINEDGE" ]
  then
    portadminedges=$(echo "$IF_MSTPCTL_PORTADMINEDGE" | tr '\n' ' ' | tr -s ' ')
    for portadminedge in $portadminedges
    do
       port=$(echo $portadminedge | cut -d '=' -f1)
       adminedge=$(echo $portadminedge | cut -d '=' -f2)
       if [ -n $port -a -n $adminedge ]
       then
          mstpctl setportadminedge $IFACE $port $adminedge
       fi
    done
  fi

  if [ "$IF_MSTPCTL_PORTAUTOEDGE" ]
  then
    portautoedges=$(echo "$IF_MSTPCTL_PORTAUTOEDGE" | tr '\n' ' ' | tr -s ' ')
    for portautoedge in $portautoedges
    do
       port=$(echo $portautoedge | cut -d '=' -f1)
       autoedge=$(echo $portautoedge | cut -d '=' -f2)
       if [ -n $port -a -n $autoedge ]
       then
          mstpctl setportautoedge $IFACE $port $autoedge
       fi
    done
  fi

  if [ "$IF_MSTPCTL_PORTP2P" ]
  then
    portp2ps=$(echo "$IF_MSTPCTL_PORTP2P" | tr '\n' ' ' | tr -s ' ')
    for portp2p in $portp2ps
    do
       port=$(echo $portp2p | cut -d '=' -f1)
       p2p=$(echo $portp2p | cut -d '=' -f2)
       if [ -n $port -a -n $p2p ]
       then
          mstpctl setportp2p $IFACE $port $p2p
       fi
    done
  fi

  if [ "$IF_MSTPCTL_PORTRESTRROLE" ]
  then
    portrestrroles=$(echo "$IF_MSTPCTL_PORTRESTRROLE" | tr '\n' ' ' | tr -s ' ')
    for portrestrrole in $portrestrroles
    do
       port=$(echo $portrestrrole | cut -d '=' -f1)
       restrrole=$(echo $portrestrrole | cut -d '=' -f2)
       if [ -n $port -a -n $restrrole ]
       then
          mstpctl setportrestrrole $IFACE $port $restrrole
       fi
    done
  fi

  if [ "$IF_MSTPCTL_PORTRESTRTCN" ]
  then
    portrestrtcns=$(echo "$IF_MSTPCTL_PORTRESTRTCN" | tr '\n' ' ' | tr -s ' ')
    for portrestrtcn in $portrestrtcns
    do
       port=$(echo $portrestrtcn | cut -d '=' -f1)
       restrtcn=$(echo $portrestrtcn | cut -d '=' -f2)
       if [ -n $port -a -n $restrtcn ]
       then
          mstpctl setportrestrtcn $IFACE $port $restrtcn
       fi
    done
  fi

  if [ "$IF_MSTPCTL_BPDUGUARD" ]
  then
    portbpduguards=$(echo "$IF_MSTPCTL_BPDUGUARD" | tr '\n' ' ' | tr -s ' ')
    for portbpduguard in $portbpduguards
    do
       port=$(echo $portbpduguard | cut -d '=' -f1)
       bpduguard=$(echo $portbpduguard | cut -d '=' -f2)
       if [ -n $port -a -n $bpduguard ]
       then
          mstpctl setbpduguard $IFACE $port $bpduguard
       fi
    done
  fi

  if [ "$IF_MSTPCTL_TREEPORTPRIO" ]
  then
    treeportprios=$(echo "$IF_MSTPCTL_TREEPORTPRIO" | tr '\n' ' ' | tr -s ' ')
    for treeportprio in $treeportprios
    do
       treeport=$(echo $treeportprio | cut -d '=' -f1)
       prio=$(echo $treeportprio | cut -d '=' -f2)
       if [ -n $treeport -a -n $prio ]
       then
          mstpctl settreeportprio $IFACE $treeport 0 $prio
       fi
    done
  fi

  if [ "$IF_MSTPCTL_TREEPORTCOST" ]
  then
    treeportcosts=$(echo "$IF_MSTPCTL_TREEPORTCOST" | tr '\n' ' ' | tr -s ' ')
    for treeportcost in $treeportcosts
    do
       treeport=$(echo $treeportcost | cut -d '=' -f1)
       cost=$(echo $treeportcost | cut -d '=' -f2)
       if [ -n $treeport -a -n $cost ]
       then
          mstpctl settreeportcost $IFACE $treeport 0 $cost
       fi
    done
  fi

  if [ "$IF_MSTPCTL_HELLO" ]
  then
    mstpctl sethello $IFACE $IF_MSTPCTL_HELLO
  fi

  if [ "$IF_MSTPCTL_AGEING" ]
  then
    mstpctl setageing $IFACE $IF_MSTPCTL_AGEING
  fi

  if [ "$IF_MSTPCTL_PORTNETWORK" ]
  then
    portnetworks=$(echo "$IF_MSTPCTL_PORTNETWORK" | tr '\n' ' ' | tr -s ' ')
    for portnetwork in $portnetworks
    do
       port=$(echo $portnetwork | cut -d '=' -f1)
       network=$(echo $portnetwork | cut -d '=' -f2)
       if [ -n $port -a -n $network ]
       then
          mstpctl setportnetwork $IFACE $port $network
       fi
    done
  fi

  # We activate the bridge
  ifconfig $IFACE 0.0.0.0 up

# Finally we destroy the interface
elif [ "$MODE" = "stop" ];  then

  brctl delbr $IFACE

fi

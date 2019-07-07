#the following line combines the last line to prevent this file from being sourced twice
if [ "x$cfgmgr_sh" = "x" ]; then cfgmgr_sh="sourced"
PATH=/usr/sbin:/usr/bin:/sbin:/bin
cfg_libdir=/lib/cfgmgr
cfg_cmddir=/sbin/cfgmgr
CONFIG=/bin/config
FIREWALL=/www/cgi-bin/firewall.sh

oc () { "$@" >> /dev/console 2>&1 ; } # stdout & stderr to /dev/console
qt () { "$@" >> /dev/null 2>&1 ; } # stdout & stderr to /dev/null

fi #-------------------- this must be the last line -----------------------------

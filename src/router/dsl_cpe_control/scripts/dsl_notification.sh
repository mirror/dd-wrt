#!/bin/sh
PIPECMD=/usr/sbin/dsl_cpe_pipe.sh
if [ ${DSL_NOTIFICATION_TYPE} = "DSL_NOTIFICATION_STATUS" ] ; then
	nvram set dsl_status=${DSL_INTERFACE_STATUS}
fi
if [ ${DSL_NOTIFICATION_TYPE} = "DSL_INTERFACE_STATUS" ] ; then
	nvram set dsl_iface_status=${DSL_INTERFACE_STATUS}
	nvram set dsl_tcl_status=${DSL_TC_LAYER_STATUS}
	nvram set dsl_xtu_status=${DSL_XTU_STATUS}
fi
#if [ ${DSL_NOTIFICATION_TYPE} = "DSL_DATARATE_STATUS_DS" ] ; then
	#nvram set dsl_datarate_ds=${DSL_DATARATE_DS_BC1}
#fi
#if [ ${DSL_NOTIFICATION_TYPE} = "DSL_DATARATE_STATUS_US" ] ; then
	#nvram set dsl_datarate_us=${DSL_DATARATE_US_BC1}
#fi
nvram set dsl_snr_down=`$PIPECMD g997lsg 1 1 |tr -d "\n" | awk -F " " '{printf substr($6,5)/10}')`
nvram set dsl_snr_up=`$PIPECMD g997lsg 0 1 |tr -d "\n" | awk -F " " '{printf substr($6,5)/10}')`
nvram set dsl_datarate_ds=`$PIPECMD g997csg 0 1 |tr -d "\n" | awk -F " " '{printf ("%.2f",substr($4,16)/1024/1024)}'`
nvram set dsl_datarate_us=`$PIPECMD g997csg 0 0 |tr -d "\n" | awk -F " " '{printf ("%.2f",substr($4,16)/1024/1024)}'`

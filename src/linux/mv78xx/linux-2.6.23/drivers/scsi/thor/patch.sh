#!/bin/sh

subtxt(){

    if [ ! -f "$1" ] || [ ! -w "$1" ]; then
	echo "File does not exist or is not writable."
	exit 1
    fi

    if [ "$2" == "a" ];then
	grep SCSI_MV_THOR "$1" >/dev/null 2>&1
	if [ "$?" == "0" ];then
	    cat "$1"
	    return
	fi

	sed -e '/SCSI low-level drivers/{
		n
		n
		a\
config SCSI_MV_THOR\
	tristate "Marvell Storage Controller 6121/6122/6141/6145"\
	depends on SCSI && BLK_DEV_SD\
	help\
		Provides support for Marvell thor Storage Controller series.\n
}' "$1" 
    else
	sed -e '/SCSI_MV_THOR/,+5 d' "$1"
    fi

}

# $1 is supposed to be the $KERNEL_SRC/drivers/scsi
if [ ! -d "$1" ];then
    echo "Cannot find the specified directory."
    exit 1
fi

cd "$1"
subtxt Kconfig $2 > Kconfig.new
mv Kconfig Kconfig.orig
mv Kconfig.new Kconfig


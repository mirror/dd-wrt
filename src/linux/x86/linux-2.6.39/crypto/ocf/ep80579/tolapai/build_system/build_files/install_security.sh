#!/bin/sh

###############################################################################
#
# This file is provided under a dual BSD/GPLv2 license.  When using or 
#   redistributing this file, you may do so under either license.
# 
#   GPL LICENSE SUMMARY
# 
#   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
# 
#   This program is free software; you can redistribute it and/or modify 
#   it under the terms of version 2 of the GNU General Public License as
#   published by the Free Software Foundation.
# 
#   This program is distributed in the hope that it will be useful, but 
#   WITHOUT ANY WARRANTY; without even the implied warranty of 
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#   General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License 
#   along with this program; if not, write to the Free Software 
#   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
#   The full GNU General Public License is included in this distribution 
#   in the file called LICENSE.GPL.
# 
#   Contact Information:
#   Intel Corporation
# 
#   BSD LICENSE 
# 
#   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
#   All rights reserved.
# 
#   Redistribution and use in source and binary forms, with or without 
#   modification, are permitted provided that the following conditions 
#   are met:
# 
#     * Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright 
#       notice, this list of conditions and the following disclaimer in 
#       the documentation and/or other materials provided with the 
#       distribution.
#     * Neither the name of Intel Corporation nor the names of its 
#       contributors may be used to endorse or promote products derived 
#       from this software without specific prior written permission.
# 
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 
#  version: Security.L.1.0.3-98
#
###############################################################################
BUILD_OUTPUT_DIR=$2
cd $BUILD_OUTPUT_DIR

MODULE_DIR=/lib/modules/`uname -r`
OBJ_LIST="icp_asd.ko icp_crypto.ko icp_debug.ko icp_hal.ko"
#note icp_debugmgr.ko is not include here as its optional
BIN_LIST="uof_firmware.bin mmp_firmware.bin"

usage() {
echo
echo ----------------------------------
echo USAGE:
echo ----------------------------------
echo "#  $0 install||uninstall BUILD_OUTPUT_DIR "
echo "#"
echo ----------------------------------

exit 1

}
case $1 in
    install)
    
	echo "Copy QAT firmware to /lib/firmware"
	for bin_obj in $BIN_LIST
	do
	    cp $bin_obj /lib/firmware
	done    

	# delete the existing kernel objects if they exist in the /lib/modules
	# folder
	echo "Copying QAT kernel object to lib/modules/`uname -r`"
	for kern_obj in $OBJ_LIST
	do
	    install -D -m 644 $kern_obj $MODULE_DIR/$kern_obj
	done
	echo "Creating module.dep file for QAT released kernel object"
	echo "This will take a few moments"
	/sbin/depmod -a

	echo "Creating startup and kill scripts"
	cp qat_service /etc/init.d
	cp icp_asd.conf /etc
	cp asd_ctl /etc/init.d

	chkconfig --add qat_service

	# not loading icp_debugmgmt.ko as its optional
	#echo "Loading icp_debugmgmt.ko"
	#insmod icp_debugmgmt.ko
	#MIL=`cat /proc/devices | grep mil_driver | awk '{print $1}'`
	#mknod /dev/mil_driver c $MIL 0

	
	echo "Starting QAT service"
	/etc/init.d/qat_service start
	
	;;

    uninstall)
	echo "Unloading QAT kernel object"
	/etc/init.d/qat_service stop
	#/sbin/rmmod icp_debugmgmt.ko
	#if [ -e /dev/mil_driver ];
	#then
	#    /sbin/rm -rf /dev/mil_driver
	#fi    
	
	echo "Removing startup scripts"
	# Check if one of the /etc/rc3.d script exist, then all the
	# startup scripts would exist
	if [ -e /etc/init.d/qat_service ];
	then
	    chkconfig --del qat_service
	    /bin/rm -f /etc/init.d/qat_service
	    /bin/rm -f /etc/init.d/asd_ctl
	fi

	if [ -e /etc/icp_asd.conf ];
	then
	    /bin/rm -f /etc/icp_asd.conf
	fi
	

	echo "Removing the QAT firmware"
	for bin_obj in $BIN_LIST
	do
	    if [ -e /lib/firmware/$bin_obj ];
	    then
		/bin/rm -f /lib/firmware/$bin_obj
	    fi
	done
	
	echo "Removing kernel objects from /lib/modules/`uname -r`"
	for kern_obj in $OBJ_LIST
	do
	    if [ -e $MODULE_DIR/$kern_obj ] ;
	    then
		/bin/rm -f $MODULE_DIR/$kern_obj
	    fi
	done
	echo "Rebuilding the module.dep file, this will take a few moments"
	/sbin/depmod -a

	;;

    *)
	usage
	;;
esac
    

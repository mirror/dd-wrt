#!/bin/sh
#Copyright (C) BlueWave Projects and Services 2015-2023
#This software is released under the GNU GPL license.
#
# Warning - shebang sh is for compatibliity with busybox ash (eg on OpenWrt)
# This is changed to bash automatically by Makefile for generic Linux
#

# functions:

download_image_files() {
	# The list of images to be downloaded is defined in $ndscustomimages ( see near the end of this file )
	# The source of the images is defined in the openNDS config

	for nameofimage in $ndscustomimages; do
		get_image_file "$nameofimage"
	done
}

download_data_files() {
	# The list of files to be downloaded is defined in $ndscustomfiles ( see near the end of this file )
	# The source of the files is defined in the openNDS config

	for nameoffile in $ndscustomfiles; do
		get_data_file "$nameoffile"
	done
}

##################################################
#  Start - Main entry point for this sub script
#  It is called by inclusion in its parent script
##################################################

# Construct the list of custom images and files to download
customimagelist=$(uci -q get opennds.@opennds[0].fas_custom_images_list)
customfilelist=$(uci -q get opennds.@opennds[0].fas_custom_files_list)
ndscustomimages=""
ndscustomfiles=""

if [ ! -z "$customimagelist" ]; then

	for imageconfig in $customimagelist; do
		imagename=$(echo "$imageconfig" | awk -F"=" '{printf "%s" $1}')
		imageurl=$(echo "$imageconfig" | awk -F"=" '{printf "%s" $2}')
		ndscustomimages="$ndscustomimages $imagename"
		eval $imagename=$imageurl
	done
fi

if [ ! -z "$customfilelist" ]; then

	for fileconfig in $customfilelist; do
		filename=$(echo "$fileconfig" | awk -F"=" '{printf "%s" $1}')
		fileurl=$(echo "$fileconfig" | awk -F"=" '{printf "%s" $2}')
		ndscustomfiles="$ndscustomfiles $filename"
		eval $filename=$fileurl
	done
fi


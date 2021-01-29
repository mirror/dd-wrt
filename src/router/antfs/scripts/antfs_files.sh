#!/bin/sh

set -ue

prog="${0##*/}"
verbose=0
frag=0
ntfs_path=""
ntfs_fil_size=0
write_size=0
number_of_files=0
directory="tiny_files"
dir=0
directory_limit=512
# TODO: Don't touch this! ... or adapt string generator.
max_bytes=1024


show_help() {
	echo "<INFO> Usage: ${prog} [OPTIONS] Device
		
Options:
    -d		Directory name, where the files will be written to
    -l		Limit of files per directory
    -s		Size (byte) that will be used up
    -n		Number of files that will be created
    -f		Do you like it fragmented?
    -v		Increase verbosity"
}

verbose() {
	if [ -n "$verbose" ] ; then
		echo -ne "$@" >&2
	fi
}

write_file() {
	local filename=$1
	local filesize=$2

	if [ $filename -gt 0 ] && [ $(($filename % $directory_limit)) -eq 0 ]; then
		dir=$(($dir + 1))
		[ $verbose -eq 0 ] || echo "<INFO> Creating directory: $ntfs_path/$directory/$dir"
		mkdir -p "$ntfs_path/$directory/$dir"
	fi

	if [ $verbose -eq 0 ]; then
		# dd if=/dev/urandom of="$ntfs_path/$directory/$dir/$filename" bs=1 count=$filesize >& /dev/null
		echo $y | dd of="$ntfs_path/$directory/$dir/$filename" count=1 bs=$filesize >& /dev/null
	else
		echo "<INFO> Writing file: $ntfs_path/$directory/$dir/$filename"
		# dd if=/dev/urandom of="$ntfs_path/$directory/$dir/$filename" bs=1 count=$filesize
		echo $y | dd of="$ntfs_path/$directory/$dir/$filename" count=1 bs=$filesize
	fi
}

# Create lots of small files.
create_files() {
	local dir=0
	local file=0
	local file_size=0

	if [ $number_of_files -gt 0 ]; then
		if [ $write_size -gt 0 ]; then
			max_bytes=$(($ntfs_fil_size / $number_of_files))
			while [ $number_of_files -gt $file ]; do
				rnd_size=$((( RANDOM % $max_bytes ) + 1))
				if [ $ntfs_fil_size -lt $rnd_size ] ||
					[ $(($file + 1)) -eq $number_of_files ]; then
					file_size=$ntfs_fil_size
				else
					file_size=$rnd_size
				fi
				write_file $file $file_size
				ntfs_fil_size=$((ntfs_fil_size - file_size))
				file=$(($file + 1))
			done
		else
			while [ $number_of_files -gt $file ]; do
				write_file $file $max_bytes
				file=$(($file + 1))
			done
		fi
	else
		while [ $ntfs_fil_size -gt 0 ]; do
			rnd_size=$((( RANDOM % $max_bytes ) + 1))
			if [ $ntfs_fil_size -lt $rnd_size ]; then
				file_size=$ntfs_fil_size
			else
				file_size=$rnd_size
			fi
			write_file $file $file_size
			ntfs_fil_size=$((ntfs_fil_size - file_size))
			file=$(($file + 1))
		done
	fi	

	sleep 1		# wait a second due to sync
	if [ $frag -eq 1 ]; then
		local remove_file=
		find "$ntfs_path/$directory" -type f -print0 \
		| while read -d $'\0' remove_file; do
			file=${remove_file##*\/}
			(( file % 3 )) || rm "$remove_file"
		done
	fi
}

get_device_info() {
	local device=$1

	ntfs_path=`mount | grep $device | cut -d' ' -f3`

	if [ $write_size -gt 0 ]; then
		ntfs_fil_size=$write_size
	fi

	if [ ! -e "$ntfs_path/$directory/$dir" ]; then
		[ $verbose -eq 0 ] || echo "<INFO> Creating directory: $ntfs_path/$directory/$dir"
		mkdir -p "$ntfs_path/$directory/$dir"
	fi
}

OPTIND=1         # Reset in case getopts has been used previously in the shell.
while getopts ":h?vd:l:n:s:f" opt; do
	case "$opt" in
		d)
			directory=$OPTARG
			;;
		l)
			directory_limit=$OPTARG
			;;
		s)
			write_size=$OPTARG
			;;
		n)
			number_of_files=$OPTARG
			;;
		f)
			frag=1
			;;
		v)
			verbose=1
			;;
		h|\?)
			show_help
			exit 0
			;;
	esac
done
shift $((OPTIND-1))

if [ -z $@ ]; then
	show_help
	exit 1
fi

if [ -e "$1" ]; then
       #prepare testdata 1024(2^10) bytes with char 'y'
        y=y
        for a in $( seq 10 ); do
            y=$y$y
        done

	get_device_info $1
	create_files
	echo "<INFO> Written $number_of_files files to $directory"
	echo "<INFO> Finished successfully"
else
	echo "<ERROR>: Unknown device $1" >&2
	exit 1
fi

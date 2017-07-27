#!/bin/sh
#
# get-hdr-loc.sh -- get Darwin XNU kernel header file location
#
# Interactively requests the specification of the path to the host's Darwin
# XNU kernel header files.  Checks that path and returns it to the caller.
#
# Usage: file1 file2 ... fileN
#
# Where: file1	first header file needed
#	 file2	second header file needed
#	 ...
#	 fileN	last header file needed
#
# Exit:
#
#	Exit code: 0 if path found; path returned on STDOUT
#
#		   1 if path not found: error message returned
#					on STDOUT
#
#set -x	# for DEBUGging

# Check argument count.  There must be at least one argument.

if test $# -lt 1	# {
then
  echo "insufficient arguments: $#"
  exit 1
fi	# }
lst=$*

# Request the path to the Darwin XNU kernel header files.

trap 'stty echo; echo interrupted; exit 1' 1 2 3 15
FOREVER=1
while test $FOREVER -ge 1	# {
do
  if test $FOREVER -eq 1	# {
  then
    echo "---------------------------------------------------------------" 1>&2
    echo "" 1>&2
    echo "Lsof cannot find some Darwin XNU kernel header files it needs." 1>&2
    echo "They should have already been downloaded from:" 1>&2
    echo "" 1>&2
    echo "    http://www.opensource.apple.com/darwinsource/index.html" 1>&2
    echo "" 1>&2
    echo "and then installed.  (See 00FAQ for download and installation" 1>&2
    echo "instructions.)" 1>&2
    echo "" 1>&2
    echo "Please specify the path to the place where they were installed." 1>&2
    echo "" 1>&2
    echo "---------------------------------------------------------------" 1>&2
  fi	# }

  END=0
  while test $END = 0	# {
  do
    echo "" 1>&2
    echo -n "What is the path? " 1>&2
    read HP EXCESS
    HP=`echo echo $HP | /bin/csh -fs`
    if test $? -eq 0	# {
    then
      if test "X$HP" = "X"	# {
      then
        echo "" 1>&2
        echo "+================================+" 1>&2
        echo "| Please enter a non-empty path. |" 1>&2
        echo "+================================+" 1>&2
        echo "" 1>&2
      else
        END=1
      fi	# }
    else
      echo "" 1>&2
      echo "+============================+" 1>&2
      echo "| Please enter a legal path. |" 1>&2
      echo "+============================+" 1>&2
      echo "" 1>&2
    fi	# }
  done	# }
  
  # See if the header files are available in the specified path.

  MH=""
  for i in $lst	# {
  do
    if test ! -f ${HP}/bsd/$i -a ! -f ${HP}/osfmk/$i    # {
    then
      if test "X$MH" = "X"	# {
      then
        MH=$i
      else
        MH="$MH $i"
      fi	# }
    fi	# }
  done	# }
  if test "X$MH" = "X"	# {
  then

    # All header files are available, so return the path and exit cleanly.

    echo $HP
    exit 0
  else
    echo "" 1>&2
    echo "ERROR: not all header files are in:" 1>&2
    echo "" 1>&2
    echo "       ${HP}" 1>&2
    echo "" 1>&2
    echo " These are missing:" 1>&2
    echo "" 1>&2
    echo "     $MH" 1>&2
    FOREVER=2
  fi	# }
done	# }
echo "unknown error"
exit 1

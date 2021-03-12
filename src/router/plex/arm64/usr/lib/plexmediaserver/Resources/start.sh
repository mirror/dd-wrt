#!/bin/sh
#SCRIPTPATH=$(dirname $(python -c 'import sys,os;print os.path.realpath(sys.argv[1])' $0))
SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname ${SCRIPT}`
export LD_LIBRARY_PATH="${SCRIPTPATH}"
export PLEX_MEDIA_SERVER_HOME="${SCRIPTPATH}"
export PLEX_MEDIA_SERVER_MAX_PLUGIN_PROCS=6
export LC_ALL="en_US.UTF-8"
export LANG="en_US.UTF-8"
ulimit -s 3000
./Plex\ Media\ Server

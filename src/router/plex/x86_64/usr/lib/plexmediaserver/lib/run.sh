#!/bin/sh
### BEGIN INIT INFO
# Provides:          plexmediaserver
# Required-Start:    $remote_fs $local_fs $all $syslog $networking
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Plex Media Server
# Description:       Plex Media Server for Linux,
#                    More information at http://www.plexapp.com
# Author:            Plex Packaging team
# Version:           1.06
### END INIT INFO

# Set Crash Reporting identification variables
export PLEX_MEDIA_SERVER_INFO_VENDOR="DD-WRT $(nvram get DD_BOARD)"
export PLEX_MEDIA_SERVER_INFO_DEVICE="PC"
export PLEX_MEDIA_SERVER_INFO_MODEL="$(uname -m)"
export PLEX_MEDIA_SERVER_INFO_PLATFORM_VERSION="$(nvram get os_version)"

# Default values prior to ingesting and validating contents of /etc/default/plexmediaserver
PlexUser="root"
PlexHome="/usr/lib/plexmediaserver"
PlexAppSuppDir="/var/lib/plexmediaserver/Library/Application Support"
PlexTempDir="/tmp"
PlexStackSize=3000
PlexPluginProcs=6

# Read configuration variable file if it is present
[ -r /etc/default/plexmediaserver ] && . /etc/default/plexmediaserver

Running=$(ps | grep 'Plex Media Server'| grep -v grep | wc -l)
    if [ $Running -gt 0 ]; then
      echo "Plex is already running."
      exit 0
    fi

    if [ -f /etc/default/locale ]; then
      export LANG="$(cat /etc/default/locale | awk -F '=' '/LANG=/{print $2}' | sed 's/"//g')"
      export LC_ALL="$LANG"
    fi


    # PlexUser's HOME directory is the default location. Supersede if specified
    PlexAppSuppDir="$1/Library/Application Support"
    mkdir -p "$PlexAppSuppDir"

    # Check TempDir
    Candidate=""
    # Be generous with TEMP DIR specification
    if [ "$TMPDIR" != "" ]; then
      Candidate="$TMPDIR"

    elif [ "$TEMP" != "" ]; then
      Candidate="$TEMP"

    elif [ "$TMP" != "" ] && [ -d "$TMP" ]; then
      Candidate="$TMP"
    fi

    # Validate TempCandidate
    if [ "$Candidate" != "" ]; then
      if [ -d "$Candidate" ]; then
        PlexTempDir="$Candidate"
      else
        echo "${0}: Temp Directory does not exist: \"$Candidate\".  Using default location."
      fi
    fi

    # Plug-in Procs  (No checking.  PMS handles internally)
    if [ "$PLEX_MEDIA_SERVER_MAX_PLUGIN_PROCS" != "" ]; then
      Candidate="$(echo $PLEX_MEDIA_SERVER_MAX_PLUGIN_PROCS | grep -x -E '[[:digit:]]+' )"
      if [ "$Candidate" != "" ]; then
        PlexPluginProcs="$Candidate"
      else
        echo "${0}: Non-numeric Max Plug-in Procs given: \"$PLEX_MEDIA_SERVER_MAX_PLUGIN_PROCS\".  Using default value."
      fi
    fi

    # Stack Size
    if [ "$PLEX_MEDIA_SERVER_MAX_STACK_SIZE" != "" ]; then
      Candidate="$(echo $PLEX_MEDIA_SERVER_MAX_STACK_SIZE | grep -x -E '[[:digit:]]+' )"
      if [ "$Candidate" != "" ]; then
        PlexStackSize="$Candidate"
      else
        echo "${0}: Non-numeric Max Stack Size given: \"$PLEX_MEDIA_SERVER_MAX_STACK_SIZE\".  Using default value."
      fi
    fi

    # Verify Plex Media Server is indeed where it says it is
    if [ "$PLEX_MEDIA_SERVER_HOME" != "" ]; then
      if [ -d "$PLEX_MEDIA_SERVER_HOME" ]; then
        PlexHome="$PLEX_MEDIA_SERVER_HOME"
      else
        echo "${0}: Given application location \"${PLEX_MEDIA_SERVER_HOME}\" does not exist.  Using default location."
      fi
    fi

    # Create AppSuppDir if not present and set ownership
    if [ ! -d "$PlexAppSuppDir" ]; then
      mkdir -p "$PlexAppSuppDir"
      if [ $? -eq 0 ]; then
        chown "${PlexUser}"."${PlexUser}" "$PlexAppSuppDir"
      else
        echo "ERROR: Could not create \"$PlexAppSuppDir\".  System error code $?"
        exit 1
      fi
    fi

    # Build the final runtime environment variables.  Specify these parameters in /etc/default/plexmediaserver
    export PLEX_MEDIA_SERVER_USER="$PlexUser"
    export PLEX_MEDIA_SERVER_MAX_PLUGIN_PROCS="$PlexPluginProcs"
    export PLEX_MEDIA_SERVER_HOME="$PlexHome"
    export PLEX_MEDIA_SERVER_MAX_STACK_SIZE="$PlexStackSize"
    export PLEX_MEDIA_SERVER_TMPDIR="$PlexTempDir"
    export PLEX_MEDIA_SERVER_APPLICATION_SUPPORT_DIR="$PlexAppSuppDir"

    export TMPDIR="${PlexTempDir}"
    ulimit -s "$PLEX_MEDIA_SERVER_MAX_STACK_SIZE"

    # Add sleep - for those who launch with this script
    echo "Starting Plex Media Server."
    export LD_LIBRARY_PATH=/lib:/usr/lib/plexmediaserver/lib:/usr/lib
    /bin/sh -c "exec ${PLEX_MEDIA_SERVER_HOME}/Plex\ Media\ Server &" >/dev/null 2>&1

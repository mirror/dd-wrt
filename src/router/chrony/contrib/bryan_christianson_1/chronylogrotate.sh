#!/bin/sh

#  chronyd/chronyc - Programs for keeping computer clocks accurate.
#
#  **********************************************************************
#  * Copyright (C) Bryan Christianson  2015
#  *
#  * This program is free software; you can redistribute it and/or modify
#  * it under the terms of version 2 of the GNU General Public License as
#  * published by the Free Software Foundation.
#  *
#  * This program is distributed in the hope that it will be useful, but
#  * WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  * General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License along
#  * with this program; if not, write to the Free Software Foundation, Inc.,
#  * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#  *
#  **********************************************************************

LOGDIR=/var/log/chrony

rotate () {
  prefix=$1

  rm -f $prefix.log.10

  for (( count=9; count>= 0; count-- ))
  do
    next=$(( $count+1 ))
    if [ -f $prefix.log.$count ]; then
      mv $prefix.log.$count $prefix.log.$next
    fi
  done

  if [ -f $prefix.log ]; then
    mv $prefix.log $prefix.log.0
  fi
}

if [ ! -e "$LOGDIR" ]; then
  logger -s "missing directory: $LOGDIR"
  exit 1
fi

cd $LOGDIR

rotate measurements
rotate statistics
rotate tracking

#
# signal chronyd via chronyc
/usr/local/bin/chronyc cyclelogs > /dev/null

exit $?
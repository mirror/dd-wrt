#!/usr/bin/env bash
#
# Configure.sh	Generates interactively a config.h from config.in
#
# net-tools	A collection of programs that form the base set of the
#		NET-3 Networking Distribution for the LINUX operating
#		system.
#
# Usage:	Install.sh [--nobackup] [--test]
#
# Version:	Install.sh 1.65	(1996-01-12)
#
# Authors:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
#		Johannes Grosen, <grosen@argv.cs.ndsu.nodak.edu>
#		Copyright 1988-1993 MicroWalt Corporation
#
# Modified:
#        {1.65} Bernd eckes Eckenfels <net-tools@lina.inka.de>
#		some layout cleanups, slattach/plipconfig removed.
#		--test for testinstallations added.
#
#		This program is free software; you can redistribute it
#		and/or  modify it under  the terms of  the GNU General
#		Public  License as  published  by  the  Free  Software
#		Foundation;  either  version 2 of the License, or  (at
#		your option) any later version.
#
#
# Make sure we're really running bash.
#
# I would really have preferred to write this script in a language with
# better string handling, but alas, bash is the only scripting language
# that I can be reasonable sure everybody has on their Linux machine.
#

CONFIG=config.h
MAKECONFIG=config.make


[ -z "$BASH" ] && { echo "Configure requires bash" 1>&2; exit 1; }

# Disable filename globbing once and for all.
# Enable function cacheing.
set -f -h

# set up reading of config file
if [ "$#" != "1" ] || [ ! -f "$1" ]; then
	echo "usage: $0 configfile" 1>&2
	exit 1
fi
exec 7<$1
config_fd_redir='<&7'

#
# readln reads a line into $ans.
#
#	readln prompt default
#
function readln()
{
  echo -n "$1"
  IFS='@' read ans || exit 1
  [ -z "$ans" ] && ans=$2
}

# bool processes a boolean argument
#
#	bool tail
#
function bool()
{
  # Slimier hack to get bash to rescan a line.
  eval "set -- $1"
  ans=""
  while [ "$ans" != "y" -a "$ans" != "n" ]
  do
	readln "$1 ($2) [$3] " "$3"
  done
  if [ "$ans" = "y" ]; then
	echo "#define $2 1" >>${CONFIG}
	echo "$2=1" >>${MAKECONFIG}
    else
	echo "#define $2 0" >>${CONFIG}
	echo "# $2=0" >> ${MAKECONFIG}
  fi
  raw_input_line="bool '$1' $2 $ans"
  eval "$2=$ans"
}

# int processes an integer argument
#
#	int tail
#
function int()
{
  # Slimier hack to get bash to rescan a line.
  eval "set -- $1"
  ans="x"
  while [ $[$ans+0] != "$ans" ];
  do
	readln "$1 ($2) [$3] " "$3"
  done
  echo "#define $2 ($ans)" >>${CONFIG}
  raw_input_line="int '$1' $2 $ans"
  eval "$2=$ans"
}

  #
  # Make sure we start out with a clean slate.
  #
  > config.new
  > ${CONFIG}
  > ${MAKECONFIG}

  stack=''
  branch='t'

  while IFS='@' eval read raw_input_line ${config_fd_redir}
  do
	# Slimy hack to get bash to rescan a line.
	read cmd rest <<-END_OF_COMMAND
		$raw_input_line
	END_OF_COMMAND

	if [ "$cmd" = "*" ]; then
		if [ "$branch" = "t" ]; then
			echo "$raw_input_line"
			# echo "# $rest" >>$CONFIG
			if [ "$prevcmd" != "*" ]; then
				echo >>${CONFIG}
				echo "/* $rest" >>${CONFIG}
			else
				echo " * $rest" >>${CONFIG}
			fi
			prevcmd="*"
		fi
	else
		[ "$prevcmd" = "*" ] && echo " */" >>${CONFIG}
		prevcmd=""
		case "$cmd" in
		=)	[ "$branch" = "t" ] && echo "$rest" >>${CONFIG};;
		:)	[ "$branch" = "t" ] && echo "$raw_input_line" ;;
		int)	[ "$branch" = "t" ] && int "$rest" ;;
		bool)	[ "$branch" = "t" ] && bool "$rest" ;;
		exec)	[ "$branch" = "t" ] && ( sh -c "$rest" ) ;;
		if)	stack="$branch $stack"
			if [ "$branch" = "t" ] && eval "$rest"; then
				branch=t
			else
				branch=f
			fi ;;
		else)	if [ "$branch" = "t" ]; then
				branch=f
			else
				read branch rest <<-END_OF_STACK
					$stack
				END_OF_STACK
			fi ;;
		fi)	[ -z "$stack" ] && echo "Error!  Extra fi." 1>&2
			read branch stack <<-END_OF_STACK
				$stack
			END_OF_STACK
			;;
		esac
	fi
	echo "$raw_input_line" >>config.new
  done
  [ "$prevcmd" = "*" ] && echo " */" >>${CONFIG}

  [ -z "$stack" ] || echo "Error!  Unterminated if." 1>&2

  mv config.new config.status
  exit 0

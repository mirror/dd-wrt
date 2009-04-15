// ///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Program is based on the
// http://www.imasy.or.jp/~gotoh/ssh/connect.c
// Written By Shun-ichi GOTO <gotoh@taiyo.co.jp>
//
// If the source code for the program is not available from the place
// from which you received this file, check
// http://ultravnc.sourceforge.net/
//
// Linux port (C) 2005- Jari Korhonen, jarit1.korhonen@dnainternet.net
//////////////////////////////////////////////////////////////////////

#ifndef REPEATER_H
#define REPEATER_H

extern void debug(int msgLevel, const char *fmt, ...);
extern void fatal(const char *fmt, ...);
extern int openConnectionToEventListener(const char *host, unsigned short port, char *listenerIp, int listenerIpSize);
extern int writeExact(int sock, char *buf, int len, int timeOutSecs);

#endif

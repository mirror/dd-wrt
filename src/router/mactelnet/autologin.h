/*
    Mac-Telnet - Connect to RouterOS or mactelnetd devices via MAC address
    Copyright (C) 2010, Håkon Nessjøen <haakon.nessjoen@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUTOLOGIN_MAXSTR 100
#define AUTOLOGIN_MAXPROFILES 100

struct autologin_profile {
	char identifier[AUTOLOGIN_MAXSTR];
	char username[AUTOLOGIN_MAXSTR];
	char password[AUTOLOGIN_MAXSTR];
	char inuse:1;
	char hasUsername:1;
	char hasPassword:1;
};

enum autologin_state {
	ALS_NONE,
	ALS_PREIDENTIFIER,
	ALS_IDENTIFIER,
	ALS_PREKEY,
	ALS_KEY,
	ALS_PREVALUE,
	ALS_VALUE
};

extern struct autologin_profile login_profiles[AUTOLOGIN_MAXPROFILES];
struct autologin_profile *autologin_find_profile(char *identifier);
int autologin_readfile(char *configfile);

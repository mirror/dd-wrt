/*
 * wireless_generic.c
 *
 * Copyright (C) 2005 - 2013 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
static char *UPTIME(int uptime)
{
	int days, minutes;
	static char str[64] = { 0 };
	memset(str, 0, 64);
	days = uptime / (60 * 60 * 24);
	if (days)
		sprintf(str, "%d day%s, ", days, (days == 1 ? "" : "s"));
	minutes = uptime / 60;
	if (strlen(str) > 0)
		sprintf(str, "%s %d:%02d:%02d", str, (minutes / 60) % 24, minutes % 60, uptime % 60);
	else
		sprintf(str, "%d:%02d:%02d", (minutes / 60) % 24, minutes % 60, uptime % 60);
	return str;
}

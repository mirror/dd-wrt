/*
 * wireless_generic.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <gottschall@dd-wrt.com>
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
static char *UPTIME(int uptime, char *str)
{
	int days, minutes;
	char str2[64] = { 0 };
	bzero(str, 64);
	bzero(str2, 64);
	days = uptime / (60 * 60 * 24);
	if (days)
		sprintf(str2, "%d day%s, ", days, (days == 1 ? "" : "s"));
	minutes = uptime / 60;
	if (strlen(str2) > 0)
		sprintf(str, "%s %d:%02d:%02d", str2, (minutes / 60) % 24, minutes % 60, uptime % 60);
	else
		sprintf(str, "%d:%02d:%02d", (minutes / 60) % 24, minutes % 60, uptime % 60);
	return str;
}

#ifndef HAVE_ATH9K
void ej_get_busy(webs_t wp, int argc, char_t ** argv)
{
}

void ej_get_active(webs_t wp, int argc, char_t ** argv)
{
}

void ej_show_busy(webs_t wp, int argc, char_t ** argv)
{
}
#endif

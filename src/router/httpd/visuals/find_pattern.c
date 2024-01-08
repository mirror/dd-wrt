/*
 * find_pattern.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <broadcom.h>

/*
 * Example data = www.kimo.com.tw<&nbsp;>www.google<&nbsp;> find_each(name,
 * sizeof(name), data, "<&nbsp;>", 0, ""); produces : ret = 1, name =
 * "www.kimo.com.tw" find_each(name, sizeof(name), data, "<&nbsp;>", 1, "");
 * produces : ret = 1, name = "google.com" find_each(name, sizeof(name),
 * data, "<&nbsp;>", 2, "No find!"); produces : ret = 0, name = "No Find!" 
 */
int find_each(char *name, int len, char *data, char *token, int which, char *def)
{
	int i;
	int maxlen;
	char *str;
	int tokenlen = strlen(token);

	bzero(name, len);

	for (i = 0;; i++) {
		str = strstr(data, token);
		if (!str)
			break;
		maxlen = (str - data) > len ? len : str - data;
		strncpy(name, data, maxlen);
		name[maxlen] = '\0';
		if (i == which) {
			return 1;
		}
		data = str + tokenlen;
	}

	strncpy(name, def, len); // No find

	return 0;
}

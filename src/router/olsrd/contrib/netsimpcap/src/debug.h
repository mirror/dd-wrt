/*
 * NetsimPcap - a userspace network bridge with simulated packet loss
 *             Copyright 2008 H. Rogge (rogge@fgan.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WLAN_DEBUG_H
#define WLAN_DEBUG_H

#define DPRINT(...)		if(debugMode) { printf(__VA_ARGS__); }

#define DHEXDUMP(purpose, data,length)	if (debugMode) {	\
		int i;										\
		printf("Hexdump: %s\n", purpose);						\
		for (i=0; i<length; i++) {					\
			if ((i & 31) == 0)	printf("%04x: ", i);\
			printf("%02x", ((int)data[i])  & 255);	\
			if ((i & 3) == 3) printf(" ");			\
			if ((i & 31) == 31) printf("\n");		\
		}											\
		printf("\n\n");								\
	}

extern int debugMode;

#endif /* WLAN_DEBUG_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

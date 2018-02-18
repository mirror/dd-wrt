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

#ifndef NETWORK_H_
#define NETWORK_H_

#include <linux/types.h>

typedef struct MacAddress {
  char mac[6];
} MacAddress;

int readMac(char *value, MacAddress * target);
void closeTap(int fd);
int createTap(char *name, MacAddress * mac);

#endif /* NETWORK_H_ */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

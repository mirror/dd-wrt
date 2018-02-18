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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "debug.h"

/*
 * stripString
 *
 * this function strips leading and trailing whitespaces from a string
 *
 * @param string to strip
 * @return stripped string
 */
char *
stripString(char *str)
{

  /* strip left whitespaces */
  while (*str == ' ' || *str == '\t') {
    str++;
  }

  /* strip right whitespaces */
  int i = strlen(str);
  while (--i >= 0) {
    if (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t') {
      str[i] = 0;
      break;
    }
  }

  return str;
}

/*
 * readConnectionMatrix
 *
 * This function reads the network settings and place them in the
 * connection matrix (float array with size "len" x "len".
 *
 * @param pointer to begin of connection matrix
 * @param pointer to filename
 * @param number of elements in each connection matrix line
 * @return 0 if function was successfull, 1 if an error happened
 */
int
readConnectionMatrix(float *connectionMatrix, char *filename, int len)
{
  FILE *file = fopen(filename, "r");
  if (!file) {
    return 1;
  }

  char buffer[1024];
  while (fgets(buffer, 1024, file)) {
    int from, to;
    float propability;

    char *line = stripString(buffer);
    DPRINT("%s\n", line);

    if (line[0] == '#' || line[0] == 0) {
      continue;
    }

    if (sscanf(line, "%d %d %f", &from, &to, &propability) != 3) {
      continue;
    }

    if (from < 0 || from >= len || to < 0 || to >= len || from == to) {
      continue;
    }

    connectionMatrix[GRID(from, to, len)] = propability;
    if (connectionMatrix[GRID(to, from, len)] == 0) {
      connectionMatrix[GRID(to, from, len)] = propability;
    }
  }
  fclose(file);
  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

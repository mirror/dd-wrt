/*
 * ndpiReader.c
 *
 * Copyright (C) 2011-25 - ntop.org
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ndpi_api.h"
#include <assert.h>

/* *********************************************** */

int main(int argc, char *argv[]) {
  ndpi_ranking rank;
  
  if(argc != 2) {
    printf("Usage: print_rank <filename>\n");
    return(0);
  }

  if(!ndpi_deserialize_ranking(&rank, argv[1])) {
    printf("Unable to read file %s\n", argv[1]);
  } else {
    ndpi_print_ranking(&rank);
    ndpi_term_ranking(&rank);
  }
    
  return(0);
}

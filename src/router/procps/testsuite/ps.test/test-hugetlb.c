/*
 * test-hugetlb - * Test program to allocate huge tables.
 *
 * Copyright ©     -2024 Craig Small <csmall@dropbear.xyz>
 *
 * To test run this program and on another console
 * ps -O htprv,htshr -C test-hugetlb p
 * HTPRV will show 2038
 *
 * Make sure you actually have huge pages to mmap()
 * /proc/sys/vm/nr_hugepages should be larger than 0
 *
 * e.g.
 * sudo sh -c 'echo 2 >  /proc/sys/vm/nr_hugepages'
 *
 * shared pages don't seem to be working yet
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define LENGTH (2*1024*1024)

void usage(const char *progname, const char *msg)
{
    if (msg)
        fprintf(stderr, "%s\n", msg);
    fprintf(stderr,
            "Usage: %s size\nSize is in kbytes\n", progname);
    exit(EXIT_FAILURE);
}

int main(int argc, const char *argv[])
{
    char *addr;
    long kbytes;

    if (argc != 2)
        usage(argv[0], NULL);

    kbytes = atoi(argv[1]);
    if (kbytes < 0)
        usage(argv[0], "Invalid size");

    if (MAP_FAILED == (addr = mmap(NULL, (kbytes*1024), (PROT_READ | PROT_WRITE),
                    (MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB), 0, 0))) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    printf("Returned address is %p\n", addr);
    printf("First char is %0x\n", addr[0]); // Need to read/write a byte for it to be used
    while(1) {
        getchar();
    }

    return EXIT_SUCCESS;
}

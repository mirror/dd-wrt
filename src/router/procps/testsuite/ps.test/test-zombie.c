/*
 * test-zombie - Test program to make a zombie
 *
 * Copyright ©     -2024 Craig Small <csmall@dropbear.xyz>
 *
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, const char *argv[])
{
    pid_t child_pid;

    child_pid = fork();

    switch (child_pid) {
        case 0:
            // Child process - try to exit
            exit(EXIT_SUCCESS);
        case -1:
            perror("fork() call failed");
            exit(EXIT_FAILURE);
        default:
            // Parent process
            printf("Zombie pid: %d\n", child_pid);
            while(1)
                sleep(30);
    }
    return (EXIT_SUCCESS);
}





/**
 * @file main_ni_only.c
 * @brief non-interactive implementation of main() for those platforms without the linenoise library
 *
 * Copyright (c) 2015-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

int main_ni(int argc, char *argv[]);

int done; /* for cmd.c */

int
main(int argc, char *argv[])
{
    return main_ni(argc, argv);
}

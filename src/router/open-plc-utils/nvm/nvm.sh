#!/bin/sh
# file: nvm/nvm.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -o chknvm chknvm.c
gcc -Wall -o chknvm2 chknvm2.c
gcc -Wall -o nvmmerge nvmmerge.c
gcc -Wall -o nvmsplit nvmsplit.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -c fdmanifest.c
gcc -Wall -c manifest.c
gcc -Wall -c nvmfile.c
gcc -Wall -c nvmfile1.c
gcc -Wall -c nvmfile2.c
gcc -Wall -c nvmpeek.c
gcc -Wall -c nvmpeek1.c
gcc -Wall -c nvmpeek2.c
gcc -Wall -c nvmseek1.c
gcc -Wall -c nvmseek2.c

# ====================================================================
# cleanse;
# --------------------------------------------------------------------

rm -f *.o


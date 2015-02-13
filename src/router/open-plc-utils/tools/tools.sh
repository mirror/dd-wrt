#!/bin/sh
# file: tools/tools.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c binout.c
gcc -Wall -Wextra -Wno-unused-parameter -c checkfilename.c
gcc -Wall -Wextra -Wno-unused-parameter -c checksum32.c
gcc -Wall -Wextra -Wno-unused-parameter -c clr32bitmap.c
gcc -Wall -Wextra -Wno-unused-parameter -c codelist.c
gcc -Wall -Wextra -Wno-unused-parameter -c codename.c
gcc -Wall -Wextra -Wno-unused-parameter -c debug.c
gcc -Wall -Wextra -Wno-unused-parameter -c decout.c
gcc -Wall -Wextra -Wno-unused-parameter -c emalloc.c
gcc -Wall -Wextra -Wno-unused-parameter -c error.c
gcc -Wall -Wextra -Wno-unused-parameter -c extra.c
gcc -Wall -Wextra -Wno-unused-parameter -c fdchecksum32.c
gcc -Wall -Wextra -Wno-unused-parameter -c getoptv.c
gcc -Wall -Wextra -Wno-unused-parameter -c getargv.c
gcc -Wall -Wextra -Wno-unused-parameter -c hexencode.c
gcc -Wall -Wextra -Wno-unused-parameter -c hexdecode.c
gcc -Wall -Wextra -Wno-unused-parameter -c hexdump.c
gcc -Wall -Wextra -Wno-unused-parameter -c hexload.c
gcc -Wall -Wextra -Wno-unused-parameter -c hexpeek.c
gcc -Wall -Wextra -Wno-unused-parameter -c hexstring.c
gcc -Wall -Wextra -Wno-unused-parameter -c hexview.c
gcc -Wall -Wextra -Wno-unused-parameter -c hexwrite.c
gcc -Wall -Wextra -Wno-unused-parameter -c memdecr.c
gcc -Wall -Wextra -Wno-unused-parameter -c memencode.c
gcc -Wall -Wextra -Wno-unused-parameter -c memincr.c
gcc -Wall -Wextra -Wno-unused-parameter -c memswap.c
gcc -Wall -Wextra -Wno-unused-parameter -c memout.c
gcc -Wall -Wextra -Wno-unused-parameter -c putoptv.c
gcc -Wall -Wextra -Wno-unused-parameter -c regview32.c
gcc -Wall -Wextra -Wno-unused-parameter -c reword.c
gcc -Wall -Wextra -Wno-unused-parameter -c set32bitmap.c
gcc -Wall -Wextra -Wno-unused-parameter -c synonym.c
gcc -Wall -Wextra -Wno-unused-parameter -c todigit.c
gcc -Wall -Wextra -Wno-unused-parameter -c typelist.c
gcc -Wall -Wextra -Wno-unused-parameter -c typename.c
gcc -Wall -Wextra -Wno-unused-parameter -c uintspec.c
gcc -Wall -Wextra -Wno-unused-parameter -c version.c

# ====================================================================
# cleanse;  
# --------------------------------------------------------------------

rm -f *.o


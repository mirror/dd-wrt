#!/bin/sh
# file: pib/pib.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -o chkpib chkpib.c
gcc -Wall -Wextra -Wno-unused-parameter -o chkpib2 chkpib2.c
gcc -Wall -Wextra -Wno-unused-parameter -o getpib getpib.c
gcc -Wall -Wextra -Wno-unused-parameter -o modpib modpib.c
gcc -Wall -Wextra -Wno-unused-parameter -o pibrump pibrump.c
gcc -Wall -Wextra -Wno-unused-parameter -o pibruin pibruin.c
gcc -Wall -Wextra -Wno-unused-parameter -o pib2xml pib2xml.c
gcc -Wall -Wextra -Wno-unused-parameter -o pibdump pibdump.c
gcc -Wall -Wextra -Wno-unused-parameter -o pibcomp pibcomp.c
gcc -Wall -Wextra -Wno-unused-parameter -o pskey pskey.c
gcc -Wall -Wextra -Wno-unused-parameter -o psin psin.c
gcc -Wall -Wextra -Wno-unused-parameter -o psout psout.c -lm
gcc -Wall -Wextra -Wno-unused-parameter -o psnotch psnotch.c
gcc -Wall -Wextra -Wno-unused-parameter -o psgraph psgraph.c
gcc -Wall -Wextra -Wno-unused-parameter -o setpib setpib.c
gcc -Wall -Wextra -Wno-unused-parameter -o xml2pib xml2pib.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c gpioinfo.c
gcc -Wall -Wextra -Wno-unused-parameter -c pibfile.c
gcc -Wall -Wextra -Wno-unused-parameter -c pibfile1.c
gcc -Wall -Wextra -Wno-unused-parameter -c pibfile2.c
gcc -Wall -Wextra -Wno-unused-parameter -c pibpeek1.c
gcc -Wall -Wextra -Wno-unused-parameter -c pibpeek2.c
gcc -Wall -Wextra -Wno-unused-parameter -c piblock.c
gcc -Wall -Wextra -Wno-unused-parameter -c pibscalers.c
gcc -Wall -Wextra -Wno-unused-parameter -c psread.c
gcc -Wall -Wextra -Wno-unused-parameter -c qosinfo.c
gcc -Wall -Wextra -Wno-unused-parameter -c ruledump.c 

# ====================================================================
# cleanse;
# --------------------------------------------------------------------

rm -f *.o


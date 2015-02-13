#!/bin/sh
# file: ram/ram.sh 

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -o config2cfg config2cfg.c 
gcc -Wall -Wextra -Wno-unused-parameter -o sdram sdram.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c nvrampeek.c 
gcc -Wall -Wextra -Wno-unused-parameter -c nvram.c 
gcc -Wall -Wextra -Wno-unused-parameter -c sdramfile.c 
gcc -Wall -Wextra -Wno-unused-parameter -c sdrampeek.c 
gcc -Wall -Wextra -Wno-unused-parameter -c sdramtext.c 
gcc -Wall -Wextra -Wno-unused-parameter -c nvram.c 
gcc -Wall -Wextra -Wno-unused-parameter -c nvrampeek.c 
gcc -Wall -Wextra -Wno-unused-parameter -c sdramfile.c 
gcc -Wall -Wextra -Wno-unused-parameter -c sdramfileA.c 
gcc -Wall -Wextra -Wno-unused-parameter -c sdrampeek.c 

# ====================================================================
# cleanse;  
# --------------------------------------------------------------------

rm -f *.o


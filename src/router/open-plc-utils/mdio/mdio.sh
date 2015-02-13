#!/bin/sh
# file: mdio/mdio.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -o mdioblock mdioblock.c
gcc -Wall -Wextra -Wno-unused-parameter -o mdioblock2 mdioblock2.c 
gcc -Wall -Wextra -Wno-unused-parameter -o mdiodump mdiodump.c
gcc -Wall -Wextra -Wno-unused-parameter -o mdiogen mdiogen.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

# ====================================================================
# cleanse;
# --------------------------------------------------------------------

rm -f *.o


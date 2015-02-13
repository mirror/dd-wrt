#!/bin/sh  

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -o int6kuart int6kuart.c
gcc -Wall -Wextra -Wno-unused-parameter -o int6kbaud int6kbaud.c
gcc -Wall -Wextra -Wno-unused-parameter -o ttysig ttysig.c
gcc -Wall -Wextra -Wno-unused-parameter -o ttysend ttysend.c
gcc -Wall -Wextra -Wno-unused-parameter -o ttyrecv ttyrecv.c
gcc -Wall -Wextra -Wno-unused-parameter -o weeder weeder.c
gcc -Wall -Wextra -Wno-unused-parameter -o ptsctl ptsctl.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c serial.c
gcc -Wall -Wextra -Wno-unused-parameter -c openport.c
gcc -Wall -Wextra -Wno-unused-parameter -c closeport.c
gcc -Wall -Wextra -Wno-unused-parameter -c baudrate.c

# ====================================================================
# cleanse;   
# --------------------------------------------------------------------

rm -f *.o


#!/bin/sh
# file: mme/mme.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -o hpav hpav.c
gcc -Wall -Wextra -Wno-unused-parameter -o mme mme.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c EthernetHeader.c
gcc -Wall -Wextra -Wno-unused-parameter -c FragmentHeader.c
gcc -Wall -Wextra -Wno-unused-parameter -c HomePlugHeader.c
gcc -Wall -Wextra -Wno-unused-parameter -c QualcommHeader.c
gcc -Wall -Wextra -Wno-unused-parameter -c MMEMode.c
gcc -Wall -Wextra -Wno-unused-parameter -c MMEName.c
gcc -Wall -Wextra -Wno-unused-parameter -c MMEPeek.c
gcc -Wall -Wextra -Wno-unused-parameter -c MMECode.c
gcc -Wall -Wextra -Wno-unused-parameter -c ARPCPrint.c
gcc -Wall -Wextra -Wno-unused-parameter -c ARPCWrite.c
gcc -Wall -Wextra -Wno-unused-parameter -c UnwantedMessage.c
gcc -Wall -Wextra -Wno-unused-parameter -c FirmwareMessage.c
gcc -Wall -Wextra -Wno-unused-parameter -c readmessage.c
gcc -Wall -Wextra -Wno-unused-parameter -c sendmessage.c

# ====================================================================
# cleanse;
# --------------------------------------------------------------------

rm -f *.o


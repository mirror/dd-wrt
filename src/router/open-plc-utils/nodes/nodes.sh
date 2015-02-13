#!/bin/sh
#file: nodes/nodes.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c xmledit.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlfree.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlnode.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlopen.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlread.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlscan.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmltree.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlschema.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlelement.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlattribute.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlvalue.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmldata.c
gcc -Wall -Wextra -Wno-unused-parameter -c xmlselect.c

# ====================================================================
# cleanse;  
# --------------------------------------------------------------------

rm -f *.o


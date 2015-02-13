# file: key/key.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -o hpavkey hpavkey.c
gcc -Wall -Wextra -Wno-unused-parameter -o hpavkeys hpavkeys.c
gcc -Wall -Wextra -Wno-unused-parameter -o mac2pw mac2pw.c
gcc -Wall -Wextra -Wno-unused-parameter -o mac2pw mac2pwd.c
gcc -Wall -Wextra -Wno-unused-parameter -o rkey rkey.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c HPAVKeyDAK.c
gcc -Wall -Wextra -Wno-unused-parameter -c HPAVKeyNID.c
gcc -Wall -Wextra -Wno-unused-parameter -c HPAVKeyNMK.c
gcc -Wall -Wextra -Wno-unused-parameter -c HPAVKeySHA.c
gcc -Wall -Wextra -Wno-unused-parameter -c HPAVKeyOut.c
gcc -Wall -Wextra -Wno-unused-parameter -c HPAVKeySpec.c
gcc -Wall -Wextra -Wno-unused-parameter -c MACPasswords.c
gcc -Wall -Wextra -Wno-unused-parameter -c RNDPasswords.c
gcc -Wall -Wextra -Wno-unused-parameter -c SHA256Block.c
gcc -Wall -Wextra -Wno-unused-parameter -c SHA256Fetch.c
gcc -Wall -Wextra -Wno-unused-parameter -c SHA256Reset.c
gcc -Wall -Wextra -Wno-unused-parameter -c SHA256Write.c
gcc -Wall -Wextra -Wno-unused-parameter -c SHA256Print.c
gcc -Wall -Wextra -Wno-unused-parameter -c SHA256Ident.c
gcc -Wall -Wextra -Wno-unused-parameter -c SHA256Match.c
gcc -Wall -Wextra -Wno-unused-parameter -c keys.c
gcc -Wall -Wextra -Wno-unused-parameter -c putpwd.c 
gcc -Wall -Wextra -Wno-unused-parameter -c strnpwd.c 

# ====================================================================
# cleanse;
# --------------------------------------------------------------------

rm -f *.o

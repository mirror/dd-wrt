cmd_libbb/lineedit_ptr_hack.o := arm-linux-uclibc-gcc -Wp,-MD,libbb/.lineedit_ptr_hack.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.19.2)" -DBB_BT=AUTOCONF_TIMESTAMP -Os -pipe -march=armv6k -mtune=mpcore -mfloat-abi=softfp -mfpu=vfp -fno-caller-saves -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os  -DNEED_PRINTF -Os -pipe -march=armv6k -mtune=mpcore -mfloat-abi=softfp -mfpu=vfp -fno-caller-saves    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(lineedit_ptr_hack)"  -D"KBUILD_MODNAME=KBUILD_STR(lineedit_ptr_hack)" -c -o libbb/lineedit_ptr_hack.o libbb/lineedit_ptr_hack.c

deps_libbb/lineedit_ptr_hack.o := \
  libbb/lineedit_ptr_hack.c \

libbb/lineedit_ptr_hack.o: $(deps_libbb/lineedit_ptr_hack.o)

$(deps_libbb/lineedit_ptr_hack.o):

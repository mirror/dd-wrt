#! /bin/sh

if test -z "$AWK"; then
  AWK=awk
fi
if test -z "$srcdir"; then
  srcdir=.
fi

LC_ALL=C
export LC_ALL

rm -f term.h
cat << EOF > term.h
/*
 * This file is automagically created from term.c -- DO NOT EDIT
 */

#define T_FLG 0
#define T_NUM 1
#define T_STR 2

struct term
{
  char *tcname;
  int type;
};

union tcu
{
  int flg;
  int num;
  char *str;
};

EOF

#
# SCO-Unix sufferers may need to use the following lines:
# perl -p < ${srcdir}/term.c \
#  -e 's/"/"C/ if /"[A-Z]."/;' \
#  -e 'y/[a-z]/[A-Z]/ if /"/;' \
#
sed < ${srcdir}/term.c \
  -e '/"[A-Z]."/s/"/"C/' \
  -e '/"/y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/' \
| $AWK '
/^  [{] ".*KMAPDEF[(].*$/{
  if (min == 0) min = s
  max = s;
}
/^  [{] ".*KMAPADEF[(].*$/{
  if (amin == 0) amin = s
  amax = s;
}
/^  [{] ".*KMAPMDEF[(].*$/{
  if (mmin == 0) mmin = s
  mmax = s;
}
/^  [{] ".*$/{
a=substr($2,2,length($2)-3);
b=substr($3,3,3);
if (nolist == 0) {
    printf "#define d_%s  d_tcs[%d].%s\n",a,s,b
    printf "#define D_%s (D_tcs[%d].%s)\n",a,s,b
  }
s++;
}
/\/* define/{
printf "#define %s %d\n",$3,s
}
/\/* nolist/{
nolist = 1;
}
/\/* list/{
nolist = 0;
}
END {
  printf "\n#ifdef MAPKEYS\n"
  printf "#  define KMAPDEFSTART %d\n", min
  printf "#  define NKMAPDEF %d\n", max-min+1
  printf "#  define KMAPADEFSTART %d\n", amin
  printf "#  define NKMAPADEF %d\n", amax-amin+1
  printf "#  define KMAPMDEFSTART %d\n", mmin
  printf "#  define NKMAPMDEF %d\n", mmax-mmin+1
  printf "#endif\n"
}
' | sed -e s/NUM/num/ -e s/STR/str/ -e s/FLG/flg/ \
>> term.h

rm -f kmapdef.c
cat << EOF > kmapdef.c
/*
 * This file is automagically created from term.c -- DO NOT EDIT
 */

#include "config.h"

#ifdef MAPKEYS

EOF

$AWK < ${srcdir}/term.c '
/^  [{] ".*KMAP.*$/{
  for (i = 0; i < 3; i++) {
    q = $(5+i)
    if (substr(q, 1, 5) == "KMAPD") {
      if (min == 0) min = s
      max = s
      arr[s] = substr(q, 9, length(q)-9)
    }
    if (substr(q, 1, 5) == "KMAPA") {
      if (amin == 0) amin = s
      amax = s
      anarr[s] = substr(q, 10, length(q)-10)
    }
    if (substr(q, 1, 5) == "KMAPM") {
      if (mmin == 0) mmin = s
      mmax = s
      mnarr[s] = substr(q, 10, length(q)-10)
    }
  }
}
/^  [{] ".*$/{
  s++;
}
END {
  printf "char *kmapdef[] = {\n"
  for (s = min; s <= max; s++) {
    if (arr[s])
      printf "%s", arr[s]
    else
      printf "0"
    if (s < max)
      printf ",\n"
    else
      printf "\n"
  }
  printf "};\n\n"
  printf "char *kmapadef[] = {\n"
  for (s = amin; s <= amax; s++) {
    if (anarr[s])
      printf "%s", anarr[s]
    else
      printf "0"
    if (s < amax)
      printf ",\n"
    else
      printf "\n"
  }
  printf "};\n\n"
  printf "char *kmapmdef[] = {\n"
  for (s = mmin; s <= mmax; s++) {
    if (mnarr[s])
      printf "%s", mnarr[s]
    else
      printf "0"
    if (s < mmax)
      printf ",\n"
    else
      printf "\n"
  }
  printf "};\n\n#endif\n"
}
' >> kmapdef.c

chmod a-w kmapdef.c
chmod a-w term.h


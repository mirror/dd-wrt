dnl modified AC_C_INLINE from autoconf/c.m4 by TWISTI

AN_IDENTIFIER([attribute], [AC_C_ATTRIBUTE])
AC_DEFUN([AC_C_ATTRIBUTE],
[AC_CACHE_CHECK([for __attribute__], ac_cv_c_attribute,
[
AC_COMPILE_IFELSE([AC_LANG_SOURCE(
[void foo(void) __attribute__ ((__noreturn__));]
)],
[ac_cv_c_attribute=yes],
[ac_cv_c_attribute=no]
)
])
AH_VERBATIM([attribute],
[/* Define to `__attribute__' to nothing if it's not supported.  */
#ifndef __cplusplus
#undef __attribute__
#endif])
case $ac_cv_c_attribute in
  yes) ;;
  no)
    cat >>confdefs.h <<_ACEOF
#ifndef __cplusplus
#define __attribute__(x)    /* nothing */
#endif
_ACEOF
    ;;
esac
])# AC_C_ATTRIBUTE


#check how to do asm(".skip 16")

AN_IDENTIFIER([skip16], [AC_ASM_SKIP16])
AC_DEFUN([AC_ASM_SKIP16],
[AC_MSG_CHECKING([if and how we can waste code space])
if test -z "$skipcode"; then
    skipcode=no
    CFLAGS_1="$CFLAGS"
    CFLAGS="$CFLAGS $ENGINE_FLAGS"
    for i in ".skip 16" ".block 16" ".org .+16" ".=.+16" ".space 16"
    do
	AC_TRY_RUN(
[int foo(int,int,int);
main()
{
  exit(foo(0,0,0)!=16);
}
int foo(int x, int y, int z)
{
  static void *labels[]={&&label1, &&label2};
  if (x) {
    y++; /* workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=12108 */
  label1:
    __asm__("$i"); /* or ".space 16" or somesuch */
  label2: ;
  }
  {
  if (y) goto *labels[z]; /* workaround for gcc PR12108 */
  return labels[1]-labels[0];
  }
}]
	,skipcode=$i; break
	,,)
    done
    CFLAGS=$CFLAGS_1
fi
AC_MSG_RESULT($skipcode)
if test "$skipcode" = no
then 
    if test -z $no_dynamic_default; then
	no_dynamic_default=1
	AC_MSG_WARN(Disabling default dynamic native code generation)
    fi
    AC_DEFINE_UNQUOTED(SKIP16,((void)0),statement for skipping 16 bytes)
else
    AC_DEFINE_UNQUOTED(SKIP16,__asm__("$skipcode"),statement for skipping 16 bytes)
fi
])
/* This file allows libmpsse to depend on an old version of GLibC's memcpy.
 * Without this, we depend on version 2.14 which is too new for some of our
 * customers.
 * This solution is copied from:
 * http://stackoverflow.com/questions/8823267/linking-against-older-symbol-version-in-a-so-file/8862631#8862631
 * and requires the linker option --wrap=memcpy
 */

#include <string.h>

#if !defined(ANDROID) && !defined(_OSX_) && (defined(__LP64__) || defined(_LP64)) && \
    defined(__x86_64__) && !defined(__ILP32__) && defined(__GNU_LIBRARY__)
// Most other GLibC functions are version 2.2.5 in 64 bit builds
asm (".symver memcpy, memcpy@GLIBC_2.2.5");

void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

#else
// For all other architectures, just use the real memcpy.

void *__real_memcpy(void *dest, const void *src, size_t n);

void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
    return __real_memcpy(dest, src, n);
}
#endif // !ANDROID && !_OSX_ && (__LP64__ || _LP64)

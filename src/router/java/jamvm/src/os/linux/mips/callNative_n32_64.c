#include "config.h"
#include "../../../jam.h"

#if !defined(USE_FFI)
#if defined(_ABIN32) || defined(_ABI64)

#if !defined(MIPSEB)
#error "Little-endian was not tested and most probably will not work"
#endif

#ifdef _ABIN32
#define FPAD(x) (x)
#else
#define FPAD(x) ((char *)x + 4)
#endif

typedef union {
    long long l;
    struct {
        float f1;
        float f2;
    } _f;
    double d;
} arg_t;

#define ARGS arg_t, arg_t, arg_t, arg_t, arg_t, arg_t, arg_t, arg_t
#define CALL(type) ((type (*)(ARGS))fn)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7])

#if defined(__mips_hard_float)

#define USEFLT(x,sfx,code...) do { \
    if (args == 2)      { asm ("mov." sfx " $f14, %0\n\t" : : "f"(x) : "$f14"); } \
    else if (args == 3) { asm ("mov." sfx " $f15, %0\n\t" : : "f"(x) : "$f15"); } \
    else if (args == 4) { asm ("mov." sfx " $f16, %0\n\t" : : "f"(x) : "$f16"); } \
    else if (args == 5) { asm ("mov." sfx " $f17, %0\n\t" : : "f"(x) : "$f17"); } \
    else if (args == 6) { asm ("mov." sfx " $f18, %0\n\t" : : "f"(x) : "$f18"); } \
    else if (args == 7) { asm ("mov." sfx " $f19, %0\n\t" : : "f"(x) : "$f19"); } \
    else { code; } } while (0)

#else

#define USEFLT(x,sfx,code...) \
    do { code; } while (0)

#endif /* __mips_hard_float */

uintptr_t *callJNIMethod(void *env, Class *class, char *sig, int extra,
                         uintptr_t *ostack, unsigned char *fn, int args)
{
    arg_t stack[extra / sizeof(arg_t)];
    uintptr_t *sstack = ostack;
    arg_t *p, a[8] = {(long)env, class ? (long)class : (long)(*ostack++)};
    double d;
    float f;

    for (args = 2, sig += 1; *sig != ')'; args++, sig++) {
        p = (args < 8) ? a + args : stack + (args - 8);
        switch (*sig) {
        case '[':
            while (*++sig == '[');
            args--;
            break;
        case 'L':
            p->l = *ostack++;
            while (*++sig != ';');
            break;
        case 'F':
            f = *((float *)FPAD(ostack));
            USEFLT(f, "s", p->_f.f1 = f, p->_f.f2 = f);
            ostack++;
            break;
        case 'J':
            p->l = *((long long *)ostack);
            ostack += 2;
            break;
        case 'D':
            d = *((double *)ostack);
            USEFLT(d, "d", p->d = d);
            ostack += 2;
            break;
        default:
            p->l = *ostack++;
            break;
        }
    }

    switch (*++sig) {
    case 'V':
        CALL(void);
        break;
    case 'F':
        ((arg_t *)(FPAD(sstack)))->_f.f1 = CALL(float);
        sstack++;
        break;
    case 'J':
        ((arg_t *)sstack)->l = CALL(long long);
        sstack += 2;
        break;
    case 'D':
        ((arg_t *)sstack)->d = CALL(double);
        sstack += 2;
        break;
    default:
        *sstack++ = CALL(int);
        break;
    }

    return sstack;
}
#endif /* _ABIN32 || _ABI64 */
#endif /* USE_FFI */

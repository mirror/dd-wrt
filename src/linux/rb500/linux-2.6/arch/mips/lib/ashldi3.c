#include <linux/libgcc.h>
#include <linux/module.h>

#ifdef ARCH_NEEDS_ashldi3

DWtype __ashldi3(DWtype u, word_type b)
{
       DWunion uu, w;
       word_type bm;

       if (b == 0)
               return u;

       uu.ll = u;
       bm = (sizeof(Wtype) * BITS_PER_UNIT) - b;

       if (bm <= 0) {
               w.s.low = 0;
               w.s.high = (UWtype) uu.s.low << -bm;
       } else {
               const UWtype carries = (UWtype) uu.s.low >> bm;

               w.s.low = (UWtype) uu.s.low << b;
               w.s.high = ((UWtype) uu.s.high << b) | carries;
       }

       return w.ll;
}

EXPORT_SYMBOL(__ashldi3);

#endif /* ARCH_NEEDS_ashldi3 */

#include <linux/libgcc.h>
#include <linux/module.h>

/* Unless shift functions are defined with full ANSI prototypes,
   parameter b will be promoted to int if word_type is smaller than an int.  */
#ifdef ARCH_NEEDS_ashrdi3

DWtype __ashrdi3(DWtype u, word_type b)
{
       DWunion uu, w;
       word_type bm;

       if (b == 0)
               return u;

       uu.ll = u;
       bm = (sizeof(Wtype) * BITS_PER_UNIT) - b;

       if (bm <= 0) {
               /* w.s.high = 1..1 or 0..0 */
               w.s.high =
                   uu.s.high >> (sizeof(Wtype) * BITS_PER_UNIT - 1);
               w.s.low = uu.s.high >> -bm;
       } else {
               const UWtype carries = (UWtype) uu.s.high << bm;

               w.s.high = uu.s.high >> b;
               w.s.low = ((UWtype) uu.s.low >> b) | carries;
       }

       return w.ll;
}

EXPORT_SYMBOL(__ashrdi3);

#endif /* ARCH_NEEDS_ashrdi3 */

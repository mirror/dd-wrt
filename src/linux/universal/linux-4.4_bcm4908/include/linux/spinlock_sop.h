/* 
 * Since we need both UP & SMP version of spinlock implementaions in 
 * SOP platform, new spinlock APIs are introduced.
 * - The standard spinlock API will be of UP version.
 * - The standard API will be suffixed with 'smp' when there is no change in
 *   the original SMP implementation else it will be suffixed with 'sop'.
 */ 

#include <asm/spinlock_types_sop.h>
#include <asm/spinlock_sop.h>

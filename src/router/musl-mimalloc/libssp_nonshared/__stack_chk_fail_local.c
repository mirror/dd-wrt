#include "atomic.h"
void __attribute__((visibility ("hidden"))) __stack_chk_fail_local(void) { a_crash(); }

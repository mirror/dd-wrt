
#define VSTR_COMPILE_INCLUDE 1
#include <vstr.h>

#define EXIT_FAILED_OK 77

int main(void)
{
  unsigned long magic_num = 0xF0F0;
  unsigned long val       = 1;
  
  if (!vstr_cntl_opt(666, magic_num, val))
    exit (EXIT_FAILED_OK);

  if (vstr_init()) /* should fail */
    exit (EXIT_FAILURE);
  
  exit (EXIT_SUCCESS);  
}

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include <output.h>

int main()
{
  unsigned int const	NUMS[] = { 0, 2, 42, 529, 1073741824,
				   2147483648u, 3141592653u, 4294967295u };
  size_t		i;

  for (i=0; i<sizeof(NUMS)/sizeof(NUMS[0]); ++i) {
    writeUInt(1, NUMS[i]); write(1, "\n", 1);
  }

  return EXIT_SUCCESS;
}

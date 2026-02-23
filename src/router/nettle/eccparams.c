#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char **argv)
{
  unsigned bits;
  unsigned max;
  unsigned c;
  if (argc < 3)
    {
    usage:
      fprintf(stderr, "Usage: %s: exp-bits max-entries\n", argv[0]);
      return EXIT_FAILURE;
    }
  bits = atoi(argv[1]);
  if (bits < 2)
    goto usage;
  max = atoi(argv[2]);
  if ( max < 2)
    goto usage;

  for (c = 3; (1<<c) <= max; c++)
    {
      unsigned b;
      for (b = 1;; b++)
	{
	  unsigned s = (1<<c) * b;
	  unsigned k;
	  if (s > max)
	    break;
	  k = (bits + (c*b) - 1) / (c * b);
	  printf("k = %2u, c = %2u, S = %3u, T = %3u (%3u A + %2u D)\n",
		 k, c, s, (b+1)*k, b*k, k);
	}
    }
  return 0;
}

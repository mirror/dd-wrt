#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "vfs.h"


unsigned int
getsize (char *name)
{
  FILE *in;
  unsigned int s;
  in = fopen (name, "rb");
  fseek (in, 0, SEEK_END);
  s = ftell (in);
  fclose (in);
  return s;
}

void
copy (char *name, FILE * out)
{
  FILE *in;
  int i;
  int s = getsize (name);
  in = fopen (name, "rb");
  for (i = 0; i < s; i++)
    putc (getc (in), out);
  fclose (in);
}

void
main (int argc, char *argv[])
{
  header h;
  FILE *out;
  h.sign[0] = 'D';
  h.sign[1] = 'D';
  h.sign[2] = '-';
  h.sign[3] = 'W';
  h.sign[4] = 'R';
  h.sign[5] = 'T';
  h.noe = argc - 2;
  out = fopen (argv[1], "wb");
  fwrite (&h, 10, 1, out);
  int i;
  int s;
  int curpos = 10;
  for (i = 0; i < argc - 2; i++)
    {
      curpos += 1;		//namelen
      curpos += strlen (argv[i + 2]);
      curpos += 8;		//fileoffset filesize
    }
  for (i = 0; i < argc - 2; i++)
    {
      char *name = argv[i + 2];
      entry e;
      e.namelen = strlen (name);
      putw (e.namelen, out);
      fwrite (name, e.namelen, 1, out);
      fwrite (&curpos, 4, 1, out);
      s = getsize (name);
      fwrite (&s, 4, 1, out);
      curpos += s;
    }

  for (i = 0; i < argc - 2; i++)
    {
      copy (argv[i + 2], out);
    }
  fclose (out);
}

#include <stdio.h>
#include <malloc.h>



int filter (char *m, char *source, char *dest, int len);

int
main (int argc, char *argv[])
{
  int i;
  char *m;
  int a;
//fprintf(stderr,"argc = %d\n",argc);
  if (argc == 1)
    return 0;
  for (a = 1; a < argc; a++)
    {
      printf ("reading %s\n", argv[a]);
      FILE *in = fopen (argv[a], "rb");
      if (in == NULL)
	return 0;
      fseek (in, 0, SEEK_END);
      int len = ftell (in);
      m = (char *) malloc (len);
      fseek (in, 0, SEEK_SET);
      for (i = 0; i < len; i++)
	m[i] = getc (in);


      fclose (in);


      len = filter (m, "<input type=", "<i>", len);
      len = filter (m, "<input class=", "<c>", len);
      len = filter (m, "<input id=", "<d>", len);
      len = filter (m, "<input name=", "<z>", len);
      len = filter (m, "<script type=\"text/javascript\">", "<m>", len);
      len = filter (m, "<div class=", "<e>", len);
      len = filter (m, "<div id=", "<n>", len);
      len = filter (m, "<a href=\"", "<j>", len);
      len = filter (m, "<option value=", "<o>", len);
      len = filter (m, "<select name=", "<s>", len);
      len = filter (m, "<span class=", "<u>", len);
      printf ("writing \n");

      in = fopen (argv[a], "wb");
      fwrite (m, len, 1, in);
      free (m);
//printf("closing\n");
      fclose (in);
//printf("next\n");
    }
  return 0;
}

int
filter (char *m, char *source, char *dest, int len)
{
  int i;
  int disable = 0;
//printf("filter %s to %s\n",source,dest);
  for (i = 0; i < len - strlen (source); i++)
    {
      if (disable == 1)
	{
	  if (strncmp ((char *) &m[i], "%>",2) == 0)
	    disable = 0;
	}
      else
	{
	  if (strncmp ((char *) &m[i], "<%",2) == 0)
	    disable = 1;
	}
      if (!disable)
	if (strncmp ((char *) &m[i], source, strlen (source)) == 0)
	  {
	    memcpy (&m[i], dest, strlen (dest));
	    int delta = strlen (source) - strlen (dest);
//      printf("len = %d\n",len);
	    memcpy (&m[i + strlen (dest)], &m[i + strlen (source)],
		    len - (i + strlen (source)));
	    len -= delta;
	    i += strlen (dest);
	  }
    }
  m[i] = 0;
  return len;
}

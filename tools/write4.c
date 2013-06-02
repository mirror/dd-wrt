#include <stdio.h>
#include <string.h>
#include <malloc.h>



int filter (char *m, char *source, char *dest, int len);

int
main (int argc, char *argv[])
{
  int i;
  char *m;
  int a;

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

      len = filter (m, "<input type=", "{i}", len);
      len = filter (m, "<input class=", "{c}", len);
      len = filter (m, "<input id=", "{d}", len);
      len = filter (m, "<input name=", "{z}", len);
      len = filter (m, "<script type=\"text/javascript\">", "{m}", len);
      len = filter (m, "<div class=", "{e}", len);
      len = filter (m, "<div id=", "{n}", len);
      len = filter (m, "<a href=\"", "{j}", len);
      len = filter (m, "<option value=", "{o}", len);
      len = filter (m, "<select name=", "{s}", len);
      len = filter (m, "<span class=", "{u}", len);
      len = filter (m, "document.write(\"", "{x}", len);
      len = filter (m, "document.(\"", "{y}", len);
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
  int len_src = strlen (source);
  int len_dest =  strlen (dest);
  char *tmp = (char *) malloc (len);
  
  
  if (len_src > len)
    return len;

  for (i = 0; i < len - len_src; i++)
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
	if (strncmp ((char *) &m[i], source, len_src) == 0)
	  {	    
	    memcpy (&m[i], dest, len_dest);
	    memcpy(tmp, m , len);
	    
	    int delta = len_src - len_dest;	    
	    memcpy (&m[i + len_dest], &tmp[i + len_src], len - (i + len_src));
	    len -= delta;
	    i += len_dest;
	  }
    }
    free(tmp);
//  m[len] = 0;
  return len;
}

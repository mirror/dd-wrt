/* broken example ...
 * code supposedly OK because the input and the output are the same
 * maximum size */

int func(const char *src)
{
  char buf[X];
  unsigned int count = 1;

  strcpy(buf, src);

  strtok(buf, ",");
  while (strtok(NULL, ","))
  {
    ++count;
  }

  return (count);
}

/* callsite 1... */
char buf[X];
func(buf);

/* callsite 2... */
func("a,b,c");

/* callsite N... */
func(tmp);

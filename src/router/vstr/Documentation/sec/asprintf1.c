/* broken code, on Linux only */
char *s1;
int off;

asprintf(&s1, "%d:%n%s,", strlen(data), &off, data);
if (!s1) goto error;

...
if (strlen(other_data) == strlen(data))
  strcpy(s1 + off, other_data);
else
{
  free(s1);
  asprintf(&s1, "%d:%s,", strlen(other_data), other_data);
}

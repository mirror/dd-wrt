/* broken example ...
 * code supposedly OK because the input and the output are the same
 * maximum size */
char buf1[X];
char buf2[X];
char buf3[X];
char  *s1 = buf1;
char  *s2 = buf2;
char  *s3 = buf3;
char  *last = s2;
...
/* find biggest shared prefix, and skip that */
while (*s1 && (*s1 == *s2))
{
  if (*s2++ == '/')
    last = s2;
  ++s1;
}

/* relative point to shared part of path */
while (*s1)
{
  if (*s1++ == '/')
  {
     memcpy(s3, "../", strlen("../"));
     s3             += strlen("../");
  }
}
strcpy(s3, last);

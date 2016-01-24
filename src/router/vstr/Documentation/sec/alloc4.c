/* broken example ...
 * code supposedly OK due to count */
int func(char *dst, size_t len, const char *src)
{
  while (*src)
  {
    const char *tmp = NULL;

    if ((src[0] == '$') && (src[1] == '{') && (tmp = strchr(src, '}')))
    { /* sub a variable */
      size_t var_len = (tmp - (src + 2));
      size_t sub_len = 0;

      src += strlen("${");

      if ((tmp = lookup(src, var_len)) && (sub_len = strlen(tmp)))
      {
        if (sub_len > len)
          sub_len = len;

        memcpy(dst, tmp, sub_len);
        len -= sub_len;
        dst += sub_len;
      }

      src += var_len + 1;
    }
    else /* no substitution */
      *dst++ = *src++;
  }

  if (len)
  {
    *dst = 0;
    return (1);
  }

  return (0);
}

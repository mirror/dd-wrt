extern struct variable variables[];
																																																																																																																																																																			    /* channel info structure *///from 11.9

/* Hook to write wl_* default set through to wl%d_* variable set */
void
wl_unit (webs_t wp, char *value, struct variable *v)
{
  char tmp[100], prefix[] = "wlXXXXXXXXXX_";
  struct nvram_tuple cur[100], *last = cur, *t;

  /* Do not write through if no interfaces are present */
  if (atoi (value) < 0)
    return;

  /* Set prefix */
  snprintf (prefix, sizeof (prefix), "wl%d_", atoi (value));

  /* Write through to selected variable set */
  for (; v >= variables && !strncmp (v->name, "wl_", 3); v--)
    {
      /* Do not interleave get and set (expensive on Linux) */
      a_assert (last < &cur[ARRAYSIZE (cur)]);
      last->name = v->name;
      last->value = nvram_safe_get (v->name);
      last++;
    }

  for (t = cur; t < last; t++)
    nvram_set (strcat_r (prefix, &t->name[3], tmp), t->value);
}

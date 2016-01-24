/* assuming vstr_num(s1, 1, s1->len) == 4 right now, from above code... */

 /* add some more data via. copying it */
 vstr_add_cstr_buf(s1, 4, "abcd");

/* now, still: vstr_num(s1, 1, s1->len) == 4 */

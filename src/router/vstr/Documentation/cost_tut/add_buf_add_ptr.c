/* assuming vstr_num(s1, 1, s1->len) == 0 right now... */

 /* add some data via. copying it */
 vstr_add_cstr_buf(s1, s1->len, "abcd");

/* now: vstr_num(s1, 1, s1->len) == 1 */

 /* add some data via. pointing to it */
 vstr_add_cstr_ptr(s1, s1->len, "abcd");

 /* add some data via. copying it */
 vstr_add_cstr_buf(s1, s1->len, "abcd");

 /* add some data via. pointing to it */
 vstr_add_cstr_ptr(s1, s1->len, "abcd");

/* now: vstr_num(s1, 1, s1->len) == 4 */

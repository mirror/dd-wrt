 /* add some data... */
 vstr_add_cstr_buf(s1, s1->len, "abcd");

 /* now this will just call memcpy() over the original */
 vstr_sub_cstr_buf(s1, s1->len - 3, 4, "abcd");

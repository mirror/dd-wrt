
 /* add some data, in a new _BUF node... */
 vstr_add_cstr_ptr(s1, s1->len, "abcd");
 vstr_add_cstr_buf(s1, s1->len, "abcd");

/* this makes sure the iovec cache is valid */
 vstr_export_iovec_ptr_all(s1, NULL, NULL);

 /* now this will just swap the pointers for the original _BUF node with the new
  * _PTR node -- and the iovec cache will still be valid */
 vstr_sub_cstr_ptr(s1, s1->len - 3, 4, "abcd");

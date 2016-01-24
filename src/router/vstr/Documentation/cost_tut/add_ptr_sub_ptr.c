
 /* add some data, in a new _PTR node... */
 vstr_add_cstr_buf(s1, s1->len, "WXYZ");

 /* now this will just swap the pointer in the original _PTR node with the new
  * pointer ... so only the original _PTR node needs to be allocated */
 vstr_sub_cstr_ptr(s1, s1->len - 3, 4, "abcd");

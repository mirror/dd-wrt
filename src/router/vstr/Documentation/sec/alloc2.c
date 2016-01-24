/* broken example */
char  *s1;
size_t l1;
...
API_add(s1, l1, "abcd ");       l1 -= strlen("abcd ");
API_add(s1, l1, external_data); l1 -= strlen(external_data);
API_add(s1, l1, " xyz");        l1 -= strlen(" xyz");

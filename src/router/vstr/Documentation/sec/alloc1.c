/* broken example when using a limiting string type API */
API_TYPE s1;
...
API_add(s1, "abcd ");
API_add(s1, external_data);
API_add(s1, " xyz");

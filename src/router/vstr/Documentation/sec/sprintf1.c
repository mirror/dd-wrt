APT_string *s1;
APT_string *s2;
...
API_add(s1, data);

API_fmt(s2, "%s %d %{API_string}", "abcd", 4, s1);

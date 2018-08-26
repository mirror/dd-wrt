import("socket");
$1 = path_concat (path_dirname (__FILE__), "help/sockfuns.hlp");
if (NULL != stat_file ($1))
  add_doc_file ($1);
provide("socket");

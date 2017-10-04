diff --git a/report.c b/report.c
index badb765..d6752eb 100644
--- a/report.c
+++ b/report.c
@@ -294,7 +294,7 @@ void xml_close(void)
     printf("    <HUB COUNT=\"%d\" HOST=\"%s\">\n", at+1, name);
     for( i=0; i<MAXFLD; i++ ) {
       j = fld_index[fld_active[i]];
-      if (j < 0) continue;
+      if (j <= 0) continue; // Field nr 0, " " shouldn't be printed in this method. 
 
       strcpy(name, "        <%s>");
       strcat(name, data_fields[j].format);

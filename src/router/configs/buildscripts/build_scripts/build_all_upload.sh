#!/bin/sh
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
chmod -R 777 /GruppenLW/releases/$DATE/
scp -r /GruppenLW/releases/$DATE ftp.dd-wrt.com:/downloads

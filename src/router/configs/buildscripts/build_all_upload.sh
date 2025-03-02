#!/bin/sh
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86/src/router/httpd)
chmod -R 777 /GruppenLW/releases/$DATE/
scp -r /GruppenLW/releases/$DATE ftp.dd-wrt.com:/downloads
#scp -r /GruppenLW/releases/$DATE 10.88.193.134:/GruppenLW/releases
ssh ftp.dd-wrt.com -t "cd /downloads/www/dd-wrtv2/downloads/betas && ln -sf 2025/$DATE latest"
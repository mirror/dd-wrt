DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
scp -r /GruppenLW/releases/$DATE ftp.dd-wrt.com:/downloads


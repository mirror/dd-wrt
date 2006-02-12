# Example watch control file for uscan
# Rename this file to "watch" and then you can run the "uscan" command
# to check for upstream updates and more.
# See uscan(1) for format

# Compulsory line, this is a version 2 file
version=2

# Uncomment to examine a Webpage 
# <Webpage URL> <string match>
#http://www.example.com/downloads.php rtpproxy-(.*)\.tar\.gz

# Uncomment to examine a Webserver directory
#http://www.example.com/pub/rtpproxy-(.*)\.tar\.gz

# Uncommment to examine a FTP server
#ftp://ftp.example.com/pub/rtpproxy-(.*)\.tar\.gz debian uupdate

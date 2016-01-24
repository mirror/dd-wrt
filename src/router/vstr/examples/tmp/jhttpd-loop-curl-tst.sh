#! /bin/sh

unset http_proxy
while :; do
for i in code.and.org dict qa-quth qr-auth www-cache \
         illiter.at www.illiter.at localhost 127.0.0.1 \
         127.0.0.2 127.0.1.2 127.2.2.2 127.4.3.2; do
curl http://$i:8008/ > /dev/null &
curl http://$i:8008/foo > /dev/null &
curl http://$i:8008/foo/ > /dev/null &
curl http://$i:8008/foo2 > /dev/null &
curl http://$i:8008/foo3 > /dev/null &
curl http://$i:8008/foo4 > /dev/null &
curl http://$i:8008/foo5 > /dev/null &
curl http://$i:8008/foo6 > /dev/null &
curl http://$i:8008/foo-index > /dev/null &
curl http://$i:8008/favicon.ico > /dev/null &
curl http://$i:8008/bar > /dev/null &
curl http://$i:8008/baz > /dev/null &
curl http://$i:8008/bar/baz > /dev/null &
curl http://$i:8008/bar/baz/ > /dev/null &
curl http://$i:8008/ex_httpd.c > /dev/null &
curl http://$i:8008/ex_httpd.c.txt > /dev/null &
curl http://$i:8008/httpd.c > /dev/null &
curl http://$i:8008/~james/ > /dev/null &
curl http://$i:8008/lang > /dev/null &
sleep 0.4
done
done

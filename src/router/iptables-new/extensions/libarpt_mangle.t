:OUTPUT
-j mangle -s 1.2.3.4 --mangle-ip-s 1.2.3.5;=;OK
-j mangle -d 1.2.3.4 --mangle-ip-d 1.2.3.5;=;OK
-j mangle -d 1.2.3.4 --mangle-mac-d 00:01:02:03:04:05;=;OK
-d 1.2.3.4 --h-length 5 -j mangle --mangle-mac-s 00:01:02:03:04:05;=;FAIL

# Installing on Ubuntu using PPAs
```sh
$ sudo apt-get install dpkg-dev
$ sudo apt-get source nginx
$ cd /usr/src/nginx
$ sudo git clone https://github.com/arut/nginx-rtmp-module.git
$ cd nginx-[version-number]
$ sudo vi debian/rules
```

Edit the rules and at then end of the add-modules configuration string add
```sh
--add-module=/usr/src/nginx/nginx-rtmp-module \
```

If installing for the first time build nginx dependancies.
```sh
$ sudo apt-get build-dep nginx
$ dpkg-buildpackage -b
```

(wait for a while while it builds... a really long while... like you might want to go grab a meal)

```sh
$ cd .. && sudo dpkg --install nginx-common_1.3.13-1chl1~quantal1_all.deb nginx-full_1.3.13-1chl1~quantal1_amd64.deb
$ sudo service nginx status
$ sudo service nginx start (if nginx isn't running)
```

[Source](http://serverfault.com/questions/227480/installing-optional-nginx-modules-with-apt-get)

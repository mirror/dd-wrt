# Installing in Gentoo

## Download module source code
You have many options:
* Get the zip at https://github.com/arut/nginx-rtmp-module/archive/master.zip
* Or much better, do a git clone (see options in top of https://github.com/arut/nginx-rtmp-module)
* Or get an ebuild from [fem-overlay](http://subversion.fem.tu-ilmenau.de/repository/fem-overlay/trunk/www-servers/nginx/nginx-1.2.5-r1.ebuild). And set the USE flag "nginx_modules_rtmp" or "nginx_modules_rtmp_hls".

## Emerge nginx with nginx-rtmp-module
> NGINX_ADD_MODULES="/path/to/nginx-rtmp-module" emerge -va nginx

Replace `/path/to/` with the actual module's source path.
You can add with this method any number of custom modules.

To make this change permanent see:
http://wiki.gentoo.org/wiki/Knowledge_Base:Overriding_environment_variables_per_package

## Configure nginx
Don't forget to include a rtmp section inside your nginx configuration file located at `/etc/nginx/nginx.conf`.

See:
* [Getting started](getting_started.md) We already have done _Download, build and install_ Gentoo style ;-)
* [More Examples](examples.md)
* [Reference of all directives](directives.md)

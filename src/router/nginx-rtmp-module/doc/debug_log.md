# Debug log

In case you need to solve a streaming problem you might need to watch debug log.
For that configure nginx with *--with-debug* flag.
```sh
$ cd nginx-X.Y.Z
$ ./configure --add-module=/path/to/nginx-rtmp-module --with-debug ...
```

After compiling set nginx error.log level to *debug* in nginx.conf
```sh
error_log logs/error.log debug;
```

After that you will have  _a lot_ of debug info in error.log. Please grep
what your problem relates to (exec, notify etc) and post to [nginx-rtmp google group](https://groups.google.com/group/nginx-rtmp) to help with solving it.
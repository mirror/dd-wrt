# Exec wrapper in bash

You can write exec wrapper in any language. However you should pay attention to termination process. When publisher closes the stream all executed processed get terminated. If you specify wrapper in ```exec``` directive instead of real ffmpeg then you might end up with your ffmpeg still alive and orphaned until it times out reading input data.

The solution is using signal traps. Here's an example of such wrapper in bash.

```sh
#!/bin/bash

on_die ()
{
    # kill all children
    pkill -KILL -P $$
}

trap 'on_die' TERM
ffmpeg -i rtmp://localhost/myapp/$1 -c copy -f flv rtmp://localhost/myapp2/$1 &
wait
```

The script registers SIGTERM handler which terminates child ffmpeg. Default signal sent by nginx-rtmp is SIGKILL which cannot be caught. For the above script to behave as expected you need to change exec kill signal with ```exec_kill_signal``` directive. It accept numeric or symbolic signal name (for POSIX.1-1990 signals). Here's example application.
```sh
application myapp {
    live on;

    exec /var/scripts/exec_wrapper.sh $name;
    exec_kill_signal term;
}

application myapp2 {
    live on;
}
```

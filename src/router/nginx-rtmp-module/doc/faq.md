# FAQ

####RTMP stream is not played normally in IE, stream stops after several seconds.

Add this directive to fix the problem
```sh
wait_video on;
```

####I use `pull` directive to get stream from remote location. That works for RTMP clients but does not work for HLS.

Currently HLS clients do not trigger any events. You cannot pull or exec when HLS client connects to server. However you can use static directives `exec_static`, `pull ... static` to pull the stream always. 

####Seek does not work with flv files recorded by the module.

To make the files seekable add flv metadata with external software like yamdi, flvmeta or ffmpeg.
```sh
exec_record_done yamdi -i $path -o /var/videos/$basename;
```

####Published stream is missing from stats page after some time and clients fail to connect

Check if you use multiple workers in nginx (`worker_processes`). In such case you have to enable:
```sh
rtmp_auto_push on;
```

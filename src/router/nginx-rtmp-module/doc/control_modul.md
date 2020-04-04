# Control module

Control module is HTTP module which makes it possible to control rtmp module from outside using HTTP protocol. Here's an example of how to enable control.
```sh
http {
...
    server {
        listen       8080;
        server_name  localhost;
        ....
        location /control {
            rtmp_control all;
        }
     }
}
```

There are several sub-modules within control module each controlling a different feature.

# Record
This sub-module starts and stops recordings created with _manual_ flag.
Syntax:
```sh
http://server.com/control/record/start|stop?srv=SRV&app=APP&name=NAME&rec=REC
```

* srv=SRV - optional server{} block number within rtmp{} block, default to first server{} block
* app=APP - required application name
* name=NAME - required stream name
* rec=REC - optional recorder name, defaults to root (unnamed) recorder

Example
```sh
rtmp {
    server {
        listen 1935;
        application myapp {
            live on;
            recorder rec1 {
               record all manual;
               record_suffix all.flv;
               record_path /tmp/rec;
               record_unique on;
           }
       }
    }
}
```

Publish the stream with the following command
```sh
$ ffmpeg -i http://someserver.com/mychannel.ts -c:v copy -c:a nellymoser -ar 44100 -ac 1 -f flv rtmp://localhost/myapp/mystream
```

Use the following commands to start and stop recording
```sh
$ curl "http://localhost:8080/control/record/start?app=myapp&name=mystream&rec=rec1"
§ curl "http://localhost:8080/control/record/stop?app=myapp&name=mystream&rec=rec1"
```

if the record start/stop request returns nothing sometimes, you should check if you use multi workers. one worker works great. 

# Drop
This sub-module provides a simple way to drop client connection.
Syntax:
```sh
http://server.com/control/drop/publisher|subscriber|client?
srv=SRV&app=APP&name=NAME&addr=ADDR&clientid=CLIENTID
```

* srv, app, name - the same as above
* addr - optional client address (the same as returned by rtmp_stat)
* clientid - optional nginx client id (displayed in log and stat) 

The first method ```drop/publisher``` drops publisher connection. The second ```drop/client``` drops every connection matching ```addr``` argument or all clients (including publisher) if ```addr``` is not specified.

Examples
```sh
$ curl http://localhost:8080/control/drop/publisher?app=myapp&name=mystream
$ curl http://localhost:8080/control/drop/client?app=myapp&name=mystream
$ curl http://localhost:8080/control/drop/client?app=myapp&name=mystream&addr=192.168.0.1
$ curl http://localhost:8080/control/drop/client?app=myapp&name=mystream&clientid=1
```

# Redirect
Redirect play/publish client to a new stream.
Syntax:
```sh
http://server.com/control/redirect/publisher|subscriber|client?
srv=SRV&app=APP&name=NAME&addr=ADDR&clientid=CLIENTID&newname=NEWNAME
```

* srv, app, name, addr, clients - the same as above
* newname - new stream name to redirect to

#!/bin/sh

java -cp ../lib/slf4j-api.jar:../lib/slf4j-log4j12.jar:../lib/log4j.jar:\
../lib/libthrift.jar:../lib/db.jar:../lib/db_thrift_interface.jar:\
../lib/db_thrift_server.jar -Djava.library.path=../lib\
	com.sleepycat.server.BdbServer $@
@echo off

set CPATH=..\jars\db_thrift_interface.jar;..\jars\db_thrift_server.jar
set CPATH=..\jars\log4j.jar;..\jars\libthrift.jar;..\jars\db.jar;%CPATH%
set CPATH=..\jars\slf4j-api.jar;..\jars\slf4j-log4j12.jar;%CPATH%;%CLASSPATH%

set LPATH=..\bin;..\..\..\build_windows\Win32\Debug
set LPATH=..\..\..\build_windows\Win32\Release;%LPATH
set LPATH=..\..\..\build_windows\x64\Debug;%LPATH
set LPATH=..\..\..\build_windows\x64\Release;%LPATH
set LPATH=%LPATH%;%PATH%

java -cp "%CPATH%" -Djava.library.path="%LPATH%" com.sleepycat.server.BdbServer %*
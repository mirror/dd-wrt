#!/bin/sh
cmake -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_TO/usr -DCMAKE_BUILD_TYPE:String="Debug" . && make && make install


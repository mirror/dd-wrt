rem This file is used to run slsh once it has been built
set SLSH_CONF_DIR=./slsh/etc
set SLSH_PATH=./slsh/lib
copy src\gw32objs\libslang.dll slsh\gw32objs
slsh\gw32objs\slsh %1 %2 %3 %4 %5 %6 %7 %8 %9
set SLSH_PATH=
set SLSH_CONF_DIR=
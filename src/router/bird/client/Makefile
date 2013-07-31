source=commands.c util.c client.c
root-rel=../
dir-name=client

clients := $(client) birdcl

source-dep := $(source) $(addsuffix .c,$(clients))

subdir: $(addsuffix .o,$(clients))

include ../Rules

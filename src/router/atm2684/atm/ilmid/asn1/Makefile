CC = cc
CFLAGS = -O
LIBS = 
SRCS = asn_len.c asn_tag.c asn_int.c asn_octs.c asn_bits.c str_stk.c \
       asn_oid.c asn_null.c asn_list.c nibble_alloc.c print.c \

OBJS = $(SRCS:%.c=%.o)

default: all
 
all: libasn1.a
 
.c.o:
	$(CC) $(CFLAGS) -c $<
 
libasn1.a: $(OBJS)
	ar cr libasn1.a $(OBJS)

depend:
#	makedepend $(INCLUDES) $(SRCS)
	$(CPP) -M $(SRCS) >.tmpdepend
	mv .tmpdepend .depend

clean:
	rm -rf libasn1.a $(OBJS)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

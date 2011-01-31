LIBS=-latm
OBJS=get_vars.o io.o k_interf.o main.o p_factory.o  p_recogn.o id_list.o tag_list.o poll2select.o lecs.o
BOOTPGMS=mpcd
MAN8= mpcd.8
# MPOA specific flags
# -DDBROKEN_POLL use select() instead of poll()
# -DMPOA_1_1     use proposed MPOA 1.1 features
CFLAGS_PRIVATE=-DBROKEN_POLL

include ../Rules.make

mpcd:		$(OBJS)
		$(CC) $(LDFLAGS) -o mpcd $(OBJS) $(LIBD) $(LDLIBS)

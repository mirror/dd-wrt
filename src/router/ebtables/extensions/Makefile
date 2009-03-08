#! /usr/bin/make

EXT_FUNC+=802_3 nat arp arpreply ip standard log redirect vlan mark_m mark \
          pkttype stp among limit ulog
EXT_TABLES+=filter nat broute
EXT_OBJS+=$(foreach T,$(EXT_FUNC), extensions/ebt_$(T).o)
EXT_OBJS+=$(foreach T,$(EXT_TABLES), extensions/ebtable_$(T).o)
EXT_LIBS+=$(foreach T,$(EXT_FUNC), extensions/libebt_$(T).so)
EXT_LIBS+=$(foreach T,$(EXT_TABLES), extensions/libebtable_$(T).so)
EXT_LIBSI+=$(foreach T,$(EXT_FUNC), -lebt_$(T))
EXT_LIBSI+=$(foreach T,$(EXT_TABLES), -lebtable_$(T))

extensions/ebt_%.so: extensions/ebt_%.o
	$(LD) -shared -o $@ -lc $<

extensions/libebt_%.so: extensions/ebt_%.so
	mv $< $@

extensions/ebtable_%.so: extensions/ebtable_%.o
	$(LD) -shared -o $@ -lc $<

extensions/libebtable_%.so: extensions/ebtable_%.so
	mv $< $@

extensions/ebt_%.o: extensions/ebt_%.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)

extensions/ebtable_%.o: extensions/ebtable_%.c
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)


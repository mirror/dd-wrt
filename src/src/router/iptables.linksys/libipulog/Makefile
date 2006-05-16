EXTRAS+=libipulog/libipulog.a

DEVEL_LIBS+=libipulog/libipulog.a

ifndef TOPLEVEL_INCLUDED
local:
	cd .. && $(MAKE) $(KERN_TARGETS) $(SHARED_LIBS) $(EXTRAS)

else
EXTRA_DEPENDS+=echo libipulog/libipulog.d

libipulog/libipulog.a: libipulog/libipulog.a(libipulog/libipulog.o)

libipulog/libipulog.d: %.d: %.c
	@-$(CC) -M -MG $(CFLAGS) $< | sed -e 's@^.*\.o:@$*.d $*.a($*.o):@' > $@
endif

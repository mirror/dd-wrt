
# Generic make rules

#
# before including this file, set:
#  BUILD_TYPE = RELEASE or DEBUG
# --> default is DEBUG
#

ifeq ($(BUILD_TYPE),RELEASE)
   CFLAGS += -Os
   CPPFLAGS += -Os
else
   CFLAGS += -g
   CPPFLAGS += -g
endif

clean:
	rm -f $(TARGET) *.o $(LDIRT)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $<

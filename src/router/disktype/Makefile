###
###	Makefile for disktype
###

RM = rm -f
#CC = gcc

OBJS   = main.o lib.o \
         buffer.o file.o cdaccess.o cdimage.o vpc.o compressed.o \
         detect.o apple.o amiga.o atari.o dos.o cdrom.o \
         linux.o unix.o beos.o archives.o \
         udf.o blank.o cloop.o

TARGET = disktype

CPPFLAGS = -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS   = -Wall $(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF
LDFLAGS  =
LIBS     =

ifeq ($(NOSYS),)
  system = $(shell uname)
  ifeq ($(system),Linux)
    CPPFLAGS += -DUSE_IOCTL_LINUX
  endif
  ifeq ($(system),FreeBSD)
    # not entirely tested yet
    #CPPFLAGS += -DUSE_IOCTL_FREEBSD
  endif
  ifeq ($(system),Darwin)
    CPPFLAGS += -DUSE_MACOS_TYPE -DUSE_IOCTL_DARWIN
    LIBS     += -framework CoreServices
    ifeq (/Developer/SDKs/MacOSX10.4u.sdk,$(wildcard /Developer/SDKs/MacOSX10.4u.sdk))
      CPPFLAGS += -isysroot /Developer/SDKs/MacOSX10.4u.sdk
      CFLAGS   += -arch i386 -arch ppc
      LDFLAGS  += -arch i386 -arch ppc -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk
    endif
  endif
  ifeq ($(system),AmigaOS)
    CC       += -noixemul
    CFLAGS   += -m68020-60 -msmall-code
    LDFLAGS  += -m68020-60
  endif
endif

# real making

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

$(OBJS): %.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

# cleanup

clean:
	$(RM) *.o *~ *% $(TARGET)
	
install:
	install -D disktype $(INSTALLDIR)/usr/sbin/disktype

distclean: clean
	$(RM) .depend

# automatic dependencies

depend: dep
dep:
	for i in $(OBJS:.o=.c) ; do $(CC) $(CPPFLAGS) -MM $$i ; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

# eof

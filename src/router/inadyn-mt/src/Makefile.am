bin_PROGRAMS = inadyn-mt


inadyn_mt_SOURCES = base64utils.c dyndns.c errorcode.c get_cmd.c http_client.c \
ip.c main.c os.c os_psos.c os_unix.c tcp.c inadyn_cmd.c unicode_util.c \
numbers.c dblhash.c lang.c path.c safe_mem.c md5.c

dist_inadyn_mt_SOURCES = *.h

if IS_WITH_SOUND

inadyn_mt_SOURCES += wave_util.c waveout.c

endif

if IS_WITH_THREADS

inadyn_mt_SOURCES += threads_wrapper.c

endif

inadyn_mt_LDADD = $(ARCH_SPECIFIC_LIBS) $(SNDFILE_LIB)

inadyn_mt_LDFLAGS = -L$(libdir) $(ARCH_SPECIFIC_LDFLAGS)

inadyn_mt_CFLAGS = -I $(includedir) -I /usr/local/include $(CFLAGS) $(ARCH_SPECIFIC_CFLAGS)

bird-symlinks:
	$(MAKE) -C bird/obj .dir-stamp
	mkdir -p bird/obj/conf bird/obj/lib
	- (cd bird/obj/conf ; \
	ln -s ../../conf/cf-lex.l cf-lex.l ; \
	ln -s ../../conf/conf.c conf.c ; \
	ln -s ../../conf/conf.h conf.h ; \
	ln -s ../../sysdep/unix/config.Y config.Y ; \
	ln -s ../../sysdep/unix/krt.Y krt.Y ; \
	ln -s ../../sysdep/linux/netlink/netlink.Y netlink.Y ; \
	)
	- (cd bird/obj/lib ; \
	ln -s ../../lib/birdlib.h birdlib.h ; \
	ln -s ../../lib/bitops.c bitops.c ; \
	ln -s ../../lib/bitops.h bitops.h ; \
	ln -s ../../lib/checksum.c checksum.c ; \
	ln -s ../../lib/checksum.h checksum.h ; \
	ln -s ../../sysdep/unix/endian.h endian.h ; \
	ln -s ../../lib/event.c event.c ; \
	ln -s ../../lib/event.h event.h ; \
	ln -s ../../sysdep/unix/io.c io.c ; \
	ln -s ../../lib/ip.c ip.c ; \
	ln -s ../../lib/ip.h ip.h ; \
	ln -s ../../lib/ipv4.c ipv4.c ; \
	ln -s ../../lib/ipv4.h ipv4.h ; \
	ln -s ../../sysdep/unix/krt.c krt.c ; \
	ln -s ../../sysdep/unix/krt.h krt.h ; \
	ln -s ../../sysdep/linux/netlink/krt-iface.h krt-iface.h ; \
	ln -s ../../sysdep/linux/netlink/krt-scan.h krt-scan.h ; \
	ln -s ../../sysdep/linux/netlink/krt-set.h krt-set.h ; \
	ln -s ../../lib/lists.c lists.c ; \
	ln -s ../../lib/lists.h lists.h ; \
	ln -s ../../sysdep/unix/log.c log.c ; \
	ln -s ../../sysdep/unix/main.c main.c ; \
	ln -s ../../lib/md5.c md5.c ; \
	ln -s ../../lib/md5.h md5.h ; \
	ln -s ../../lib/mempool.c mempool.c ; \
	ln -s ../../sysdep/linux/netlink/netlink.c netlink.c ; \
	ln -s ../../lib/patmatch.c patmatch.c ; \
	ln -s ../../lib/printf.c printf.c ; \
	ln -s ../../sysdep/unix/random.c random.c ; \
	ln -s ../../lib/resource.c resource.c ; \
	ln -s ../../lib/resource.h resource.h ; \
	ln -s ../../lib/slab.c slab.c ; \
	ln -s ../../lib/slists.c slists.c ; \
	ln -s ../../lib/slists.h slists.h ; \
	ln -s ../../lib/socket.h socket.h ; \
	ln -s ../../lib/string.h string.h ; \
	ln -s ../../sysdep/linux/sysio.h sysio.h ; \
	ln -s ../../sysdep/unix/timer.h timer.h ; \
	ln -s ../../lib/unaligned.h unaligned.h ; \
	ln -s ../../sysdep/unix/unix.h unix.h ; \
	ln -s ../../lib/xmalloc.c xmalloc.c ; \
	)

bird: bird-symlinks
	$(MAKE) -C bird

bird-install:
	@true
	install -D bird/bird $(INSTALLDIR)/bird/usr/sbin/bird
	$(STRIP) $(INSTALLDIR)/bird/usr/sbin/bird

#bird-configure:
#	cd bird && ./configure --build=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr --disable-client --with-sysconfig=sysdep/cf/linux-22.h --localstatedir=/tmp/bird --sysconfdir=/tmp/bird --no-create --no-recursion CC=$(ARCH)-linux-uclibc-gcc bird_cv_c_endian=big-endian

bird-clean:
	$(MAKE) -C bird clean

bird-distclean:
	$(MAKE) -C bird clean
	rm -rf bird/obj/nest bird/obj/filter bird/obj/proto
	rm -f bird/obj/.dir-stamp


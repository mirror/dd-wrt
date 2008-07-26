
export ARCH=$1
# strip executables
./sstrip/sstrip $ARCH-uclibc/target/bin/*
./sstrip/sstrip $ARCH-uclibc/target/sbin/rc
./sstrip/sstrip $ARCH-uclibc/target/usr/sbin/*
./sstrip/sstrip $ARCH-uclibc/target/usr/bin/*
./sstrip/sstrip $ARCH-uclibc/target/sbin/*
# strip libraries
./sstrip/sstrip $ARCH-uclibc/target/usr/lib/*
./sstrip/sstrip $ARCH-uclibc/target/usr/lib/iptables/*
./sstrip/sstrip $ARCH-uclibc/target/usr/lib/l2tp/*
./sstrip/sstrip $ARCH-uclibc/target/usr/lib/ser/modules/*
./sstrip/sstrip $ARCH-uclibc/target/usr/lib/openser/modules/*
./sstrip/sstrip $ARCH-uclibc/target/usr/lib/asterisk/modules/*
./sstrip/sstrip $ARCH-uclibc/target/usr/lib/l2tp/*
./sstrip/sstrip $ARCH-uclibc/target/lib/*.so
./sstrip/sstrip $ARCH-uclibc/target/lib/*.so.1
./sstrip/sstrip $ARCH-uclibc/target/lib/*.so.0
./sstrip/sstrip $ARCH-uclibc/target/lib/*.so.6
./sstrip/sstrip $ARCH-uclibc/target/lib/*.so.29

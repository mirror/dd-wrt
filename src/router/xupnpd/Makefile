LUA     = lua-5.1.4
#LUA     = lua-5.2.0
STATIC  = false

CFLAGS  = -fno-exceptions -fno-rtti -O2 -I$(LUA) -L$(LUA)
SRC     = main.cpp soap.cpp mem.cpp mcast.cpp luaxlib.cpp luaxcore.cpp luajson.cpp luajson_parser.cpp
LUAMYCFLAGS = -DLUA_USE_LINUX

SDK_8_09 = /u01/home/shocker_old/staff/openwrt_8_09/staging_dir
SDK_BACKFIRE = /u01/home/shocker_old/staff/openwrt/staging_dir
SDK_BBREAKER = /home/shocker/staff/openwrt/staging_dir

ifeq ($(STATIC),true)
CFLAGS+=-static
LUAMYCFLAGS+=-static
endif

x86:
	$(MAKE) -C $(LUA) CC=$(CC) a
	$(CC) -O2 -c -o md5.o md5c.c
	$(CC) $(CFLAGS) -DWITH_LIBUUID -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -o xupnpd $(SRC) md5.o -llua -ldl -lm -luuid
	strip xupnpd

# Cygwin
x86win:
	$(MAKE) -C $(LUA) CC=$(CC) a
	$(CC) -O2 -c -o md5.o md5c.c
	$(CC) $(CFLAGS) -DWITH_LIBUUID -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_UNDER_WIN -o xupnpd $(SRC) md5.o -llua -ldl -lm -luuid
	strip xupnpd

x86_bsd:
	$(MAKE) -C $(LUA) CC=$(CC) a
	$(CC) -O2 -c -o md5.o md5c.c
	$(CC) $(CFLAGS) -DINET -DWITH_LIBUUID -o xupnpd $(SRC) md5.o -llua -lm
	strip xupnpd


# D-Link DIR-320/DD-WRT (mipsel)
bcm947x_ddwrt:
	$(MAKE) embedded TARGET=bcm947x-ddwrt SDK=$(SDK_8_09)/toolchain-mipsel_gcc3.4.6/mipsel-linux/bin UTILS=$(SDK_8_09)/toolchain-mipsel_gcc3.4.6/libexec/gcc/mipsel-linux-uclibc/3.4.6 STAGING_DIR=$(SDK_8_09)

# OpenWrt Backfire
bcm947x_backfire:
	$(MAKE) embedded TARGET=bcm947x-bf SDK=$(SDK_BACKFIRE)/toolchain-mipsel_gcc-3.4.6-bcm947xx/mipsel-openwrt-linux/bin UTILS=$(SDK_BACKFIRE)/toolchain-mipsel_gcc-3.4.6-bcm947xx/libexec/gcc/mipsel-openwrt-linux-uclibc/3.4.6 STAGING_DIR=$(SDK_BACKFIRE)

ar71xx_backfire:
	$(MAKE) embedded TARGET=ar71xx-bf SDK=$(SDK_BACKFIRE)/toolchain-mips_r2_gcc-4.3.3+cs-ar71xx/mips-openwrt-linux/bin UTILS=$(SDK_BACKFIRE)/toolchain-mips_r2_gcc-4.3.3+cs-ar71xx/libexec/gcc/mips-openwrt-linux-uclibc/4.3.3 STAGING_DIR=$(SDK_BACKFIRE)

ar231x_backfire:
	$(MAKE) embedded TARGET=ar231x-bf SDK=$(SDK_BACKFIRE)/toolchain-mips_gcc-4.3.3+cs-ar231x/mips-openwrt-linux/bin UTILS=$(SDK_BACKFIRE)/toolchain-mips_gcc-4.3.3+cs-ar231x/libexec/gcc/mips-openwrt-linux-uclibc/4.3.3 STAGING_DIR=$(SDK_BACKFIRE)


# OpenWrt Barrier Breaker

# Broadcom BCM2708/BCM2835
bcm2708:
	$(MAKE) embedded TARGET=bcm2708 SDK=$(SDK_BBREAKER)/toolchain-arm_arm1176jzf-s+vfp_gcc-4.8-linaro-bcm2708/arm-openwrt-linux/bin UTILS=$(SDK_BBREAKER)/toolchain-arm_arm1176jzf-s+vfp_gcc-4.8-linaro-bcm2708/libexec/gcc/arm-openwrt-linux-uclibcgnueabi/4.8.3 STAGING_DIR=$(SDK_BBREAKER)

# Broadcom BCM47xx/BCM53xx (MIPS)
bcm47xx:
	$(MAKE) embedded TARGET=bcm47xx SDK=$(SDK_BBREAKER)/toolchain-mipsel_mips32_gcc-4.8-linaro-bcm47xx/mipsel-openwrt-linux/bin UTILS=$(SDK_BBREAKER)/toolchain-mipsel_mips32_gcc-4.8-linaro-bcm47xx/libexec/gcc/mipsel-openwrt-linux-uclibc/4.8.3 STAGING_DIR=$(SDK_BBREAKER)

# Broadcom BCM63xx
bcm63xx:
	$(MAKE) embedded TARGET=bcm63xx SDK=$(SDK_BBREAKER)/toolchain-mips_mips32_gcc-4.8-linaro-bcm63xx/mips-openwrt-linux/bin UTILS=$(SDK_BBREAKER)/toolchain-mips_mips32_gcc-4.8-linaro-bcm63xx/libexec/gcc/mips-openwrt-linux-uclibc/4.8.3 STAGING_DIR=$(SDK_BBREAKER)

# Atheros AR231x/AR5312
ar231x:
	$(MAKE) embedded TARGET=ar231x SDK=$(SDK_BBREAKER)/toolchain-mips_mips32_gcc-4.8-linaro-ar231x/mips-openwrt-linux/bin UTILS=$(SDK_BBREAKER)/toolchain-mips_mips32_gcc-4.8-linaro-ar231x/libexec/gcc/mips-openwrt-linux-uclibc/4.8.3 STAGING_DIR=$(SDK_BBREAKER)

# Atheros AR7xxx/AR9xxx
ar7xxx:
	$(MAKE) embedded TARGET=ar7xxx SDK=$(SDK_BBREAKER)/toolchain-mips_34kc_gcc-4.8-linaro-ar7xxx/mips-openwrt-linux/bin UTILS=$(SDK_BBREAKER)/toolchain-mips_34kc_gcc-4.8-linaro-ar7xxx/libexec/gcc/mips-openwrt-linux-uclibc/4.8.3 STAGING_DIR=$(SDK_BBREAKER)


# Other
samsung_d:
	$(MAKE) embedded TARGET=arm-v7a8 SDK=/opt/VDLinux-armv7a8/bin

samsung_c:
	$(MAKE) embedded TARGET=arm-v7_vfp_le SDK=/opt/arm_v7_vfp_le/bin

dreambox:
	$(MAKE) embedded TARGET=mipsel_dreambox SDK=/opt/mipsel-unknown-linux-gnu/bin


embedded:
	PATH=$(PATH):$(UTILS) $(MAKE) -C $(LUA) CC=$(SDK)/$(CC) a MYCFLAGS='$(LUAMYCFLAGS)'
	$(SDK)/$(CC) -O2 -c -B$(UTILS) -o md5.o md5c.c
	$(SDK)/$(CC) $(CFLAGS) -B$(UTILS) -DWITH_URANDOM -o xupnpd-$(TARGET) $(SRC) md5.o -llua -lm -ldl
	$(SDK)/strip xupnpd-$(TARGET)

clean:
	$(MAKE) -C $(LUA) clean
	rm -f $(LUA)/liblua.a
	rm -f md5.o

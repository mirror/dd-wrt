include $(TOP)/.config


ifeq ($(ARCH),mips64)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=mips64-boese-be-elf
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=mips64-be-elf
endif
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=mips64-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=mips64-boese-be-elf
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=mips64-be-elf
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifneq ($(CONFIG_NOWIFI),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install TARGET=mips64-be-elf install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=mips64-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=mips64-be-elf install
endif
endif

endif

ifeq ($(ARCH),aarch64)
ifeq ($(CONFIG_RAIEXTRA),y)
	HAL_TARGET=arm64-raiextra-le-elf
else
	HAL_TARGET=arm64-le-elf
endif
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=arm64-le-elf ARCH=arm64 CROSS_COMPILE=aarch64-linux- 
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=$(HAL_TARGET) BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  ARCH=arm64 CROSS_COMPILE=aarch64-linux-

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=arm64-le-elf  ARCH=arm64 CROSS_COMPILE=aarch64-linux-
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean ARCH=arm64 CROSS_COMPILE=aarch64-linux-
madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install  ARCH=arm64 CROSS_COMPILE=aarch64-linux-
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=$(HAL_TARGET) install  ARCH=arm64  CROSS_COMPILE=aarch64-linux-
endif

ifeq ($(ARCH),mipsel)


ifeq ($(ARCHITECTURE),adm5120)
madwifi:
ifeq ($(CONFIG_DIST),"micro")
	make -j 4 -C madwifi.dev/madwifi.dev/tools2 TARGET=adm5120-le-elf-micro BINDIR=$(INSTALLDIR)/madwifi/usr/sbin
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=adm5120-le-elf-micro
else
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=adm5120-le-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=adm5120-le-elf
endif

madwifi-clean:
ifeq ($(CONFIG_DIST),"micro")
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=adm5120-le-elf-micro
	make -C madwifi.dev/madwifi.dev/tools2 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=adm5120-le-elf
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
endif

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_DIST),"micro")
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=adm5120-le-elf-micro install
	make -C madwifi.dev/madwifi.dev/tools2 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=adm5120-le-elf install
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
endif


else

ifeq ($(ARCHITECTURE),rt2880)
madwifi:
	@true

madwifi-clean:
	@true
	
madwifi-install:
	@true

else
madwifi:
ifeq ($(CONFIG_DIST),"micro")
	make -j 4 -C madwifi.dev/madwifi.dev/tools2 TARGET=mipsisa32-le-elf-micro BINDIR=$(INSTALLDIR)/madwifi/usr/sbin
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-le-elf-micro
else
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=mipsisa32-le-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-le-elf
endif

madwifi-clean:
ifeq ($(CONFIG_MADWIFI),y)
ifeq ($(CONFIG_DIST),"micro")
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-le-elf-micro
	make -C madwifi.dev/madwifi.dev/tools2 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-le-elf-micro
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
endif
else
	@true
endif

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_DIST),"micro")
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=mipsisa32-le-elf-micro install
	make -C madwifi.dev/madwifi.dev/tools2 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=mipsisa32-le-elf install
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
endif
endif
endif
endif
ifeq ($(ARCH),arm)
ifeq ($(ARCHITECTURE),storm)
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=armv9tdmi-boese-le-elf  
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=armv9tdmi-boese-le-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=armv9tdmi-boese-le-elf
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=armv9tdmi-boese-le-elf install
endif
ifeq ($(ARCHITECTURE),openrisc)
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=armv9tdmi-boese-le-elf  
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=armv9tdmi-boese-le-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=armv9tdmi-boese-le-elf
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=armv9tdmi-boese-le-elf install
endif
ifeq ($(ARCHITECTURE),laguna)
#ifeq ($(CONFIG_RAIEXTRA),y)
#	HAL_TARGET=laguna-raiextra-le-elf
#else
	HAL_TARGET=laguna-le-elf
#endif
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=$(HAL_TARGET)  
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=$(HAL_TARGET) BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=$(HAL_TARGET)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=$(HAL_TARGET) install
endif

ifeq ($(ARCHITECTURE),ventana)
ifeq ($(CONFIG_RAIEXTRA),y)
	HAL_TARGET=ventana-raiextra-le-elf
else
	HAL_TARGET=ventana-le-elf
endif
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=$(HAL_TARGET)  
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=$(HAL_TARGET) BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=$(HAL_TARGET)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=$(HAL_TARGET) install
endif
ifeq ($(ARCHITECTURE),northstar)
madwifi:
	@true


madwifi-clean:
	@true

madwifi-install:
	@true

endif
ifeq ($(ARCHITECTURE),mvebu)
madwifi:
	@true


madwifi-clean:
	@true

madwifi-install:
	@true

endif

ifeq ($(ARCHITECTURE),ipq806x)
madwifi:
	@true


madwifi-clean:
	@true

madwifi-install:
	@true

endif
ifeq ($(ARCHITECTURE),alpine)
madwifi:
	@true


madwifi-clean:
	@true

madwifi-install:
	@true

endif
endif

ifeq ($(ARCH),armeb)
ifeq ($(ARCHITECTURE),wrt300nv2)
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=xscale-be-elf-wrt300n  
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=xscale-be-elf-wrt300n BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=xscale-be-elf-wrt300n
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=xscale-be-elf-wrt300n install
else

ifeq ($(CONFIG_RAIEXTRA),y)

madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=xscale-raiextra-be-elf  
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=xscale-raiextra-be-elf INDIR=$(INSTALLDIR)/madwifi/usr/sbin
	#make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=xscale-raiextra-elf ARCH=arm 
	#make -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=xscale-raiextra-elf ARCH=armeb ARCH=arm BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=xscale-raiextra-be-elf
	#make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=xscale-raiextra-elf ARCH=arm
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifneq ($(CONFIG_NOWIFI),y)
	make -j 4 -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=xscale-raiextra-be-elf install
endif

else

madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=xscale-boese-be-elf  
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=xscale-boese-be-elf INDIR=$(INSTALLDIR)/madwifi/usr/sbin
	#make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=xscale-be-elf ARCH=arm 
	#make -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=xscale-be-elf ARCH=armeb ARCH=arm BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=xscale-boese-be-elf
	#make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=xscale-be-elf ARCH=arm
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifneq ($(CONFIG_NOWIFI),y)
	make -j 4 -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=xscale-boese-be-elf install
endif


endif


endif

endif


ifeq ($(ARCH),mips)

ifeq ($(ARCHITECTURE),fonera)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap51-boese BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ap51-boese BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ap51 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap51-boese BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap51-boese install
else
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap51 install
endif


else
ifeq ($(ARCHITECTURE),eoc2610)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=eoc2610 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=eoc2610 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ap51 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=eoc2610 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-install:
ifneq ($(CONFIG_DEMO),y)
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=eoc2610 install
else
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap51 install
endif
else
madwifi-install:
	@true
endif


else 
ifeq ($(ARCHITECTURE),bwrg1000)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=bwrg1000 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=bwrg1000 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ap51 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=bwrg1000 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-install:
ifneq ($(CONFIG_DEMO),y)
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=bwrg1000 install
else
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap51 install
endif
else
madwifi-install:
	@true
endif


else
ifeq ($(ARCHITECTURE),ls2)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ls2 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ls2 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ap51 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ls2 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-install:
ifneq ($(CONFIG_DEMO),y)
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ls2 install
else
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap51 install
endif
else
madwifi-install:
	@true
endif


else
ifeq ($(ARCHITECTURE),bs2)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=bs2 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=bs2 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ap51 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=bs2 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=AHB TOOLPATH=$(LINUXDIR)
endif

madwifi-install:
ifneq ($(CONFIG_DEMO),y)
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=bs2 install
else
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap51 install
endif
else
madwifi-install:
	@true
endif



else
ifeq ($(ARCHITECTURE),lsx)
madwifi:
ifeq ($(CONFIG_RS),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ar7100-rs-be-elf BUS=PCI TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ar7100-rs-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR)
else
ifeq ($(CONFIG_WP543),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ar7100-be-elf BUS=PCI TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ar7100-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=lsx-be-elf BUS=PCI TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=lsx-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR)
endif
endif

madwifi-clean:
ifeq ($(CONFIG_RS),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ar7100-rs-be-elf BUS=PCI TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=PCI TOOLPATH=$(LINUXDIR)
else
ifeq ($(CONFIG_WP543),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ar7100-be-elf BUS=PCI TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=PCI TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=lsx-be-elf BUS=PCI TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=PCI TOOLPATH=$(LINUXDIR)
endif
endif

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_RS),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ar7100-rs-be-elf install
else
ifeq ($(CONFIG_WP543),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ar7100-be-elf install
else
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=lsx-be-elf install
endif
endif
else
ifeq ($(ARCHITECTURE),ja76pf)
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ar7100-be-elf BUS=PCI TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ar7100-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR)

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ar7100-be-elf BUS=PCI TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=PCI TOOLPATH=$(LINUXDIR)

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ar7100-be-elf install

else
ifeq ($(ARCHITECTURE),danube)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-be-elf BUS=PCI TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=mipsisa32-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap51 BUS=AHB TOOLPATH=$(LINUXDIR) LDOPTS="--no-warn-mismatch"
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=ap51 BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR)
endif

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-be-elf BUS=PCI TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=PCI TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-be-elf BUS=PCI TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean BUS=PCI TOOLPATH=$(LINUXDIR)
endif

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=PCI TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=mipsisa32-be-elf install
else
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) install
	make -C madwifi.dev/madwifi.dev/ KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap51 install 
endif


else


ifeq ($(ARCHITECTURE),whrag108)
madwifi:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap30-boese-be-elf  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=ap30-boese-be-elf BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap30-$(MADFLAG)  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="$(CONFIG_MADWIFIFLAGS) $(COPTS) -DNEED_PRINTF" TARGET=ap30-$(MADFLAG)elf  TOOLPATH=$(LINUXDIR) BUS=AHB BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
endif

madwifi-clean:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap30-boese-be-elf  BUS=AHB  TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap30-$(MADFLAG) BUS=AHB  TOOLPATH=$(LINUXDIR)
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean 

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap30-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap30-$(MADFLAG) install
endif
else
ifeq ($(ARCHITECTURE),rcaa01)
madwifi:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap30-boese-be-elf  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=ap30-boese-be-elf BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap30-$(MADFLAG)  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="$(CONFIG_MADWIFIFLAGS) $(COPTS) -DNEED_PRINTF" TARGET=ap30-$(MADFLAG)elf  TOOLPATH=$(LINUXDIR) BUS=AHB BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
endif

madwifi-clean:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap30-boese-be-elf  BUS=AHB  TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap43-$(MADFLAG) BUS=AHB  TOOLPATH=$(LINUXDIR)
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean 

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap30-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap43-$(MADFLAG) install
endif
else
ifeq ($(ARCHITECTURE),ca8)
madwifi:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap43-boese-be-elf  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=ap43-boese-be-elf BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap43-$(MADFLAG)  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="$(CONFIG_MADWIFIFLAGS) $(COPTS) -DNEED_PRINTF" TARGET=ap43-$(MADFLAG)elf  TOOLPATH=$(LINUXDIR) BUS=AHB BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
endif

madwifi-clean:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap43-boese-be-elf  BUS=AHB  TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap43-$(MADFLAG) BUS=AHB  TOOLPATH=$(LINUXDIR)
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean 

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap43-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap43-$(MADFLAG) install
endif
else
ifeq ($(ARCHITECTURE),ls5)
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ls5  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=ls5 BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ls5  BUS=AHB  TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean 

madwifi-install:
ifneq ($(CONFIG_DEMO),y)
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ls5 install
else
madwifi-install:
	@true
endif

else
ifeq ($(ARCHITECTURE),eoc5610)
madwifi:
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=eoc5610  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=eoc5610 BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)

madwifi-clean:
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=eoc5610  BUS=AHB  TOOLPATH=$(LINUXDIR)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean 

madwifi-install:
ifneq ($(CONFIG_DEMO),y)
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=eoc5610 install
else
madwifi-install:
	@true
endif

else
ifeq ($(ARCHITECTURE),mr3202a)
madwifi:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
ifeq ($(CONFIG_BOESE),y)
ifeq ($(CONFIG_WRK54G),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap61-lite-boese-be-elf  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=ap61-lite-boese-be-elf BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap61-boese-be-elf  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=ap61-boese-be-elf BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)
endif
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap61  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="$(CONFIG_MADWIFIFLAGS) $(COPTS) -DNEED_PRINTF" TARGET=ap61  TOOLPATH=$(LINUXDIR) BUS=AHB BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
endif

madwifi-clean:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
ifeq ($(CONFIG_BOESE),y)
ifeq ($(CONFIG_WRK54G),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap61-lite-boese-be-elf  BUS=AHB  TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap61-boese-be-elf  BUS=AHB  TOOLPATH=$(LINUXDIR)
endif
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap61 BUS=AHB  TOOLPATH=$(LINUXDIR)
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean 

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
ifeq ($(CONFIG_BOESE),y)
ifeq ($(CONFIG_WRK54G),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap61-lite-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap61-boese-be-elf install
endif
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap61 install
endif

else

ifeq ($(ARCHITECTURE),wrt54g2v11)
madwifi:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap61-boese-be-elf  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=ap61-boese-be-elf BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap61  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="$(CONFIG_MADWIFIFLAGS) $(COPTS) -DNEED_PRINTF" TARGET=ap61  TOOLPATH=$(LINUXDIR) BUS=AHB BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
endif

madwifi-clean:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap61-boese-be-elf  BUS=AHB  TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap61 BUS=AHB  TOOLPATH=$(LINUXDIR)
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean 

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap61-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap61 install
endif

else

ifeq ($(ARCHITECTURE),dir300)
madwifi:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap61-boese-be-elf  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="-DBOESE=1 $(COPTS)" TARGET=ap61-boese-be-elf BUS=AHB  BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  TOOLPATH=$(LINUXDIR)
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=ap61  BUS=AHB   TOOLPATH=$(LINUXDIR)
	make -j 4 -C madwifi.dev/madwifi.dev/tools CFLAGS="$(CONFIG_MADWIFIFLAGS) $(COPTS) -DNEED_PRINTF" TARGET=ap61  TOOLPATH=$(LINUXDIR) BUS=AHB BINDIR=$(INSTALLDIR)/madwifi/usr/sbin 
endif

madwifi-clean:
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap61-boese-be-elf  BUS=AHB  TOOLPATH=$(LINUXDIR)
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=ap61 BUS=AHB  TOOLPATH=$(LINUXDIR)
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean 

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
#	make -C madwifi.dev/madwifi.dev/diag TARGET=xscale-$(MADFLAG)be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap61-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin BUS=AHB TOOLPATH=$(LINUXDIR)  DESTDIR=$(INSTALLDIR)/madwifi TARGET=ap61 install
endif

else

ifeq ($(ARCHITECTURE),ap83)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),dir825)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),wrt400)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),wndr3700)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),wzrag300nh)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),wzrg450)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),ubntm)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),unifi)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),whrhpgn)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),hornet)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),wasp)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),dir615e)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),wr741)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
ifeq ($(ARCHITECTURE),jjap93)
madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
madwifi:
	make -C madwifi/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-be-elf
	make -C madwifi/madwifi.dev/tools TARGET=mipsisa32-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
	make -C madwifi/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=mipsisa32-be-elf
	make -C madwifi/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install
	make -C madwifi/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=mipsisa32-be-elf install
endif

endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif

ifeq ($(ARCH),i386)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=i386-boese-elf
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=i386-boese-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=i386-elf
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=i386-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin
endif

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=i386-boese-elf
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=i386-elf
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
endif

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifneq ($(CONFIG_NOWIFI),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install TARGET=i386-elf install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=i386-boese-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=i386-elf install
endif
endif

endif


ifeq ($(ARCH),x86_64)
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=x86_64-boese-elf
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=x86_64-boese-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=x86_64-elf
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=x86_64-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin
endif

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=x86_64-boese-elf
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=x86_64-elf
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
endif

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifneq ($(CONFIG_NOWIFI),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install TARGET=x86_64-elf install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=x86_64-boese-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=x86_64-elf install
endif
endif

endif



ifeq ($(ARCH),powerpc)
ifeq ($(CONFIG_UNIWIP),y)

madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=powerpc-boese-be-elf
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=powerpc-be-elf
endif
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=powerpc-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=powerpc-boese-be-elf
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=powerpc-be-elf
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifneq ($(CONFIG_NOWIFI),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install TARGET=powerpc-be-elf install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=powerpc-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=powerpc-be-elf install
endif
endif
else


ifeq ($(CONFIG_WDR4900),y)

madwifi:
	@true

madwifi-clean:
	@true

madwifi-install:
	@true

else
madwifi:
ifeq ($(CONFIG_BOESE),y)
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=powerpc-boese-be-elf
else
	make -j 4 -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) TARGET=powerpc-be-elf
endif
	make -j 4 -C madwifi.dev/madwifi.dev/tools TARGET=powerpc-be-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin

madwifi-clean:
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=powerpc-boese-be-elf
else
	make -C madwifi.dev/madwifi.dev clean KERNELPATH=$(LINUXDIR) TARGET=powerpc-be-elf
endif
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
ifneq ($(CONFIG_NOWIFI),y)
	make -C madwifi.dev/madwifi.dev/tools BINDIR=$(INSTALLDIR)/madwifi/usr/sbin install TARGET=powerpc-be-elf install
ifeq ($(CONFIG_BOESE),y)
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=powerpc-boese-be-elf install
else
	make -C madwifi.dev/madwifi.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi TARGET=powerpc-be-elf install
endif
endif
endif
endif
endif


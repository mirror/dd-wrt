#############################################################
#
# elf2flt -- only compiled for mmu-less systems
#
#############################################################
ELF2FLT_SITE:=http://www.uclibc.org/downloads/toolchain
ELF2FLT_SOURCE:=elf2flt-20030620.tar.bz2
ELF2FLT_DIR:=$(BUILD_DIR)/elf2flt

#$(DL_DIR)/$(ELF2FLT_SOURCE):
# 	$(WGET) -P $(DL_DIR) $(ELF2FLT_SITE)/$(ELF2FLT_SOURCE)

$(ELF2FLT_DIR)/.unpacked: $(BUILD_DIR)/.setup $(DL_DIR)/$(ELF2FLT_SOURCE)
	bzcat $(DL_DIR)/$(ELF2FLT_SOURCE) | tar -C $(BUILD_DIR) -xvf -
	touch $(ELF2FLT_DIR)/.unpacked

$(ELF2FLT_DIR)/.configured: $(ELF2FLT_DIR)/.unpacked
	# We need to fixup the binutils include files a bit
	(cd $(BINUTILS_DIR1); rm -f include; cp -a $(BINUTILS_DIR)/include .)
	cp -a $(BINUTILS_DIR1)/bfd/bfd.h $(BINUTILS_DIR1)/include/
	(cd $(ELF2FLT_DIR); ./configure \
		--target=$(GNU_TARGET_NAME) --prefix=$(STAGING_DIR) \
		--with-libbfd=$(BINUTILS_DIR1)/bfd/libbfd.a \
		--with-libiberty=$(BINUTILS_DIR1)/libiberty/libiberty.a \
		--with-binutils-include-dir=$(BINUTILS_DIR1)/include \
		--with-bfd-include-dir=$(BINUTILS_DIR1)/bfd)
	touch $(ELF2FLT_DIR)/.configured

$(ELF2FLT_DIR)/elf2flt: $(ELF2FLT_DIR)/.configured
	$(MAKE) -C $(ELF2FLT_DIR)
	chmod a+x $(ELF2FLT_DIR)/ld-elf2flt

$(STAGING_DIR)/$(GNU_TARGET_NAME)/bin/elf2flt: $(ELF2FLT_DIR)/elf2flt
	cp $(ELF2FLT_DIR)/elf2flt $(STAGING_DIR)/bin/$(ARCH)-uclibc-elf2flt
	(cd $(STAGING_DIR)/$(GNU_TARGET_NAME)/bin; ln -f ../../bin/$(ARCH)-uclibc-elf2flt elf2flt)
	cp $(ELF2FLT_DIR)/flthdr $(STAGING_DIR)/bin/$(ARCH)-uclibc-flthdr
	(cd $(STAGING_DIR)/$(GNU_TARGET_NAME)/bin; ln -f ../../bin/$(ARCH)-uclibc-flthdr flthdr)
	if [ ! -f $(STAGING_DIR)/bin/$(ARCH)-uclibc-ld.real ] ; then \
		mv $(STAGING_DIR)/bin/$(ARCH)-uclibc-ld $(STAGING_DIR)/bin/$(ARCH)-uclibc-ld.real;\
	fi;
	cp $(ELF2FLT_DIR)/ld-elf2flt $(STAGING_DIR)/bin/$(ARCH)-uclibc-ld
	cp $(ELF2FLT_DIR)/elf2flt.ld $(STAGING_DIR)/lib

elf2flt: gcc_final $(STAGING_DIR)/$(GNU_TARGET_NAME)/bin/elf2flt

elf2flt-clean:
	rm -f $(STAGING_DIR)/sbin/elf2flt
	-$(MAKE) -C $(ELF2FLT_DIR) clean

elf2flt-dirclean:
	rm -rf $(ELF2FLT_DIR)

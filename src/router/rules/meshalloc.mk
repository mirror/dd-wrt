MESHALLOC_PKG_BUILD_DIR=$(TOP)/meshalloc
MESHALLOC_STAGING_DIR=$(TOP)/_staging
MESHALLOC_PKG_INSTALL:=1
MESHALLOC_CMAKE_OPTIONS+=VERBOSE=0 CFLAGS="$(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		    -DCMAKE_BUILD_TYPE=release \
		    -DCMAKE_AR=${GCCAR} \
		    -DCMAKE_C_FLAGS="$(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		    -DCMAKE_RANLIB=${GCCRANLIB}


MESHALLOC_EXTRA_CFLAGS=$(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections
MESHALLOC_EXTRA_CXXFLAGS=$(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections
MESHALLOC_EXTRA_LDFLAGS=-ffunction-sections -fdata-sections -Wl,--gc-sections

meshalloc-configure:
	$(call CMakeClean,$(MESHALLOC_PKG_BUILD_DIR))
	$(call CMakeConfigure,$(MESHALLOC_PKG_BUILD_DIR),$(MESHALLOC_STAGING_DIR),$(MESHALLOC_CMAKE_OPTIONS),$(MESHALLOC_EXTRA_CFLAGS),$(MESHALLOC_EXTRA_LDFLAGS),.) 

meshalloc:
	$(MAKE) -C meshalloc

meshalloc-install:

meshalloc-clean:
	if [ -e "$(MESHALLOC_PKG_BUILD_DIR)/Makefile" ]; then $(MAKE) -C meshalloc clean ; fi

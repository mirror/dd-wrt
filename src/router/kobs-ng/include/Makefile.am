noinst_HEADERS=version.h

version.h: stamp-vh
	@:

stamp-vh: $(top_builddir)/configure.in
	@echo "/*" > ver.tmp
	@echo " *  version.h" >> ver.tmp
	@echo " */" >> ver.tmp
	@echo "" >> ver.tmp
	@echo "#define KOBS_NG_MAJOR		$(KOBS_NG_MAJOR)" >> ver.tmp
	@echo "#define KOBS_NG_MINOR		$(KOBS_NG_MINOR)" >> ver.tmp
	@echo "#define KOBS_NG_SUBMINOR	$(KOBS_NG_SUBMINOR)" >> ver.tmp
	@echo "#define KOBS_NG_VERSION		((KOBS_NG_MAJOR<<16)|\\" >> ver.tmp
	@echo "				 (KOBS_NG_MINOR<<8)|\\" >> ver.tmp
	@echo "				  KOBS_NG_SUBMINOR)" >> ver.tmp
	@echo "#define KOBS_NG_VERSION_STR	\"$(KOBS_NG_VERSION)\"" >> ver.tmp
	@echo >> ver.tmp
	@cmp -s version.h ver.tmp \
          || (echo "Updating version.h"; \
              cp ver.tmp version.h; \
              echo timestamp > stamp-vh)
	-@rm -f ver.tmp

INCLUDES=-I$(top_srcdir)/include

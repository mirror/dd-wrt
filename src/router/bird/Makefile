# Generated automatically from Makefile-top.in by configure.
# Makefile for in place build of BIRD
# (c) 1999--2000 Martin Mares <mj@ucw.cz>

objdir=obj

all depend tags install install-docs:
	$(MAKE) -C $(objdir) $@

docs userdocs progdocs:
	$(MAKE) -C doc $@

clean:
	$(MAKE) -C $(objdir) clean
	find . -name "*~" -or -name "*.[oa]" -or -name "\#*\#" -or -name TAGS -or -name core -or -name depend -or -name ".#*" | xargs rm -f

distclean: clean
	$(MAKE) -C doc distclean
	rm -rf $(objdir)
	rm -f config.* configure sysdep/autoconf.h sysdep/paths.h Makefile

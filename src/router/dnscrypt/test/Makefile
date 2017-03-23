
all:

test:
	rake cucumber

clean:
distclean:
install:
install-data:
install-exec:
uninstall:
install-dvi:
install-html:
install-info:
install-ps:
install-pdf:
installdirs:
check:
installcheck:
mostlyclean: clean
maintainer-clean: distclean

distdir:
	while read dir  ; do /usr/bin/mkdir -p "$(distdir)/$$dir" ; done < dist-dirs
	while read file ; do /usr/bin/install -c "$$file" "$(distdir)/$$file" ; done < dist-files

EMPTY_AUTOMAKE_TARGETS = dvi pdf ps info html tags ctags
.PHONY: $(EMPTY_AUTOMAKE_TARGETS)
$(EMPTY_AUTOMAKE_TARGETS):

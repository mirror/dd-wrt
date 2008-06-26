

top_builddir=../
top_srcdir=..
srcdir=.

LS=/bin/ls


include ../Make.rules

CONTACT_ADDRESS=core@proftpd.org
COPYRIGHT_HOLDER="The ProFTPD Project"
PACKAGE=proftpd

# This file is generated using:
#
#   `find ../ -name "*.[ch]" -print | egrep "include|modules|src" > files.txt'
#
# and is then checked into CVS.
FILES=files.txt

LANGS=en_US

# gettext utilities.  Note that this assumes gettext is installed and
# available on the system.
MSGFMT=msgfmt
MSGINIT=msginit
XGETTEXT=xgettext

.SUFFIXES: .h

.PHONY: ""

all:
	@echo "Running make from top level directory."
	cd .. && $(MAKE) all

Makefile: Makefile.in ../config.status
	cd ../ && ./config.status

template:
	-$(XGETTEXT) -L C --keyword=_ --flag=_:1:pass-c-format --keyword=N_ --flag=N_:1:pass-c-format --no-wrap --copyright-holder $(COPYRIGHT_HOLDER) --msgid-bugs-address $(CONTACT_ADDRESS) --sort-by-file -f $(FILES) -o $(PACKAGE).pot

locale: template $(LANGS)

install:

clean:
	$(RM) $(PACKAGE).pot *.po

# Specific languages
en_US:
	-$(MSGINIT) --no-translator --sort-by-file -i $(PACKAGE).pot -o $@.po -l $@
	-$(MSGFMT) $@.po -o $@.mo

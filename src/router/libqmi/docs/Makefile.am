
SUBDIRS = reference

# require gtk-doc when making dist
#
if ENABLE_GTK_DOC
dist-check-gtk-doc:
else
dist-check-gtk-doc:
	@echo "*** gtk-doc must be enabled in order to make dist"
	@false
endif

dist-hook: dist-check-gtk-doc

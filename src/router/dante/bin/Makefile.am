bin_SCRIPTS = socksify

if HAVEMOD_PAC
bin_PROGRAMS = convert_sid
#exclude symlinked files from distribution archive
convert_sid_SOURCES = convert_sid.c
convert_sid_DEPENDENCIES = Makefile.am _reconf
endif #HAVEMOD_PAC

#XXX hack used to ensure depcomp is rerun when Makefile.am files are changed
_reconf: ../_reconf Makefile.am
	@(cd ..; make _reconf)
	@echo "notice: autoconf regeneration complete, rerun make"
	@false

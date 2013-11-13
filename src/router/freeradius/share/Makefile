#
#	Scripts to format dictionary files.
#
#	$Id: ab342b41a9a879fd95f62b8e970f732c0068254b $
#
.PHONY: format

#
#  This should only be run by hand, and then sanity checked by hand!
#
format: dictionary*
	@for x in dictionary* ; do \
		cat $$x | ./format.pl > tmp; \
		mv tmp $$x; \
	done

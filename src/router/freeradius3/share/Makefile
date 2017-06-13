#
#	Scripts to format dictionary files.
#
#	$Id: d3b199c507e20c7f3d95beeeefc5c4274a4eecd0 $
#

#
#  This should only be run by hand, and then sanity checked by hand!
#
format: $(wildcard dictionary*)
	@for x in $(wildcard dictionary*) ; do \
		./format.pl $$x; \
	done

# Layer 1 tests (utils)
PROJECTS += addr
PROJECTS += iterator
PROJECTS += pkt
PROJECTS += rbtree
PROJECTS += rfc6052
PROJECTS += rfc6056
PROJECTS += types

# Layer 2 tests (tables)
PROJECTS += eamt
PROJECTS += bibtable
PROJECTS += sessiontable

# Layer 3 tests (dbs)
PROJECTS += pool4db
PROJECTS += bibdb
PROJECTS += sessiondb

# Layer 4 tests (utils that depend on the dbs)
#PROJECTS += joolns

# Layer 5 tests (translation steps)
PROJECTS += filtering
PROJECTS += translate

# Layer 6 test (global translation)
PROJECTS += page


CLEANPROJECTS = $(patsubst %,%.clean,$(PROJECTS))


all: $(PROJECTS)

$(PROJECTS):
	$(MAKE) -C $@

# Time for a rant:
#
# Kbuild is a fucking idiot. It has no problem creating object files
# outside of the module's directory (as defined by the `-objs` variables),
# but doesn't remove them. This is a major problem for `clean` targets.
#
# There used to be a little command in this target to work around that.
# It looked somewhat like this:
#
# 	find ../../src -type f -name "*.o" -delete
#
# But it had several drawbacks:
#
# 1. It indiscriminately removed all `.o` files in `src/`. Even object files
#    that had nothing to do with unit testing.
# 2. It only ran during this makefile. The individual PROJECTS didn't have it,
#    and thus would leave the garbage around when individually `clean`d.
# 3. It only deleted *object* files.
#
# That last one is actually a real problem. I don't know if it has always been
# like this, but Kbuild now generates additional garbage. And there's no way
# to predict how it will continue polluting our directories in the future. It
# literally does whatever it wants.
#
# It's not Jool's job to compensate for Kbuild's idiocy in the first place.
# It cannot do so correctly. We're going to have to just accept the garbage.
# There's a `deconf.sh` script in the root of the project. It cleans far more
# effectively than this target, if you're interested. But it needs git.
#
# TODO
# Couple of ideas:
# - In each PROJECTS directory, create a relative soft link to `src/`. That
#   might fool Kbuild into thinking that the rest of the project is a child of
#   the unit test. Off the top of my head, I think that Eclipse will go bananas
#   because it will try to index the whole project several times...
# - Do the iptables common.c include trick. It's bound to need loooooots of new
#   files, though...
clean: $(CLEANPROJECTS)

$(CLEANPROJECTS):
	$(MAKE) -C $(@:.clean=) clean

.PHONY: clean $(PROJECTS) $(CLEANPROJECTS)

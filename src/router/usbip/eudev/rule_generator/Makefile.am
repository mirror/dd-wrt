ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

# ------------------------------------------------------------------------------
# rule_generator - persistent network rule generator
# ------------------------------------------------------------------------------
dist_udevlibexec_SCRIPTS = \
        write_net_rules

udevhomedir = $(udevlibexecdir)
dist_udevhome_DATA = rule_generator.functions

dist_udevrules_DATA = \
        75-persistent-net-generator.rules

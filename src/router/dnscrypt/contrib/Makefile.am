EXTRA_DIST = \
	dnscrypt-update-resolvers.sh.in \
	domains-blacklist-local-additions.txt \
	domains-blacklist.conf \
	generate-domains-blacklist.py \
	resolvers-check.sh

noinst_SCRIPTS = \
	dnscrypt-update-resolvers.sh

dnscrypt-update-resolvers.sh: dnscrypt-update-resolvers.sh.in
	$(SED) \
	-e 's|[@]datadir@|$(datadir)|g' \
	-e 's|[@]PACKAGE@|$(PACKAGE)|g' \
	< $< > $@
	chmod u+rwx,go+rx $@

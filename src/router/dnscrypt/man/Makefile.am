man_MANS = \
	dnscrypt-proxy.8 \
	hostip.8

EXTRA_DIST= \
	dnscrypt-proxy.8 \
	dnscrypt-proxy.8.markdown \
	hostip.8 \
	hostip.8.markdown

dnscrypt-proxy.8: dnscrypt-proxy.8.markdown
	@RONN@ dnscrypt-proxy.8.markdown

hostip.8: hostip.8.markdown
	@RONN@ hostip.8.markdown

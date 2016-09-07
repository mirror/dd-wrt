# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Id$

EAPI="4"
inherit subversion

DESCRIPTION="Light DLNA Media Server"
HOMEPAGE="http://xupnpd.org"
SRC_URI=""

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86"

DEPEND="=dev-lang/lua-5.1*"
RDEPEND="${DEPEND}"

ESVN_REPO_URI="http://tsdemuxer.googlecode.com/svn/trunk/xupnpd"

pkg_setup() {
	tc-export CC CXX
}

src_unpack() {
	subversion_src_unpack
	subversion_wc_info
}

src_prepare() {
	cd src || die "Can't cd to src"
	rm -rf lua-5* || die "Faild removing bundled lua"
	epatch "${FILESDIR}/makefile.patch" || die "Failed pathing Makefile"
	sed -i -r -e "/^cfg\.version/s/(=[ ]*'{0,1})([^']*)('{0,1})/\1\2-r${ESVN_WC_REVISION}\3/" \
		xupnpd.lua || die "Failed sed to change version"
	sed -i -r -e "\
		/^cfg/s/cfg\.ssdp_interface='lo'/cfg\.ssdp_interface='eth0'/g;\
		/^cfg/s/cfg\.ssdp_loop=1/cfg\.ssdp_loop=0/g;\
		/^cfg/s/cfg\.debug=1/cfg\.debug=0/g;\
		/^cfg/s/cfg\.daemon=false/cfg\.daemon=true/" \
		xupnpd.lua || die "Failed sed to modify config defaults"
	sed -i -e "/core\.restart/s/\.\/xupnpd/\/usr\/sbin\/xupnpd/" \
		ui/xupnpd_ui.lua || die "Failed sed to modify binary location"

	sed -i -r -e "/^cfg\.pid_file/s/\/var\/run/\/run\/xupnpd/" \
		xupnpd.lua || die "Failed sed to modify pid location"
}

src_compile() {
	cd src || die "Can't cd to src"
	emake || die "emake failed"
}

pkg_preinst() {
	enewgroup xupnpd
	enewuser xupnpd -1 -1 /dev/null xupnpd
}

src_install() {
	dodoc README src/ui/*.txt

	dosbin src/xupnpd

	insinto /usr/share/xupnpd/
	doins src/xupnpd*.lua

	dodir /var/lib/xupnpd/config
	dodir /var/lib/xupnpd/config/postinit

	dodir /usr/share/xupnpd/localmedia

	insinto /usr/share/xupnpd/plugins/
	doins src/plugins/*

	insinto /usr/share/xupnpd/plugins/skel/
	doins src/plugins/skel/*

	insinto /usr/share/xupnpd/plugins/staff/
	doins src/plugins/staff/*

	insinto /usr/share/xupnpd/profiles/
	doins src/profiles/*

	insinto /usr/share/xupnpd/profiles/skel/
	doins src/profiles/skel/*

	insinto /usr/share/xupnpd/www/
	doins src/www/*

	insinto /var/lib/xupnpd/playlists/
	doins src/playlists/*

	insinto /var/lib/xupnpd/playlists/example/
	doins src/playlists/example/*

	insinto /usr/share/xupnpd/ui/
	doins src/ui/*.lua src/ui/*.css src/ui/*.html || die "8"

	dosym /usr/share/xupnpd/xupnpd.lua /etc/xupnpd/xupnpd.lua
	dosym /var/lib/xupnpd/playlists /usr/share/xupnpd/playlists
	dosym /var/lib/xupnpd/config /usr/share/xupnpd/config
	dosym /var/lib/xupnpd/config /etc/xupnpd/config

	newenvd "${FILESDIR}"/xupnpd.envd 99xupnpd
	newinitd "${FILESDIR}/xupnpd.init" xupnpd

	keepdir /run/xupnpd
	fowners xupnpd:xupnpd /run/xupnpd

	fowners -R root:xupnpd /usr/share/xupnpd
	fperms 0775 /usr/share/xupnpd

	fowners -R xupnpd:xupnpd /var/lib/xupnpd
}

pkg_postinst() {
	einfo "Main configuration and plugins: /usr/share/xupnpd"
	einfo "User-defined configuration and playlists: /var/lib/xupnpd"
}

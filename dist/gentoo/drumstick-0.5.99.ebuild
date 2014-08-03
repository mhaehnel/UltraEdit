# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/media-sound/drumstick/drumstick-0.5.0.ebuild,v 1.8 2013/12/03 07:40:56 ssuominen Exp $

EAPI=4

inherit base cmake-utils fdo-mime gnome2-utils subversion

DESCRIPTION="Qt4/C++ wrapper for ALSA sequencer"
HOMEPAGE="http://drumstick.sourceforge.net/"
#SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"

ESVN_REPO_URI="svn://svn.code.sf.net/p/drumstick/code/trunk"
ESVN_PROJECT="${PN/-svn}"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE="dbus doc"

RDEPEND="media-libs/alsa-lib
	dev-qt/qtgui:5
	dev-qt/qtsvg:5
	x11-misc/shared-mime-info
	dbus? ( dev-qt/qtdbus:5 )"
DEPEND="${RDEPEND}
	virtual/pkgconfig
	doc? (
		app-doc/doxygen
		app-text/docbook-xsl-stylesheets
		dev-libs/libxslt
	)
"

DOCS=( AUTHORS ChangeLog NEWS README TODO )

#PATCHES=(
#	"${FILESDIR}"/${PV}-underlinking.patch
#)

src_prepare() {
	sed -i \
		-e '/CMAKE_EXE_LINKER_FLAGS/d' \
		CMakeLists.txt || die
	base_src_prepare
}

src_configure() {
	local mycmakeargs=(
		$(cmake-utils_use_use dbus)
		$(cmake-utils_use_with doc)
	)

	cmake-utils_src_configure
}

pkg_preinst() {
	gnome2_icon_savelist
}

pkg_postinst() {
	fdo-mime_desktop_database_update
	fdo-mime_mime_database_update
	gnome2_icon_cache_update
}

pkg_postrm() {
	fdo-mime_desktop_database_update
	fdo-mime_mime_database_update
	gnome2_icon_cache_update
}

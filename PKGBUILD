# Maintainer: Jesse McClure AKA "Trilby" <jmcclure [at] cns [dot] umass [dot] edu>
_gitname="squabble"
pkgname="${_gitname}-git"
pkgver=0.0
pkgrel=1
pkgdesc="An interactive crossword puzzle board game"
url="http://trilbywhite.github.io/squabble/"
arch=('any')
license=('GPLv3')
depends=('libx11' 'cairo')
makedepends=('git')
source=("${_gitname}::git://github.com/TrilbyWhite/squabble.git")
sha256sums=('SKIP')

pkgver() {
	cd "${_gitname}";
	echo "0.$(git rev-list --count HEAD).$(git describe --always )"
}

build() {
	cd "${_gitname}"
	make
}

package() {
	cd "${_gitname}"
	make PREFIX=/usr DESTDIR="${pkgdir}" install
}

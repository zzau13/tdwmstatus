pkgname=tdwmstatus
pkgver=2.74-1
pkgver(){
  cd $pkgname
  git describe --tags |sed 's/-/./g'
}
pkgrel=1
pkgdesc="Status bar for dwm"
url="git+https://BotCats@bitbucket.org/BotCats/tdwmstatus.git"
arch=('i686' 'x86_64')
license=('MIT')
depends=('libx11' 'alsa-lib')
makedepends=('git')
provides=('tdwmstatus')
conflicts=('')
source=($url)
md5sums=('SKIP')

build() {
  cd $pkgname
  make
}

package() {
  make -C $pkgname DESTDIR=$pkgdir install
}

# vim:set ts=2 sw=2 et:

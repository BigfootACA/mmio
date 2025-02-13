# Maintainer: BigfootACA <bigfoot@classfun.cn>

pkgname=mmio
pkgver=0.1
pkgrel=1
pkgdesc="Read and write /dev/mem device"
arch=(x86_64 i686 pentium4 arm armv6h armv7h aarch64)
url="https://github.com/BigfootACA"
license=(GPL-3.0-or-later)
depends=(glibc)
makedepends=(gcc make)
source=(
	LICENSE
	Makefile
	mmio.c
)
md5sums=(SKIP SKIP SKIP)

pkgver(){
	cd "$(dirname "$(realpath "$srcdir/mmio.c")")"
	cnt="$(git rev-list --count HEAD 2>/dev/null||true)"
	sha="$(git rev-parse --short HEAD 2>/dev/null||true)"
	if [ -n "$cnt" ] && [ -n "$sha" ]; then
		printf "r%s.%s" "$cnt" "$sha"
	else
		printf "0.1"
	fi
}

build(){
	make
}

package() {
	make DESTDIR="$pkgdir" install
	install -vDm644 LICENSE -t "$pkgdir/usr/share/licenses/$pkgname"
}

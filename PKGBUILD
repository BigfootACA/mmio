# Maintainer: BigfootACA <bigfoot@classfun.cn>

pkgname=mmio
pkgver=0.2
pkgrel=1
pkgdesc="Read and write /dev/mem device"
arch=(x86_64 i686 pentium4 arm armv6h armv7h aarch64 loong64 riscv32 riscv64)
url="https://github.com/BigfootACA"
license=(GPL-3.0-or-later)
depends=(glibc)
makedepends=(gcc meson ninja)
source=(LICENSE meson.build)
md5sums=(SKIP SKIP)

pkgver(){
	cd "$(dirname "$(realpath "$srcdir/meson.build")")"
	cnt="$(git rev-list --count HEAD 2>/dev/null||true)"
	sha="$(git rev-parse --short HEAD 2>/dev/null||true)"
	if [ -n "$cnt" ] && [ -n "$sha" ]; then
		printf "r%s.%s" "$cnt" "$sha"
	else
		printf "0.2"
	fi
}

build(){
	mkdir -p "$srcdir/build"
	DIR="$(dirname "$(realpath "$srcdir/meson.build")")"
	for file in "$DIR/"*.{c,h}; do
		name="$(basename "$file")"
		if [ -h "$name" ] || ! [ -e "$name" ]; then
			ln -sf "$file" "$name"
		fi
	done
	arch-meson "$srcdir" "$srcdir/build"
	ninja -C "$srcdir/build"
}

package() {
	DESTDIR="$pkgdir" ninja -C "$srcdir/build" install
	install -vDm644 LICENSE -t "$pkgdir/usr/share/licenses/$pkgname"
}

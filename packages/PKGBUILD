# Maintainer: Your Name <your.email@example.com>
pkgname=open-yolo
pkgver=1.0.0
pkgrel=1
pkgdesc="Gestionnaire de curseurs personnalisés pour Linux avec support des curseurs animés"
arch=('x86_64')
url="https://github.com/Roddygithub/Open-Yolo"
license=('MIT')
depends=(
    'gtkmm3'
    'sdl2'
    'sdl2_image'
    'glew'
    'giflib'
    'libx11'
    'libxrandr'
    'libxcursor'
    'libxinerama'
    'libxi'
    'mesa'
)
makedepends=(
    'cmake'
    'ninja'
    'git'
    'pkgconf'
)
optdepends=(
    'gtest: pour exécuter les tests unitaires'
)
source=("git+$url.git#tag=v$pkgver")
sha256sums=('SKIP')

build() {
    cd "$srcdir/$pkgname"
    
    cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DENABLE_LOGGING=ON \
        -DENABLE_LTO=ON \
        -DBUILD_TESTS=OFF
    
    cmake --build build
}

check() {
    cd "$srcdir/$pkgname"
    
    # Reconstruire avec les tests
    cmake -B build-test -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON
    
    cmake --build build-test
    
    cd build-test
    ctest --output-on-failure
}

package() {
    cd "$srcdir/$pkgname"
    
    DESTDIR="$pkgdir" cmake --install build
    
    # Installer la documentation
    install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
    
    # Installer les fichiers de configuration
    install -Dm644 resources/OpenYolo.desktop "$pkgdir/usr/share/applications/OpenYolo.desktop"
    
    # Installer les icônes
    for size in 16 22 24 32 48 64 128 256; do
        if [ -f "resources/icons/hicolor/${size}x${size}/apps/openyolo.png" ]; then
            install -Dm644 "resources/icons/hicolor/${size}x${size}/apps/openyolo.png" \
                "$pkgdir/usr/share/icons/hicolor/${size}x${size}/apps/openyolo.png"
        fi
    done
}

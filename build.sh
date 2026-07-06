#!/bin/sh
set -e
cd "$(dirname "$0")"
ROOT=$(pwd)
BR_HOST=buildroot-2026.05/output/host
export PATH="$BR_HOST/bin:$PATH"
CROSS=mipsel-buildroot-linux-musl-

# --- vendor doomgeneric (gitignored) ---
[ -d doomgeneric/.git ] || git clone --depth 1 https://github.com/ozkl/doomgeneric.git doomgeneric

# --- apply local patches (later tasks), no-op if patches/ empty ---
for p in patches/*.patch; do
  [ -e "$p" ] || continue
  patch -d doomgeneric -p1 -N --forward < "$p" || true
done

# --- pin render resolution to the panel (320x200 letterboxed into 320x240) ---
sed -i 's/#define DOOMGENERIC_RESX .*/#define DOOMGENERIC_RESX 320/' doomgeneric/doomgeneric/doomgeneric.h
sed -i 's/#define DOOMGENERIC_RESY .*/#define DOOMGENERIC_RESY 200/' doomgeneric/doomgeneric/doomgeneric.h

# --- tinyalsa 1.1.1 static (same version erosq-ui pins) ---
[ -f tinyalsa/src/libtinyalsa.a ] || {
  [ -d tinyalsa ] || git clone --depth 1 --branch 1.1.1 https://github.com/tinyalsa/tinyalsa.git tinyalsa
  # musl doesn't pull in ssize_t transitively via <stdlib.h> like glibc does;
  # same fix erosq-ui carries locally (uncommitted) on this exact pinned commit.
  grep -q 'sys/types.h' tinyalsa/include/tinyalsa/interval.h || \
    sed -i '/#include <stdlib.h>/a #include <sys/types.h> \/* ssize_t (musl doesn'"'"'t pull it in transitively) *\/' tinyalsa/include/tinyalsa/interval.h
  make -C tinyalsa/src CROSS_COMPILE=$CROSS libtinyalsa.a
}

make -C . -f Makefile.erosq CROSS=$CROSS
echo "binary: $ROOT/build/doomgeneric"

#!/bin/sh
set -e
cd "$(dirname "$0")"
ROOT=$(pwd)
BR_HOST=buildroot-2026.08/output/host
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

# --- Rebind for the erosq's limited buttons: the Play button emits KEY_ENTER
#     (menu select). Make ENTER also fire in-game and confirm yes/no prompts, so
#     one button does select + fire + quit-confirm (clean exit back to the UI).
#     Idempotent. ---
sed -i 's/^int key_fire = KEY_FIRE;/int key_fire = KEY_ENTER;/' doomgeneric/doomgeneric/m_controls.c
sed -i "s/^int key_menu_confirm   = 'y';/int key_menu_confirm   = KEY_ENTER;/" doomgeneric/doomgeneric/m_controls.c
# key_fire=ENTER collides with key_message_refresh (also KEY_ENTER by default):
# HU_Responder runs before the fire keydown in G_Responder and eats ENTER as
# "refresh message", so Play never fired in-game. Move refresh to '/' (no erosq
# button emits it; matches doom's Strife default). Idempotent.
sed -i "s|^int key_message_refresh = KEY_ENTER;|int key_message_refresh = '/';|" doomgeneric/doomgeneric/m_controls.c

# --- FEATURE_SOUND drags <SDL_mixer.h> into i_sound.c, but nothing from it is
#     used there (the real SDL backend is i_sdlsound.c, which we exclude). Drop
#     the dead include so -DFEATURE_SOUND links our tinyalsa DG_sound_module.
#     Idempotent. ---
sed -i 's|#include <SDL_mixer.h>|/* SDL_mixer.h removed: erosq uses tinyalsa (src/sound_erosq.c) */|' \
    doomgeneric/doomgeneric/i_sound.c

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

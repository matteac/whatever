#!/bin/sh
set -eu

: "${CC:=cc}"
CC_DEFS="-D_GNU_SOURCE"
CC_FLAGS="-std=c23 -Wall -Wextra -Wpedantic -O0 -g -ggdb"
CC_INCLS="-Isrc -Iprotocols -I/usr/include/freetype2"
CC_LIBS="-lwayland-client -lm -lcairo -lfreetype -lharfbuzz"

BUILD="build"
OBJS="$BUILD/objs"

_pre() {
    mkdir -p "$BUILD"
}

_obj_pre() {
    mkdir -p "$OBJS"
}

_xdg() {
    _obj_pre
    $CC -c protocols/xdg-shell/*.c -o "$OBJS/xdg-shell.o"
}

_wlr() {
    _obj_pre
    $CC -c protocols/wlr-layer-shell/*.c -o "$OBJS/wlr-layer-shell.o"
}

build() {
    _pre
    _wlr
    _xdg
    $CC src/*.c -o "$BUILD/whatever" "$OBJS"/*.o \
        $CC_DEFS $CC_INCLS $CC_LIBS $CC_FLAGS
}

run() {
    build
    exec "$BUILD/whatever"
}


build_profile() {
    _pre
    _wlr
    _xdg
    $CC src/*.c "$OBJS"/*.o \
        -o "$BUILD/whatever" \
        -DPROFILE \
        $CC_DEFS $CC_INCLS $CC_LIBS $CC_FLAGS
}

profile() {
    build_profile
    exec "$BUILD/whatever"
}


case "${1:-build}" in
    build)
      set -xeu
      build
    ;;
    run)
      set -xeu
      run
    ;;
    build_profile)
      set -xeu
      build_profile
    ;;
    profile)
      set -xeu
      profile
    ;;
    *)
      echo "Usage: $0 {build|run|build_profile|profile}" >&2
      exit 1
    ;;
esac

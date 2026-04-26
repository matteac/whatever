#!/bin/sh
set -eu

: "${CC:=cc}"
CC_DEFS="-D_GNU_SOURCE"
CC_FLAGS="-Wall -Wextra -Wpedantic"
CC_INCLS="-Isrc -Iprotocols"
CC_LIBS="-lwayland-client -lcairo"

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
    $CC src/*.c -o "$BUILD/main" "$OBJS"/*.o \
        $CC_DEFS $CC_INCLS $CC_LIBS $CC_FLAGS
}

run() {
    build
    exec "$BUILD/main"
}

set -xeu
case "${1:-build}" in
    build) build ;;
    run) run ;;
    *)
        echo "Usage: $0 {build|run}" >&2
        exit 1
        ;;
esac

default: build

xdg:
    cc -c protocols/xdg-shell/xdg-shell.c -o build/obj/xdg-shell.o
wlr:
    cc -c protocols/wlr-layer-shell/wlr-layer-shell.c -o build/obj/wlr-layer-shell.o

build: wlr xdg
    cc src/main.c src/core.c build/obj/*.o -o build/main -D_GNU_SOURCE -Iprotocols -lwayland-client -Wall -Wextra -Wpedantic

run: build
    build/main

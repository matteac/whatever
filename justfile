default: build

CC := env_var_or_default("CC", "cc")
CC_DEFS  := "-D_GNU_SOURCE"
CC_FLAGS := "-Wall -Wextra -Wpedantic"
CC_INCLS := "-Isrc -Iprotocols"
CC_LIBS  := "-lwayland-client -lcairo"

BUILD := "build"
OBJS  := f'{{BUILD}}/objs'

pre:
    @mkdir -p {{BUILD}}
obj_pre:
    @mkdir -p {{OBJS}}

xdg: obj_pre
    {{CC}} -c protocols/xdg-shell/*.c -o {{OBJS}}/xdg-shell.o
wlr: obj_pre
    {{CC}} -c protocols/wlr-layer-shell/*.c -o {{OBJS}}/wlr-layer-shell.o

build: pre wlr xdg
    {{CC}} src/*.c -o {{BUILD}}/main {{OBJS}}/*.o {{CC_DEFS}} {{CC_INCLS}} {{CC_LIBS}} {{CC_FLAGS}}

run: build
    {{BUILD}}/main

# whatever

> **Under development**

## Build from source

To build this project from source, you need:

- `wayland` dev libraries
- `cairo` dev libraries
- A C compiler
- Optionally, `just` if you want to use the `justfile`

### justfile
You can build the project using the included `justfile`:
```sh
just build # you can set your C compiler by setting the CC env var
# CC="clang" just build
```

### build.sh
Or using any POSIX shell with `build.sh`:
```sh
sh build.sh # you can set your C compiler by setting the CC env var
# CC="clang" sh build.sh
```
> NOTE: `build.sh` may be outdated compared to `justfile`

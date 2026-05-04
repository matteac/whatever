# whatever

> **Under development**

Minimal Wayland App Launcher.

## Build from source

### Requirements

* C compiler (`cc`, `clang`, etc.)
* Development packages:
  * `wayland-client`
  * `cairo`
  * `freetype2`
  * `harfbuzz`

> Package names may vary depending on your distribution.

---

### Using `just` (recommended)
```sh
just build
# CC=clang just build
```

---

### Using `build.sh`
```sh
sh build.sh
# CC=clang sh build.sh
```
> `justfile` is the primary build definition. `build.sh` may lag behind.


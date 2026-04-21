#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <wayland-client.h>

#include "wlr-layer-shell/wlr-layer-shell.h"

#include "log.h"

struct we_app;
struct we_window;

typedef void (*we_update_callback)(void *update_data, struct we_window *win);
typedef void (*we_render_callback)(void *render_data, struct we_window *win);

typedef struct we_app {
  struct we_window *win;

  struct wl_display *display;
  struct wl_registry *registry;

  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct wl_shm *shm;
  struct wl_seat *seat;

  struct wl_keyboard *kbd;

  bool running;
} we_app;

bool we_app_init(we_app *app);
bool we_app_deinit(we_app *app);
void we_app_run(we_app *app);

typedef struct we_window {
  struct we_app *app;

  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;

  uint32_t width;
  uint32_t height;
  bool configured;

  struct wl_buffer *buffer;
  void *shm_data;
  size_t shm_size;

  we_update_callback update_callback;
  void *update_data;
  we_render_callback render_callback;
  void *render_data;
} we_window;

bool we_window_init(we_window *win, uint32_t width, uint32_t height, enum zwlr_layer_surface_v1_anchor anchor, we_app *app);
bool we_window_deinit(we_window *win);
void we_window_set_render(we_window *win, we_render_callback cb, void *data);
void we_window_set_update(we_window *win, we_update_callback cb, void *data);

#include "core.h"
#include "listeners.h"

bool we_app_init(we_app *app) {
  assert(app != NULL);

  // NOTE(matt): WAYLAND_DISPLAY is handled by libwayland-client
  app->display = wl_display_connect(NULL);
  assert(app->display != NULL);

  app->registry = wl_display_get_registry(app->display);
  assert(app->registry);
  wl_registry_add_listener(app->registry, &registry_listener, app);

  wl_display_roundtrip(app->display);

  assert(app->compositor != NULL);
  assert(app->layer_shell != NULL);
  assert(app->shm != NULL);
  assert(app->seat != NULL);

  wl_shm_add_listener(app->shm, &shm_listener, app);
  wl_seat_add_listener(app->seat, &seat_listener, app);

  wl_display_roundtrip(app->display);

  // NOTE(matt): maybe check for xrgb as fallback, but argb should be quite ubiquitous
  assert(app->argb8888_support == true);

  return true;
}

bool we_app_deinit(we_app *app) {
  assert(app != NULL);

  if (app->win)
    we_window_deinit(app->win);

  if (app->kbd)
    wl_keyboard_destroy(app->kbd);

  if (app->seat)
    wl_seat_destroy(app->seat);
  if (app->shm)
    wl_shm_destroy(app->shm);
  if (app->layer_shell)
    zwlr_layer_shell_v1_destroy(app->layer_shell);
  if (app->compositor)
    wl_compositor_destroy(app->compositor);

  if (app->registry)
    wl_registry_destroy(app->registry);
  if (app->display)
    wl_display_disconnect(app->display);

  memset(app, 0, sizeof(we_app));
  return true;
}

void we_app_run(we_app *app) {
  assert(app != NULL);

  app->running = true;
  while (app->running) {
    int32_t dispatched = wl_display_dispatch(app->display);
    if (dispatched < 0) {
      LOG_ERR("dispatch error: %d", dispatched);
      app->running = false;
    }
  }
}

bool we_window_init(we_window *win, uint32_t width, uint32_t height, enum zwlr_layer_surface_v1_anchor anchor, we_app *app) {
  assert(win != NULL);
  assert(app != NULL);

  win->app = app;
  win->width = width;
  win->height = height;

  win->surface = wl_compositor_create_surface(app->compositor);
  assert(win->surface != NULL);

  win->layer_surface =
      zwlr_layer_shell_v1_get_layer_surface(app->layer_shell, win->surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_TOP, "whatever");
  assert(win->layer_surface != NULL);

  zwlr_layer_surface_v1_set_size(win->layer_surface, win->width, win->height);
  zwlr_layer_surface_v1_set_anchor(win->layer_surface, anchor);
  zwlr_layer_surface_v1_set_keyboard_interactivity(win->layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND);

  zwlr_layer_surface_v1_add_listener(win->layer_surface, &layer_surface_listener, win);

  wl_surface_commit(win->surface);

  return true;
}

bool we_window_deinit(we_window *win) {
  assert(win != NULL);

  if (win->shm_data)
    munmap(win->shm_data, win->shm_size);
  if (win->buffer)
    wl_buffer_destroy(win->buffer);

  if (win->layer_surface)
    zwlr_layer_surface_v1_destroy(win->layer_surface);
  if (win->surface)
    wl_surface_destroy(win->surface);

  memset(win, 0, sizeof(we_window));
  return true;
}

void we_window_set_render(we_window *win, we_render_callback cb, void *data) {
  assert(win != NULL);
  win->render_callback = cb;
  win->render_data = data;
}
void we_window_set_update(we_window *win, we_update_callback cb, void *data) {
  assert(win != NULL);
  win->update_callback = cb;
  win->update_data = data;
}

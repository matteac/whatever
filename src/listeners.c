#include "core.h"

void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version);
void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name);
static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format);
static const struct wl_shm_listener shm_listener = {
    .format = shm_format,
};

void seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities);
void seat_name(void *data, struct wl_seat *wl_seat, const char *name);
static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
    .name = seat_name,
};

void layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1, uint32_t serial, uint32_t width,
                             uint32_t height);
void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1);
static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

void frame_callback_done(void *data, struct wl_callback *callback, uint32_t callback_data);
static const struct wl_callback_listener frame_callback_listener = {.done = frame_callback_done};

// registry impl
void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *iface, uint32_t version) {
  if (data == NULL) {
    LOG_ERR("data is null, should be we_app*");
    return;
  }

  we_app *app = data;

  if /*  */ (!strcmp(iface, wl_compositor_interface.name)) {
    app->compositor = wl_registry_bind(app->registry, name, &wl_compositor_interface, version);
    assert(app->compositor != NULL);
  } else if (!strcmp(iface, zwlr_layer_shell_v1_interface.name)) {
    app->layer_shell = wl_registry_bind(app->registry, name, &zwlr_layer_shell_v1_interface, version);
    assert(app->layer_shell != NULL);
  } else if (!strcmp(iface, wl_shm_interface.name)) {
    app->shm = wl_registry_bind(app->registry, name, &wl_shm_interface, version);
    assert(app->shm != NULL);
  } else if (!strcmp(iface, wl_seat_interface.name)) {
    app->seat = wl_registry_bind(app->registry, name, &wl_seat_interface, version);
    assert(app->seat != NULL);
  }
}
void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) { LOG_WARN("Unimplemented"); }

// shm impl
void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format) {
  if (data == NULL) {
    LOG_ERR("data is null, should be we_app*");
    return;
  }
  we_app *app = data;

  if (format == WL_SHM_FORMAT_ARGB8888) {
    LOG_INFO("ARGB8888 support");
    app->argb8888_support = true;
  }
}

// seat impl
void seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) { LOG_WARN("Unimplemented"); }
void seat_name(void *data, struct wl_seat *wl_seat, const char *name) { LOG_WARN("Unimplemented"); }

// layer surface impl
void layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface, uint32_t serial, uint32_t width, uint32_t height) {
  if (data == NULL) {
    LOG_ERR("data is null, should be we_window*");
    return;
  }
  we_window *win = data;

  zwlr_layer_surface_v1_ack_configure(win->layer_surface, serial);
}
void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *layer_surface) { LOG_WARN("Unimplemented"); }

// frame callback impl
void frame_callback_done(void *data, struct wl_callback *callback, uint32_t callback_data) { LOG_WARN("Unimplemented"); }

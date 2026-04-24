#include "core.h"

bool create_shm_buffer(struct wl_shm *shm, uint32_t width, uint32_t height, uint32_t stride, enum wl_shm_format format,
                       struct wl_buffer **buff_out, void **data_out, size_t *size_out) {
  assert(shm != NULL);
  assert(buff_out != NULL);
  assert(data_out != NULL);

  if (width == 0 || height == 0)
    return false;

  size_t size = stride * height;

  int fd = memfd_create("wl_shm", MFD_CLOEXEC);
  if (fd < 0)
    return false;

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return false;
  }

  void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close(fd);
    return false;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  if (pool == NULL) {
    munmap(data, size);
    close(fd);
    return false;
  }

  struct wl_buffer *buff = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);

  wl_shm_pool_destroy(pool);
  close(fd);

  if (buff == NULL) {
    munmap(data, size);
    return false;
  }

  *buff_out = buff;
  *data_out = data;
  if (size_out)
    *size_out = size;

  return true;
}

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

  if (width == 0)
    width = win->width;
  if (height == 0)
    height = win->height;

  if (width == 0 || height == 0) {
    LOG_ERR("Invalid configure size: %ux%u", width, height);
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
    return;
  }

  bool resized = (width != win->width || height != win->height);

  if (resized) {
    if (win->buffer) {
      wl_buffer_destroy(win->buffer);
      win->buffer = NULL;
    }

    if (win->shm_data) {
      munmap(win->shm_data, win->shm_size);
      win->shm_data = NULL;
      win->shm_size = 0;
    }
  }

  win->width = width;
  win->height = height;

  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

  if (!win->buffer) {
    if (!win->app->argb8888_support) {
      LOG_ERR("ARGB8888 support needed");
      return;
    }

    uint32_t stride = width * 4;
    enum wl_shm_format format = WL_SHM_FORMAT_ARGB8888;

    if (!create_shm_buffer(win->app->shm, width, height, stride, format, &win->buffer, &win->shm_data, &win->shm_size)) {
      LOG_ERR("Error creating shm buffer");
      return;
    }
  }

  struct wl_callback *frame_callback = wl_surface_frame(win->surface);
  wl_callback_add_listener(frame_callback, &frame_callback_listener, win);

  wl_surface_attach(win->surface, win->buffer, 0, 0);
  wl_surface_damage(win->surface, 0, 0, width, height);
  wl_surface_commit(win->surface);
}
void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *layer_surface) { LOG_WARN("Unimplemented"); }

// frame callback impl
void frame_callback_done(void *data, struct wl_callback *callback, uint32_t callback_data) {
  if (data == NULL) {
    LOG_ERR("data is null, should be we_window*");
    return;
  }

  we_window *win = data;

  // uint32_t *pixels = win->shm_data;
  // for (uint32_t x = 0; x <= win->width; x++) {
  //   for (uint32_t y = 0; y <= win->height; y++) {
  //     pixels[y * win->width + x] = 0xf0141414;
  //   }
  // }

  if (win->render_callback != NULL) {
    win->render_callback(win->render_data, win);
  }

  wl_callback_destroy(callback);

  struct wl_callback *next_frame_callback = wl_surface_frame(win->surface);
  wl_callback_add_listener(next_frame_callback, &frame_callback_listener, win);
  wl_surface_attach(win->surface, win->buffer, 0, 0);
  wl_surface_damage(win->surface, 0, 0, win->width, win->height);
  wl_surface_commit(win->surface);
}

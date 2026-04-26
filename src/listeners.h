#pragma once

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

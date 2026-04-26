#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "we_string.h"

struct we_window;

typedef struct we_color {
  union {
    struct {
      uint8_t a, b, g, r;
    };
    uint8_t c[4];
    uint32_t d;
  };
} we_color;

typedef struct we_vec2i {
  union {
    struct {
      int32_t x, y;
    };
    int32_t c[2];
  };
} we_vec2i;

typedef struct we_image {
  uint32_t *pixels;
  uint32_t width;
  uint32_t height;
} we_image;

typedef struct we_font we_font;

we_font *we_font_load(const char *name, uint32_t size, bool italic);
void we_font_destroy(we_font *font);

typedef struct we_renderer we_renderer;

we_renderer *we_renderer_create(struct we_window *win);
void we_renderer_destroy(we_renderer *renderer);

void we_clear(we_renderer *r, we_color color);
void we_fill_rect(we_renderer *r, we_vec2i pos, we_vec2i size, we_color color);
void we_fill_circle(we_renderer *r, we_vec2i center, int32_t radius, we_color color);
void we_fill_triangle(we_renderer *r, we_vec2i a, we_vec2i b, we_vec2i c, we_color color);
void we_blit_scaled(we_renderer *r, we_image *img, we_vec2i pos, we_vec2i size);
void we_draw_text(we_renderer *r, we_font *font, we_string_view *text, we_vec2i pos, we_color color);

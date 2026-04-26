#include <assert.h>
#include <cairo/cairo.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "renderer.h"

static inline void we_set_source_color(cairo_t *cr, we_color c) {
  cairo_set_source_rgba(cr, c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
}

struct we_renderer {
  cairo_surface_t *surface;
  cairo_t *cr;

  uint32_t *pixels;
  uint32_t width;
  uint32_t height;
  uint32_t stride;
};

struct we_font {
  cairo_font_face_t *face;
  uint32_t size;
};

we_font *we_font_load(const char *name, uint32_t size, bool italic) {
  we_font *font = malloc(sizeof(*font));
  if (!font)
    return NULL;

  cairo_font_slant_t slant = italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL;

  font->size = size;
  font->face = cairo_toy_font_face_create(name, slant, CAIRO_FONT_WEIGHT_NORMAL);

  return font;
}
void we_font_destroy(we_font *font) {
  if (!font)
    return;

  if (font->face)
    cairo_font_face_destroy(font->face);

  free(font);
}

we_renderer *we_renderer_create(struct we_window *win) {
  assert(win != NULL);
  assert(win->shm_data != NULL);

  we_renderer *r = malloc(sizeof(we_renderer));
  if (r == NULL)
    return NULL;

  r->width = win->width;
  r->height = win->height;
  r->pixels = win->shm_data;

  cairo_surface_t *s = cairo_image_surface_create_for_data(win->shm_data, CAIRO_FORMAT_ARGB32, r->width, r->height, win->width * 4);
  assert(s != NULL);

  cairo_t *cr = cairo_create(s);
  assert(cr != NULL);

  r->cr = cr;
  r->surface = s;

  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

  return r;
}

void we_renderer_destroy(we_renderer *r) {
  assert(r != NULL);

  if (r->cr)
    cairo_destroy(r->cr);
  if (r->surface)
    cairo_surface_destroy(r->surface);
  free(r);
}

void we_clear(we_renderer *r, we_color color) {
  cairo_t *cr = r->cr;

  cairo_save(cr);
  we_set_source_color(cr, color);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);
  cairo_restore(cr);
}

void we_fill_rect(we_renderer *r, we_vec2i pos, we_vec2i size, we_color color) {
  cairo_t *cr = r->cr;

  cairo_save(cr);
  we_set_source_color(cr, color);
  cairo_rectangle(cr, pos.x, pos.y, size.x, size.y);
  cairo_fill(cr);
  cairo_restore(cr);
}

void we_fill_circle(we_renderer *r, we_vec2i center, int32_t radius, we_color color) {
  cairo_t *cr = r->cr;

  cairo_save(cr);
  we_set_source_color(cr, color);
  cairo_arc(cr, center.x, center.y, radius, 0.0, 2.0 * M_PI);
  cairo_fill(cr);
  cairo_restore(cr);
}

void we_fill_triangle(we_renderer *r, we_vec2i a, we_vec2i b, we_vec2i c, we_color color) {
  cairo_t *cr = r->cr;

  cairo_save(cr);
  we_set_source_color(cr, color);

  cairo_move_to(cr, a.x, a.y);
  cairo_line_to(cr, b.x, b.y);
  cairo_line_to(cr, c.x, c.y);
  cairo_close_path(cr);

  cairo_fill(cr);
  cairo_restore(cr);
}

void we_blit_scaled(we_renderer *r, we_image *img, we_vec2i pos, we_vec2i size) {
  cairo_t *cr = r->cr;

  cairo_surface_t *surface =
      cairo_image_surface_create_for_data((unsigned char *)img->pixels, CAIRO_FORMAT_ARGB32, img->width, img->height, img->width * 4);

  cairo_save(cr);

  cairo_translate(cr, pos.x, pos.y);
  cairo_scale(cr, (double)size.x / img->width, (double)size.y / img->height);

  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_BILINEAR);
  cairo_paint(cr);

  cairo_restore(cr);
  cairo_surface_destroy(surface);
}

void we_draw_text(we_renderer *r, we_font *font, we_string_view *text, we_vec2i pos, we_color color) {
  assert(font != NULL);
  assert(font->face != NULL);
  cairo_t *cr = r->cr;

  char buf[text->len + 1];
  memcpy(buf, text->data, text->len);
  buf[text->len] = '\0';

  cairo_save(cr);

  we_set_source_color(cr, color);
  cairo_set_font_face(cr, font->face);
  cairo_set_font_size(cr, font->size);
  cairo_move_to(cr, pos.x, pos.y);
  cairo_show_text(cr, buf);
  cairo_restore(cr);
}

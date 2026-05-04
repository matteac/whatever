#include <assert.h>
#include <cairo/cairo.h>
#include <stdlib.h>
#include <string.h>

#include <freetype/freetype.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb.h>

#include "core.h"
#include "renderer.h"

static FT_Library ft;
static bool ft_initialized = false;

static inline void we_set_source_color(cairo_t *cr, we_color c) {
  cairo_set_source_rgba(cr, c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
}

#define WE_GLYPH_CACHE_SIZE 512

typedef struct {
  uint32_t glyph_id;

  uint32_t width;
  uint32_t height;
  uint32_t left;
  uint32_t top;
  int32_t pitch;

  uint8_t *buffer;
} glyph_cache_entry;

static glyph_cache_entry *glyph_cache_lookup(glyph_cache_entry *cache, uint32_t glyph_id) {
  for (size_t i = 0; i < WE_GLYPH_CACHE_SIZE; i++) {
    if (cache[i].glyph_id == glyph_id)
      return &cache[i];
  }
  return NULL;
}

static glyph_cache_entry *glyph_cache_insert(glyph_cache_entry *cache, FT_GlyphSlot g, uint32_t glyph_id) {
  uint32_t slot = glyph_id % WE_GLYPH_CACHE_SIZE;
  glyph_cache_entry *e = &cache[slot];

  if (e->buffer)
    free(e->buffer);

  e->glyph_id = glyph_id;
  e->width = g->bitmap.width;
  e->height = g->bitmap.rows;
  e->left = g->bitmap_left;
  e->top = g->bitmap_top;
  e->pitch = g->bitmap.pitch;

  uint32_t size = abs(e->pitch) * e->height;

  e->buffer = malloc(size);
  memcpy(e->buffer, g->bitmap.buffer, size);

  return e;
}

struct we_renderer {
  cairo_surface_t *surface;
  cairo_t *cr;

  uint32_t *pixels;
  uint32_t width;
  uint32_t height;
  uint32_t stride;

  hb_buffer_t *hb_buffer;

  glyph_cache_entry glyph_cache[WE_GLYPH_CACHE_SIZE];
};

we_renderer *we_renderer_create(struct we_window *win) {
  assert(win && win->shm_data);

  we_renderer *r = calloc(1, sizeof(*r));

  r->width = win->width;
  r->height = win->height;
  r->pixels = win->shm_data;

  r->surface = cairo_image_surface_create_for_data(win->shm_data, CAIRO_FORMAT_ARGB32, r->width, r->height, win->width * 4);
  r->cr = cairo_create(r->surface);

  cairo_set_operator(r->cr, CAIRO_OPERATOR_SOURCE);

  r->hb_buffer = hb_buffer_create();

  return r;
}

void we_renderer_destroy(we_renderer *r) {
  if (!r)
    return;

  hb_buffer_destroy(r->hb_buffer);

  for (size_t i = 0; i < WE_GLYPH_CACHE_SIZE; i++)
    free(r->glyph_cache[i].buffer);

  cairo_destroy(r->cr);
  cairo_surface_destroy(r->surface);

  free(r);
}

struct we_font {
  FT_Face face;
  hb_font_t *hb_font;
  uint32_t size;
};

we_font *we_font_load(const char *name, uint32_t size) {
  assert(name);

  if (!ft_initialized) {
    FT_Init_FreeType(&ft);
    ft_initialized = true;
  }

  we_font *font = calloc(1, sizeof(*font));

  FT_New_Face(ft, name, 0, &font->face);
  FT_Set_Pixel_Sizes(font->face, 0, size);

  font->hb_font = hb_ft_font_create(font->face, NULL);
  font->size = size;

  return font;
}

we_font *we_font_load_from_memory(const unsigned char *data, size_t data_size, uint32_t size) {
  assert(data);

  if (!ft_initialized) {
    FT_Init_FreeType(&ft);
    ft_initialized = true;
  }

  we_font *font = calloc(1, sizeof(*font));

  FT_New_Memory_Face(ft, data, data_size, 0, &font->face);
  FT_Set_Pixel_Sizes(font->face, 0, size);

  font->hb_font = hb_ft_font_create(font->face, NULL);
  font->size = size;

  return font;
}

void we_font_destroy(we_font *font) {
  if (!font)
    return;

  hb_font_destroy(font->hb_font);
  FT_Done_Face(font->face);

  free(font);
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

static void draw_bitmap(we_renderer *r, glyph_cache_entry *g, uint32_t x, uint32_t y, we_color color) {
  uint8_t *dst = (uint8_t *)r->pixels;
  uint32_t width = r->width;
  uint32_t height = r->height;
  uint32_t stride = 4 * width;

  int32_t pitch = g->pitch;
  uint8_t *src = g->buffer;

  if (pitch < 0) {
    src += (g->height - 1) * (-pitch);
    pitch = -pitch;
  }

  for (uint32_t j = 0; j < g->height; j++) {
    uint32_t target_y = y + j;

    if (target_y >= height)
      continue;

    uint8_t *src_row = src + j * pitch;
    uint8_t *dst_row = dst + target_y * stride;

    for (uint32_t i = 0; i < g->width; i++) {
      uint32_t target_x = x + i;

      if (target_x >= width)
        continue;

      uint8_t cov = src_row[i];
      if (!cov)
        continue;

      uint8_t *px = dst_row + target_x * 4;

      uint8_t src_a = (uint8_t)((color.a * cov) / 255);
      if (!src_a)
        continue;

      uint8_t src_r = (uint8_t)((color.r * src_a) / 255);
      uint8_t src_g = (uint8_t)((color.g * src_a) / 255);
      uint8_t src_b = (uint8_t)((color.b * src_a) / 255);

      uint8_t inv_a = 255 - src_a;

      px[0] = src_b + (uint8_t)((px[0] * inv_a) / 255);
      px[1] = src_g + (uint8_t)((px[1] * inv_a) / 255);
      px[2] = src_r + (uint8_t)((px[2] * inv_a) / 255);
      px[3] = src_a + (uint8_t)((px[3] * inv_a) / 255);
    }
  }
}

void we_draw_text(we_renderer *r, we_font *font, we_string_view *text, we_vec2i pos, we_color color) {
  FT_Face face = font->face;

  uint32_t width = r->width;
  uint32_t height = r->height;

  uint32_t pen_x = pos.x;
  uint32_t pen_y = pos.y + (face->size->metrics.ascender >> 6);

  hb_buffer_clear_contents(r->hb_buffer);
  hb_buffer_add_utf8(r->hb_buffer, text->data, text->len, 0, -1);
  hb_buffer_set_direction(r->hb_buffer, HB_DIRECTION_LTR);

  hb_shape(font->hb_font, r->hb_buffer, NULL, 0);

  uint32_t count;
  hb_glyph_info_t *infos = hb_buffer_get_glyph_infos(r->hb_buffer, &count);
  hb_glyph_position_t *pos_data = hb_buffer_get_glyph_positions(r->hb_buffer, &count);

  for (uint32_t i = 0; i < count; i++) {
    if (pen_x > width || pen_y > height)
      break;
    uint32_t glyph_id = infos[i].codepoint;

    glyph_cache_entry *g = glyph_cache_lookup(r->glyph_cache, glyph_id);

    if (!g) {
      FT_Load_Glyph(face, glyph_id, FT_LOAD_DEFAULT);
      FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

      g = glyph_cache_insert(r->glyph_cache, face->glyph, glyph_id);
    }

    uint32_t x = pen_x + (pos_data[i].x_offset >> 6) + g->left;
    uint32_t y = pen_y - (pos_data[i].y_offset >> 6) - g->top;

    draw_bitmap(r, g, x, y, color);

    pen_x += (pos_data[i].x_advance >> 6);
    pen_y += (pos_data[i].y_advance >> 6);
  }
}

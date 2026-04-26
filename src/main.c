#include "core.h"

typedef struct app_state {
  we_font *font;
} app_state;

void render(void *a, we_window *win, we_renderer *r) {
  assert(a != NULL);
  (void)win;

  app_state *state = a;

  we_clear(r, (we_color){.r = 0x14, .g = 0x14, .b = 0x14, .a = 0xff});
  we_fill_rect(r, (we_vec2i){{{0, 0}}}, (we_vec2i){{{100, 100}}}, (we_color){.d = 0x0000ff4f});
  we_draw_text(r, state->font, &(we_string_view){.data = "Hello, Window!", .len = 14}, (we_vec2i){.c = {100, 100}},
               (we_color){.d = 0xffffffff});
  we_fill_triangle(r, (we_vec2i){.c = {0, 0}}, (we_vec2i){.c = {100, 0}}, (we_vec2i){.c = {50, 100}}, (we_color){.d = 0xFF00004F});
  we_fill_circle(r, (we_vec2i){.c = {50, 50}}, 24, (we_color){.d = 0x00FF004F});
}

int main(void) {
  we_app app = {0};
  we_window win = {0};
  app_state state = {0};

  assert(we_app_init(&app) != false);
  assert(we_window_init(&win, 400, 200, 0, &app) != false);

  state.font = we_font_load("BlexMono Nerd Font Light", 32, true);

  we_window_set_render(&win, render, &state);

  we_app_run(&app);

  we_window_deinit(&win);
  we_app_deinit(&app);
}

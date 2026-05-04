#include "core.h"

#include <stdio.h>
#include <time.h>

#ifdef PROFILE

#define start_timer(name)                                                                                                                  \
  struct timespec name##_start, name##_end;                                                                                                \
  clock_gettime(CLOCK_MONOTONIC, &name##_start);

#define end_timer(name)                                                                                                                    \
  clock_gettime(CLOCK_MONOTONIC, &name##_end);                                                                                             \
  long sec = name##_end.tv_sec - name##_start.tv_sec;                                                                                      \
  long nsec = name##_end.tv_nsec - name##_start.tv_nsec;                                                                                   \
  double elapsed = sec + nsec * 1e-9;                                                                                                      \
  LOG_PROF("took %.6f ms", elapsed * 1000.0);

#else

#define start_timer(name) ((void)0)
#define end_timer(name) ((void)0)

#endif

static const unsigned char font_data[] = {
#embed "../assets/ttf/JetBrainsMono-Regular.ttf"
};

typedef struct app_state {
  we_font *font;
  uint32_t font_size;
} app_state;

void render(void *a, we_window *win, we_renderer *r) {
  start_timer(render);

  assert(a != NULL);
  (void)win;

  app_state *state = a;
  if (state->font == NULL) {
    // state->font = we_font_load("assets/ttf/JetBrainsMono-Regular.ttf", state->font_size);
    state->font = we_font_load_from_memory(font_data, sizeof(font_data), state->font_size);
    LOG_INFO("Font loaded");
  }

  we_clear(r, (we_color){.r = 0x14, .g = 0x14, .b = 0x14, .a = 0xff});
  we_fill_rect(r, (we_vec2i){{{0, 0}}}, (we_vec2i){{{100, 100}}}, (we_color){.d = 0x0000ff4f});
  we_fill_triangle(r, (we_vec2i){.c = {0, 0}}, (we_vec2i){.c = {100, 0}}, (we_vec2i){.c = {50, 100}}, (we_color){.d = 0xFF00004F});
  we_fill_circle(r, (we_vec2i){.c = {50, 50}}, 24, (we_color){.d = 0x00FF004F});

  // NOTE(matt): for testing using '-1' as len is fine because harfbuzz handles it, but the real len should be set in any other scenario
  we_string_view text = {.data = "é̲ΩЖ∑┼", .len = -1};
  we_vec2i pos = {.x = 100, .y = 0};
  for (size_t i = 0; i < 200; i++) {
    we_draw_text(r, state->font, &text, pos, (we_color){.d = 0xff00ffaf});
    pos.y += state->font_size;
  }

  end_timer(render);
}

int main(void) {
  we_app app = {0};
  we_window win = {0};
  app_state state = {0};
  state.font_size = 32;

  assert(we_app_init(&app) != false);
  assert(we_window_init(&win, 400, 800, 0, &app) != false);

  we_window_set_render(&win, render, &state);

  we_app_run(&app);

  we_window_deinit(&win);
  we_app_deinit(&app);
}

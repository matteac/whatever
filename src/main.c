#include "core.h"

void render(void *a, we_window *win) {
  uint32_t *pixels = win->shm_data;
  for (uint32_t x = 0; x <= win->width; x++) {
    for (uint32_t y = 0; y <= win->height; y++) {
      pixels[y * win->width + x] = 0xf0ff00ff;
    }
  }
}

int main() {
  we_app app = {0};
  we_window win = {0};

  assert(we_app_init(&app) != false);
  assert(we_window_init(&win, 400, 200, 0, &app) != false);

  we_window_set_render(&win, render, NULL);

  we_app_run(&app);

  we_window_deinit(&win);
  we_app_deinit(&app);
}

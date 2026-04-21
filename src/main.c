#include "core.h"

int main() {
  we_app app = {0};
  we_window win = {0};

  assert(we_app_init(&app) != false);
  assert(we_window_init(&win, 400, 200, 0, &app) != false);

  we_app_run(&app);

  we_window_deinit(&win);
  we_app_deinit(&app);
}

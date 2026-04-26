#pragma once
#include <stddef.h>

typedef struct we_string {
  char *data;
  size_t len;
  size_t cap;
} we_string;

typedef struct we_string_view {
  const char *data;
  size_t len;
} we_string_view;

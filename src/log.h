#pragma once
#include <stdio.h>

#define LOG(file, level, fmt, ...)                                             \
  do {                                                                         \
    fprintf(file, "[%-14s] %s:%d @%s: " fmt "\n", level, __FILE_NAME__,        \
            __LINE__, __func__, ##__VA_ARGS__);                                \
  } while (0)

#ifndef NO_INFO
#define LOG_INFO(fmt, ...)                                                     \
  LOG(stdout, "\x1b[36mINFO\x1b[0m", fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...) ((void)0)
#endif

#define LOG_WARN(fmt, ...)                                                     \
  LOG(stderr, "\x1b[33mWARN\x1b[0m", fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)                                                      \
  LOG(stderr, "\x1b[35mERROR\x1b[0m", fmt, ##__VA_ARGS__)

#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define LOG(level, ...) if (debug && (_log_level <= level)) { \
    printf("[%s] ", log_level_string[level]); \
    printf(__VA_ARGS__); \
  }

#define LOG_ENABLE_SET(enable) debug = enable
#define LOG_LEVEL_SET(level) _log_level = level

enum log_level {
  LOG_DEBUG = 0,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  LOG_MAX,
};

static const char * log_level_string[LOG_MAX] = {"DEBUG", "INFO", "WARNING", "ERROR"};
static bool debug;
static uint _log_level = LOG_INFO;

#ifdef __cplusplus
}
#endif

#endif

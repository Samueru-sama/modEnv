#ifndef MODENV_H
#define MODENV_H

#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdio.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define DEBUGF(...) do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

#ifndef BUILD_ID
#define BUILD_ID "unknown"
#endif

#define MODENV_BUILD_ID_ENV "MODENV_BUILD_ID"

#ifndef ANTI_REHOOK
#define ANTI_REHOOK 1
#endif

#ifndef NO_UNHOOK_DEPTH
#define UNHOOK_DEPTH_ENABLED 1
#else
#define UNHOOK_DEPTH_ENABLED 0
#endif

#ifndef NO_INJECT_DEPTH
#define INJECT_DEPTH_ENABLED 1
#else
#define INJECT_DEPTH_ENABLED 0
#endif

#endif // MODENV_H

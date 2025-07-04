#define _POSIX_C_SOURCE 200809L
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "envs.h"

const char* get_exe_name(void) {
  static char name[256] = {0};
  ssize_t len = readlink("/proc/self/exe", name, sizeof(name) - 1);
  if (len > 0) {
    name[len] = '\0';
    return name;
  }
  return "modEnv";
}

#ifndef BUILD_ID
#define BUILD_ID "unknown"
#endif
#define MODENV_BUILD_ID_ENV "MODENV_BUILD_ID"

static bool hook_enabled = true;

__attribute__((constructor)) void check_rehook_guard(void) {
  const char* current_id = getenv(MODENV_BUILD_ID_ENV);
  if (current_id && strcmp(current_id, BUILD_ID) == 0) {
#ifdef DEBUG
    fprintf(stderr, "%s: modEnv: rehook prevented (BUILD_ID=%s)\n", get_exe_name(), BUILD_ID);
#endif
    hook_enabled = false;
    return;
  }
  setenv(MODENV_BUILD_ID_ENV, BUILD_ID, 1);
#ifdef DEBUG
  fprintf(stderr, "%s: modEnv: hook initialized (BUILD_ID=%s)\n", get_exe_name(), BUILD_ID);
#endif
}


typedef int (*execv_func)(const char* path, char* const argv[]);
typedef int (*execvp_func)(const char* file, char* const argv[]);
typedef int (*execve_func)(const char* path,
                           char* const argv[],
                           char* const envp[]);

static execv_func original_execv = NULL;
static execvp_func original_execvp = NULL;
static execve_func original_execve = NULL;

void handle_conditional_envs(void) {
  for (ConditionalEnv* env = conditional_envs; env->env != NULL; env++) {
    if (env->condition()) {
      if (env->value == NULL) {
        unsetenv(env->env);
      } else {
        setenv(env->env, env->value, 1);
      }
    }
  }
}

int execv(const char* path, char* const argv[]) {
  if (!original_execv) {
    original_execv = (execv_func)dlsym(RTLD_NEXT, "execv");
  }

  if (!hook_enabled) return original_execv(path, argv);

#ifdef DEBUG
  fprintf(stderr, "Hook called: execv\n");
#endif

  handle_conditional_envs();
  return original_execv(path, argv);
}

int execvp(const char* file, char* const argv[]) {
  if (!original_execvp) {
    original_execvp = (execvp_func)dlsym(RTLD_NEXT, "execvp");
  }

  if (!hook_enabled) return original_execvp(file, argv);

#ifdef DEBUG
  fprintf(stderr, "Hook called: execvp\n");
#endif

  handle_conditional_envs();
  return original_execvp(file, argv);
}

int execve(const char* path, char* const argv[], char* const envp[]) {
  if (!original_execve) {
    original_execve = (execve_func)dlsym(RTLD_NEXT, "execve");
  }

  if (!hook_enabled) return original_execve(path, argv, envp);

#ifdef DEBUG
  fprintf(stderr, "Hook called: execve\n");
#endif

  handle_conditional_envs();
  return original_execve(path, argv, envp);
}

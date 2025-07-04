#define _POSIX_C_SOURCE 200809L
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "envs.h"

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

#ifdef DEBUG
  fprintf(stderr, "Hook called: execve\n");
#endif

  handle_conditional_envs();
  return original_execve(path, argv, envp);
}

#ifdef DEBUG
__attribute__((constructor)) void on_load(void) {
  fprintf(stderr, "hooks added\n");
}
#endif

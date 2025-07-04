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
  return (len > 0) ? (name[len] = '\0', name) : "modEnv";
}

#ifndef BUILD_ID
#define BUILD_ID "unknown"
#endif
#define MODENV_BUILD_ID_ENV "MODENV_BUILD_ID"

static bool hook_enabled = true;
static int hook_depth = 0;
static int current_depth = 0;

__attribute__((constructor)) void check_rehook_guard(void) {
  const char* current_id = getenv(MODENV_BUILD_ID_ENV);
  const char* depth_str = getenv("_UNHOOK_DEPTH");
  int allowed_depth = 0;
  if (depth_str) {
    int d = atoi(depth_str);
    if (d >= 0 && d <= 16) allowed_depth = d;
  }

  const char* hook_depth_str = getenv("_HOOK_DEPTH");
  if (hook_depth_str) hook_depth = atoi(hook_depth_str);
  else hook_depth = 0;

  char depth_var[64];
  snprintf(depth_var, sizeof(depth_var), "_MODENV_DEPTH_%s", BUILD_ID);

  const char* existing_depth = getenv(depth_var);
  if (existing_depth) current_depth = atoi(existing_depth);

  if (current_id && strcmp(current_id, BUILD_ID) == 0 && current_depth >= allowed_depth) {
#ifdef DEBUG
    fprintf(stderr, "%s: modEnv: rehook prevented (BUILD_ID=%s)\n", get_exe_name(), BUILD_ID);
#endif
    hook_enabled = false;
    return;
  }

  setenv(MODENV_BUILD_ID_ENV, BUILD_ID, 1);
  char new_depth_str[8];
  snprintf(new_depth_str, sizeof(new_depth_str), "%d", current_depth + 1);
  setenv(depth_var, new_depth_str, 1);

#ifdef DEBUG
  fprintf(stderr, "%s: modEnv: hook initialized (BUILD_ID=%s, DEPTH=%d/%d)\n", get_exe_name(), BUILD_ID, current_depth, allowed_depth);
#endif
}

typedef int (*execv_func)(const char*, char* const[]);
typedef int (*execvp_func)(const char*, char* const[]);
typedef int (*execve_func)(const char*, char* const[], char* const[]);

static execv_func original_execv;
static execvp_func original_execvp;
static execve_func original_execve;

void handle_conditional_envs(void) {
  for (ConditionalEnv* env = conditional_envs; env->env; env++) {
    if (env->condition()) {
      if (!env->value) unsetenv(env->env);
      else setenv(env->env, env->value, 1);
    }
  }
}

int execv(const char* path, char* const argv[]) {
  if (!original_execv) original_execv = (execv_func)dlsym(RTLD_NEXT, "execv");
  if (!hook_enabled) return original_execv(path, argv);
#ifdef DEBUG
  fprintf(stderr, "Hook called: execv\n");
#endif
  if (hook_depth == -1) handle_conditional_envs();
  return original_execv(path, argv);
}

int execvp(const char* file, char* const argv[]) {
  if (!original_execvp) original_execvp = (execvp_func)dlsym(RTLD_NEXT, "execvp");
  if (!hook_enabled) return original_execvp(file, argv);
#ifdef DEBUG
  fprintf(stderr, "Hook called: execvp\n");
#endif
  if (hook_depth == -1) handle_conditional_envs();
  return original_execvp(file, argv);
}

int execve(const char* path, char* const argv[], char* const envp[]) {
  if (!original_execve) original_execve = (execve_func)dlsym(RTLD_NEXT, "execve");
  if (!hook_enabled) return original_execve(path, argv, envp);
#ifdef DEBUG
  fprintf(stderr, "Hook called: execve\n");
#endif
  if (hook_depth == -1) {
    handle_conditional_envs();
    return original_execve(path, argv, envp);
  }
  char** new_envp = create_modified_envp(envp);
  int result = original_execve(path, argv, new_envp ? new_envp : envp);
  if (new_envp) free_envp(new_envp);
  return result;
}

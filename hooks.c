#define _POSIX_C_SOURCE 200809L
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "envs.h"

#ifndef DEBUG
#define DEBUG 0
#endif
#define DEBUGF(...) do { if (DEBUG != 0) fprintf(stderr, __VA_ARGS__); } while(0)

const char* get_exe_name(void) {
  static char name[256];
  ssize_t len = readlink("/proc/self/exe", name, sizeof(name) - 1);
  if (len <= 0) return "modEnv";
  name[len] = '\0';
  return name;
}

#ifndef BUILD_ID
#define BUILD_ID "unknown"
#endif
#define MODENV_BUILD_ID_ENV "MODENV_BUILD_ID"

#ifndef ANTI_REHOOK
#define ANTI_REHOOK 1
#endif

static bool hook_enabled = true;

#if !defined(NO_INJECT_DEPTH)
static int inject_depth = 0;
#endif

#if !defined(NO_UNHOOK_DEPTH)
static int current_depth = 0;
#endif

static void handle_conditional_envs(void) {
  for (ConditionalEnv* env = conditional_envs; env->env; env++) {
    if (env->condition()) {
      if (!env->value) unsetenv(env->env);
      else setenv(env->env, env->value, 1);
    }
  }
}

__attribute__((constructor)) void check_rehook_guard(void) {
#if ANTI_REHOOK
  const char* current_id = getenv(MODENV_BUILD_ID_ENV);

#if !defined(NO_UNHOOK_DEPTH)
  const char* depth_str = getenv("_UNHOOK_DEPTH");
  int allowed_depth = 0;
  if (depth_str) {
    int d = atoi(depth_str);
    if (d >= 0 && d <= 16) allowed_depth = d;
  }
#else
  int allowed_depth = 0;
#endif

#if !defined(NO_INJECT_DEPTH)
  const char* inject_depth_str = getenv("_DIRECT_INJECT_DEPTH");
  if (inject_depth_str) {
    inject_depth = atoi(inject_depth_str);
  } else {
    inject_depth = -2;
  }
#endif

#if !defined(NO_UNHOOK_DEPTH)
  char depth_var[64];
  snprintf(depth_var, sizeof(depth_var), "_MODENV_DEPTH_%s", BUILD_ID);
  const char* existing_depth = getenv(depth_var);
  current_depth = existing_depth ? atoi(existing_depth) : 0;

  if (current_id && strcmp(current_id, BUILD_ID) == 0 && current_depth >= allowed_depth) {
    DEBUGF("%s: modEnv: rehook prevented (BUILD_ID=%s)\n", get_exe_name(), BUILD_ID);
    hook_enabled = false;
    return;
  }

  setenv(MODENV_BUILD_ID_ENV, BUILD_ID, 1);
  char new_depth_str[8];
  snprintf(new_depth_str, sizeof(new_depth_str), "%d", current_depth + 1);
  setenv(depth_var, new_depth_str, 1);
#else
  if (current_id && strcmp(current_id, BUILD_ID) == 0) {
    DEBUGF("%s: modEnv: rehook prevented (BUILD_ID=%s) [no depth check]\n", get_exe_name(), BUILD_ID);
    hook_enabled = false;
    return;
  }
  setenv(MODENV_BUILD_ID_ENV, BUILD_ID, 1);
#endif

#if !defined(NO_INJECT_DEPTH)
  if (inject_depth != -2 && (inject_depth == -1 || current_depth >= inject_depth))
    handle_conditional_envs();
#else
  handle_conditional_envs();
#endif

  /* Prepare debug variables outside macro args */
  int dbg_current_depth = 0, dbg_allowed_depth = 0;
#if !defined(NO_UNHOOK_DEPTH)
  dbg_current_depth = current_depth;
  dbg_allowed_depth = allowed_depth;
#endif

  DEBUGF("%s: modEnv: hook initialized (BUILD_ID=%s, DEPTH=%d/%d)\n",
         get_exe_name(), BUILD_ID, dbg_current_depth, dbg_allowed_depth);

#else
  /* ANTI_REHOOK == 0: minimal setup */
  DEBUGF("%s: modEnv: hook initialized (BUILD_ID=%s, anti_rehook disabled)\n", get_exe_name(), BUILD_ID);
  handle_conditional_envs();
#endif
}

typedef int (*execv_func)(const char*, char* const[]);
typedef int (*execvp_func)(const char*, char* const[]);
typedef int (*execve_func)(const char*, char* const[], char* const[]);

static execv_func original_execv;
static execvp_func original_execvp;
static execve_func original_execve;

int execv(const char* path, char* const argv[]) {
  if (!original_execv) original_execv = (execv_func)dlsym(RTLD_NEXT, "execv");
  if (!hook_enabled) return original_execv(path, argv);
  DEBUGF("Hook called: execv\n");
  return original_execv(path, argv);
}

int execvp(const char* file, char* const argv[]) {
  if (!original_execvp) original_execvp = (execvp_func)dlsym(RTLD_NEXT, "execvp");
  if (!hook_enabled) return original_execvp(file, argv);
  DEBUGF("Hook called: execvp\n");
  return original_execvp(file, argv);
}

int execve(const char* path, char* const argv[], char* const envp[]) {
  if (!original_execve) original_execve = (execve_func)dlsym(RTLD_NEXT, "execve");
  if (!hook_enabled) return original_execve(path, argv, envp);
  DEBUGF("Hook called: execve\n");
  char** new_envp = create_modified_envp(envp);
  int result = original_execve(path, argv, new_envp ? new_envp : envp);
  if (new_envp) free_envp(new_envp);
  return result;
}

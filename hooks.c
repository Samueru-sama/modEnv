#include "modEnv.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "envs.h"

#if UNHOOK_DEPTH_ENABLED
static int current_depth = 0;
static int allowed_depth = 0;
#endif

#if INJECT_DEPTH_ENABLED
static int inject_depth = 0;
#endif

static bool hook_enabled = true;

const char* get_exe_name(void) {
  static char name[256];
  ssize_t len = readlink("/proc/self/exe", name, sizeof(name) - 1);
  if (len <= 0) return "modEnv";
  name[len] = '\0';
  return name;
}

static void handle_conditional_envs(void) {
  for (ConditionalEnv* env = conditional_envs; env->env; env++) {
    if (env->condition()) {
      if (env->value)
        setenv(env->env, env->value, 1);
      else
        unsetenv(env->env);
    }
  }
}

__attribute__((constructor)) void check_rehook_guard(void) {
#if ANTI_REHOOK
  const char* current_id = getenv(MODENV_BUILD_ID_ENV);

#if UNHOOK_DEPTH_ENABLED
  const char* depth_str = getenv("_UNHOOK_DEPTH");
  if (depth_str) {
    int d = atoi(depth_str);
    if (d >= 0 && d <= 16) allowed_depth = d;
  }

  char depth_var[64];
  snprintf(depth_var, sizeof(depth_var), "_MODENV_DEPTH_%s", BUILD_ID);
  const char* existing_depth = getenv(depth_var);
  current_depth = existing_depth ? atoi(existing_depth) : 0;

  if (current_id && strcmp(current_id, BUILD_ID) == 0 && current_depth >= allowed_depth) {
    DEBUGF("%s: modEnv: rehook prevented (BUILD_ID=%s)\n", get_exe_name(), BUILD_ID);
    hook_enabled = false;
    return;
  }
#else
  if (current_id && strcmp(current_id, BUILD_ID) == 0) {
    DEBUGF("%s: modEnv: rehook prevented (BUILD_ID=%s) [no depth check]\n", get_exe_name(), BUILD_ID);
    hook_enabled = false;
    return;
  }
#endif

#if INJECT_DEPTH_ENABLED
  const char* inject_str = getenv("_DIRECT_INJECT_DEPTH");
  inject_depth = inject_str ? atoi(inject_str) : -2;
  if (inject_depth == -1 || inject_depth == -2 || current_depth >= inject_depth)
    handle_conditional_envs();
#else
  handle_conditional_envs();
#endif

#if UNHOOK_DEPTH_ENABLED
  DEBUGF("%s: modEnv: hook initialized (BUILD_ID=%s, DEPTH=%d/%d)\n",
         get_exe_name(), BUILD_ID, current_depth, allowed_depth);
#else
  DEBUGF("%s: modEnv: hook initialized (BUILD_ID=%s, DEPTH=0/0)\n",
         get_exe_name(), BUILD_ID);
#endif

#else
  DEBUGF("%s: modEnv: hook initialized (BUILD_ID=%s, anti_rehook disabled)\n",
         get_exe_name(), BUILD_ID);
  handle_conditional_envs();
#endif
}

int get_current_depth(void) {
#if UNHOOK_DEPTH_ENABLED
  return current_depth;
#else
  return 0;
#endif
}

int get_allowed_depth(void) {
#if UNHOOK_DEPTH_ENABLED
  return allowed_depth;
#else
  return 0;
#endif
}

int get_inject_depth(void) {
#if INJECT_DEPTH_ENABLED
  return inject_depth;
#else
  return -2;
#endif
}

const char* get_build_id(void) {
  return BUILD_ID;
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
  int r = original_execve(path, argv, new_envp ? new_envp : envp);
  if (new_envp) free_envp(new_envp);
  return r;
}

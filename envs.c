#include "envs.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static size_t env_count(char* const envp[]) {
  size_t count = 0;
  while (envp[count]) count++;
  return count;
}

char** create_modified_envp(char* const envp[]) {
  size_t count = env_count(envp);
  char** new_env = malloc(sizeof(char*) * (count + 64));
  if (!new_env) return NULL;

  size_t i = 0;
  for (; envp[i]; ++i) new_env[i] = strdup(envp[i]);

  for (ConditionalEnv* e = conditional_envs; e->env; ++e) {
    if (!e->condition()) continue;
    size_t key_len = strlen(e->env);
    for (size_t j = 0; j < i; ++j) {
      if (strncmp(new_env[j], e->env, key_len) == 0 && new_env[j][key_len] == '=') {
        free(new_env[j]);
        new_env[j] = new_env[i - 1];
        i--;
        break;
      }
    }
    if (e->value) {
      size_t len = key_len + strlen(e->value) + 2;
      new_env[i] = malloc(len);
      snprintf(new_env[i++], len, "%s=%s", e->env, e->value);
    }
  }
  new_env[i] = NULL;
  return new_env;
}

void free_envp(char** envp) {
  for (size_t i = 0; envp[i]; ++i) free(envp[i]);
  free(envp);
}

bool is_appdir_set(void) { return getenv("APPDIR") != NULL; }
bool is_appimage_set(void) { return getenv("APPIMAGE") != NULL; }
static bool always(void) { return true; }

ConditionalEnv conditional_envs[] = {
  {"BABL_PATH", NULL, is_appdir_set},
  {"GBM_BACKENDS_PATH", NULL, is_appdir_set},
  {"GCONV_PATH", NULL, is_appdir_set},
  {"GDK_PIXBUF_MODULEDIR", NULL, is_appdir_set},
  {"GDK_PIXBUF_MODULE_FILE", NULL, is_appdir_set},
  {"GEGL_PATH", NULL, is_appdir_set},
  {"GIO_MODULE_DIR", NULL, is_appdir_set},
  {"GI_TYPELIB_PATH", NULL, is_appdir_set},
  {"GSETTINGS_SCHEMA_DIR", NULL, is_appdir_set},
  {"GST_PLUGIN_PATH", NULL, is_appdir_set},
  {"GST_PLUGIN_SCANNER", NULL, is_appdir_set},
  {"GST_PLUGIN_SYSTEM_PATH", NULL, is_appdir_set},
  {"GST_PLUGIN_SYSTEM_PATH_1_0", NULL, is_appdir_set},
  {"GTK_DATA_PREFIX", NULL, is_appdir_set},
  {"GTK_EXE_PREFIX", NULL, is_appdir_set},
  {"GTK_IM_MODULE_FILE", NULL, is_appdir_set},
  {"GTK_PATH", NULL, is_appdir_set},
  {"LIBDECOR_PLUGIN_DIR", NULL, is_appdir_set},
  {"LIBGL_DRIVERS_PATH", NULL, is_appdir_set},
  {"LIBVA_DRIVERS_PATH", NULL, is_appdir_set},
  {"PERLLIB", NULL, is_appdir_set},
  {"PIPEWIRE_MODULE_DIR", NULL, is_appdir_set},
  {"QT_PLUGIN_PATH", NULL, is_appdir_set},
  {"SPA_PLUGIN_DIR", NULL, is_appdir_set},
  {"TCL_LIBRARY", NULL, is_appdir_set},
  {"TK_LIBRARY", NULL, is_appdir_set},
  {"XTABLES_LIBDIR", NULL, is_appdir_set},
  {"APPDIR", NULL, always},
  {"APPIMAGE", NULL, always},
  {"_TEST", "HAPPY HAPPY HAPPY", always},
  {NULL, NULL, NULL}
};

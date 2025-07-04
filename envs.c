#include "envs.h"

#include <stdbool.h>
#include <stdlib.h>

bool is_appdir_set(void) {
  return getenv("APPDIR") != NULL;
}

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
    {"APPDIR", NULL, is_appdir_set},
    {NULL, NULL, NULL}
};

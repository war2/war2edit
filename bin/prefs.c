/*
 * prefs.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

static Elm_Prefs_Data *_prefs[__PREFS_LAST];

static inline Elm_Prefs_Data *
_prefs_new(const char *suffix)
{
   char dirname[64];
   char path[128];

   snprintf(dirname, sizeof(dirname), "%s/.war2edit", getenv("HOME"));
   snprintf(path, sizeof(path), "%s/config_%s", dirname, suffix);
   ecore_file_mkdir(dirname);

   return elm_prefs_data_new(path, NULL, EET_FILE_MODE_READ_WRITE);
}

static inline const char *
_prefs_file_get(Prefs type)
{
   switch (type)
     {
      case PREFS_DOSBOX:
         return "dosbox.epb";

      default:
         return NULL;
     }
}

Eina_Bool
prefs_init(void)
{
   _prefs[PREFS_DOSBOX] = _prefs_new("dosbox");
   return EINA_TRUE;
}

void
prefs_shutdown(void)
{
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(_prefs); ++i)
     elm_prefs_data_unref(_prefs[i]);
}

const Elm_Prefs_Data *
prefs_get(Prefs type)
{
   return _prefs[type];
}

Evas_Object *
prefs_new(Evas_Object *parent,
          Prefs        type)
{
   Evas_Object *obj;
   char path[PATH_MAX];

   obj = elm_prefs_add(parent);
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_prefs_autosave_set(obj, EINA_TRUE);

   snprintf(path, sizeof(path), "%s/prefs/%s",
            elm_app_data_dir_get(), _prefs_file_get(type));
   elm_prefs_file_set(obj, path, NULL);
   elm_prefs_data_set(obj, _prefs[type]);
   evas_object_show(obj);

   return obj;
}

const char *
prefs_value_string_get(Prefs       type,
                       const char *key)
{
   Eina_Value value;
   const char *val;

   elm_prefs_data_value_get(prefs_get(type), key, NULL, &value);
   eina_value_get(&value, &val);
   return val;
}


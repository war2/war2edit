/*
 * prefs.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef __PREFS_H__
#define __PREFS_H__

typedef enum
{
   PREFS_DOSBOX = 0,

   __PREFS_LAST /* sentinel */
} Prefs;

Eina_Bool prefs_init(void);
void prefs_shutdown(void);
Evas_Object *prefs_new(Evas_Object *parent, Prefs type);
const Elm_Prefs_Data *prefs_get(Prefs type);
char *prefs_value_string_get(Prefs type, const char *key);

#endif /* ! __PREFS_H__ */


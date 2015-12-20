/*
 * inwin.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef __INWIN_H__
#define __INWIN_H__

typedef enum
{
   INWIN_GENERIC        = 0,
   INWIN_MAINCONFIG,
   INWIN_MAP_PROPERTIES,
   INWIN_PLAYER_PROPERTIES,
   INWIN_EDITOR_ERROR,
   INWIN_FILE_SELECTOR
} Inwin;

Evas_Object *inwin_add(Editor *ed);
void
inwin_set(Editor        *ed,
          Evas_Object   *obj,
          Inwin          id,
          const char    *ok_label,
          Evas_Smart_Cb  ok_smart_cb,
          const char    *cancel_label,
          Evas_Smart_Cb  cancel_smart_cb);
void inwin_dismiss(Editor *ed);
Eina_Bool inwin_id_is(const Editor *ed, const Inwin id);
void inwin_activate(Editor *ed);

#endif /* ! __INWIN_H__ */


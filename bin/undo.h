/*
 * undo.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef __UNDO_H__
#define __UNDO_H__

Eina_Bool undo_add(Editor *ed);
void undo_del(Editor *ed);
void undo_menu_connect(Editor *ed, Elm_Object_Item *undo, Elm_Object_Item *redo);
void undo_push(Editor *ed, const char *message);

#endif /* ! __UNDO_H__ */


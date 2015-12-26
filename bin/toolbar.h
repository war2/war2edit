/*
 * toolbar.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

Eina_Bool toolbar_add(Editor *ed, Evas_Object *box);
void toolbar_actions_segment_unselect(const Editor *ed);
void toolbar_runner_segment_selected_set(Editor *ed, Eina_Bool select);

#endif /* ! _TOOLBAR_H_ */


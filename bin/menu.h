/*
 * menu.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _MENU_H_
#define _MENU_H_

Eina_Bool menu_init(void);
void menu_shutdown(void);

Eina_Bool menu_add(Editor *ed);
void menu_enabled_set(Editor *ed, Eina_Bool set);
void menu_unit_selection_reset(Editor *ed);
void menu_units_side_enable(Editor *ed, Pud_Side enable);

Evas_Object *menu_map_properties_new(Editor *ed, Evas_Object *parent);
Evas_Object *menu_player_properties_new(Editor *ed, Evas_Object *parent);
Evas_Object *menu_starting_properties_new(Editor *ed, Evas_Object *parent);
Evas_Object *menu_units_properties_new(Editor *ed, Evas_Object *parent);

#endif /* ! _MENU_H_ */


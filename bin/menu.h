/*
 * Copyright (c) 2015-2016 Jean Guyomarc'h
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
Evas_Object *menu_dosbox_prefs_new(Editor *ed, Evas_Object *parent);

Eina_Bool menu_units_add(Editor *ed);
Eina_Bool menu_players_add(Editor *ed);

typedef struct
{
   Evas_Object *gen;
   Evas_Object *sight;
   Evas_Object *hp;
   Evas_Object *gold;
   Evas_Object *lumber;
   Evas_Object *oil;
   Evas_Object *range;
   Evas_Object *armor;
   Evas_Object *basic_damage;
   Evas_Object *piercing_damage;
   Evas_Object *has_magic;
   Evas_Object *weapons_upgradable;
   Evas_Object *armor_upgradable;
   Evas_Object *missile;

   Pud_Unit selected;
} Menu_Units;

#endif /* ! _MENU_H_ */

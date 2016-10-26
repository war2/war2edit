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

#ifndef _EDITOR_H_
#define _EDITOR_H_

typedef uint16_t Editor_Sel;

#define EDITOR_SEL_NONE                      ((Editor_Sel) 0)

   /* xxxxxxxxxxxxxxSS */
#define EDITOR_SEL_SPREAD_NORMAL             ((Editor_Sel) (0x01 << 0))
#define EDITOR_SEL_SPREAD_CIRCLE             ((Editor_Sel) (0x02 << 0))
#define EDITOR_SEL_SPREAD_SPECIAL            ((Editor_Sel) (0x03 << 0))
#define EDITOR_SEL_SPREAD_MASK               ((Editor_Sel) (0x03 << 0))

   /* xxxxxxxxxxxxRRxx */
#define EDITOR_SEL_RADIUS_SMALL              ((Editor_Sel) (0x01 << 2))
#define EDITOR_SEL_RADIUS_MEDIUM             ((Editor_Sel) (0x02 << 2))
#define EDITOR_SEL_RADIUS_BIG                ((Editor_Sel) (0x03 << 2))
#define EDITOR_SEL_RADIUS_MASK               ((Editor_Sel) (0x03 << 2))

   /* xxxxxxxxxxTTxxxx */
#define EDITOR_SEL_TINT_LIGHT                ((Editor_Sel) (0x01 << 4))
#define EDITOR_SEL_TINT_DARK                 ((Editor_Sel) (0x02 << 4))
#define EDITOR_SEL_TINT_MASK                 ((Editor_Sel) (0x03 << 4))

   /* xxxxxxAAAAxxxxxx */
#define EDITOR_SEL_ACTION_NONE               ((Editor_Sel) (0x01 << 6))
#define EDITOR_SEL_ACTION_SELECTION          ((Editor_Sel) (0x02 << 6))
#define EDITOR_SEL_ACTION_WATER              ((Editor_Sel) (0x03 << 6))
#define EDITOR_SEL_ACTION_GROUND             ((Editor_Sel) (0x04 << 6))
#define EDITOR_SEL_ACTION_GRASS              ((Editor_Sel) (0x05 << 6))
#define EDITOR_SEL_ACTION_TREES              ((Editor_Sel) (0x06 << 6))
#define EDITOR_SEL_ACTION_ROCKS              ((Editor_Sel) (0x07 << 6))
#define EDITOR_SEL_ACTION_HUMAN_WALLS        ((Editor_Sel) (0x08 << 6))
#define EDITOR_SEL_ACTION_ORC_WALLS          ((Editor_Sel) (0x09 << 6))
#define EDITOR_SEL_ACTION_MASK               ((Editor_Sel) (0x0f << 6))


enum
{
   EDITOR_DEBUG_NONE            = 0,
   EDITOR_DEBUG_CELLS_COORDS    = (1 << 0),
};

struct _Editor
{

   /* === GUI === */
   Evas_Object  *win;
   Evas_Object  *lay;
   Evas_Object  *edje;
   Evas_Object  *inwin;
   Evas_Object  *mainbox;
   Evas_Object  *scroller;
   Evas_Object  *fs; /* File selector */
   Evas_Object  *menu_swamp_radio;
   Evas_Object  *menu_map_radio_group;
   Evas_Object  *rpanel;
   Evas_Object  *units_genlist;
   Evas_Object  *tileselector;
   Evas_Object  *unitsmenu;
   Evas_Object  *unitsmenu_btn;
   Evas_Object  *playersmenu;
   Evas_Object  *propertiesmenu;
   Evas_Object  *playersmenu_btn;

   Eina_Stringshare *filename;
   Cell        **cells;

   struct {
      unsigned char *pixels;
      Evas_Object *obj;
   } preview;

   /* === Toolbar === */
   Evas_Object  *segs[4];
   Editor_Sel    tb_sel;
   Pud_Unit      sel_unit;
   Pud_Player    sel_player;

   Bitmap  bitmap;
   struct {
      Eina_Bool menu_in;
      Eina_Bool visible;
      Eina_Bool enabled;
   } menu_in_out_cache;

   struct {
      unsigned char **data;
      Evas_Object    *map;
      Evas_Object    *rect;
      unsigned int    ratio;
   } minimap;

   struct {
      Evas_Object *sel[3];
      unsigned int x, y;
   } unitselector;

   struct {
      Evas_Object *obj;
      int          x;
      int          y;
      Eina_Bool    active;
      Eina_Bool    inclusive;
      unsigned int selections;
      struct {
         unsigned int x;
         unsigned int y;
      } rel1, rel2;
   } sel;

   Evas_Object *radio_units_reset;
   Evas_Object *pop;

   Eina_Stringshare *save_file_tmp;

   Eina_Array  *orc_menus;
   Eina_Array  *human_menus;

   struct {
      Elm_Object_Item *menu_undo;
      Eina_Inlist *items;
      uint8_t *buffer;
      size_t buf_len;
      int requests;
   } snapshot;

   Elm_Object_Item *gen_group_players[8];
   Elm_Object_Item *gen_group_neutral;

   Menu_Units *menu_units;
   Menu_Upgrades *menu_upgrades;
   Menu_Allows *menu_allows;
   Eina_Bool extension;

   /* === PUD specific === */
   Pud        *pud;
   uint8_t     sides[8]; /* Orc, Human */
   Evas_Point  start_locations[8];

   unsigned int    debug;
   double zoom;

   int mainconfig;
   /* Used to avoid setting tiles in the same cell every time
    * the mouse is moved within the cell */
   int prev_x;
   int prev_y;
   Eina_Bool was_oob;
};


static inline void
editor_sel_spread_set(Editor * ed,
                      Editor_Sel       sel)
{
   ed->tb_sel &= (Editor_Sel)(~EDITOR_SEL_SPREAD_MASK);
   ed->tb_sel |= sel;
}

static inline void
editor_sel_tint_set(Editor * ed,
                    Editor_Sel       sel)
{
   ed->tb_sel &= (Editor_Sel)(~EDITOR_SEL_TINT_MASK);
   ed->tb_sel |= sel;
}

static inline void
editor_sel_radius_set(Editor * ed,
                      Editor_Sel       sel)
{
   ed->tb_sel &= (Editor_Sel)(~EDITOR_SEL_RADIUS_MASK);
   ed->tb_sel |= sel;
}

static inline void
editor_sel_action_set(Editor * ed,
                      Editor_Sel       sel)
{
   ed->tb_sel &= (Editor_Sel)(~EDITOR_SEL_ACTION_MASK);
   ed->tb_sel |= sel;
}

static inline Editor_Sel
editor_sel_spread_get(const Editor * ed)
{
   return ed->tb_sel & EDITOR_SEL_SPREAD_MASK;
}

static inline Editor_Sel
editor_sel_tint_get(const Editor * ed)
{
   return ed->tb_sel & EDITOR_SEL_TINT_MASK;
}

static inline Editor_Sel
editor_sel_radius_get(const Editor * ed)
{
   return ed->tb_sel & EDITOR_SEL_RADIUS_MASK;
}

static inline Editor_Sel
editor_sel_action_get(const Editor * ed)
{
   return ed->tb_sel & EDITOR_SEL_ACTION_MASK;
}

Eina_Bool editor_init(void);
void editor_shutdown(void);

void editor_free(Editor *ed);
Editor *editor_new(const char *pud_file, unsigned int debug);
Eina_Bool editor_load(Editor *ed, const char *file);
Eina_Bool editor_save(Editor *ed, const char *file);
void editor_error(Editor *ed, const char *fmt, ...) EINA_PRINTF(2,3);
void editor_name_set(Editor *ed, const char *name);
Eina_Bool editor_unit_ref(Editor *ed, unsigned int x, unsigned int y, Unit type);
Eina_Bool editor_unit_unref(Editor *ed, unsigned int x, unsigned int y, Unit type);
uint16_t
editor_alter_defaults_get(const Editor *ed,
                          const Pud_Unit          unit);
Eina_Bool editor_sync(Editor *ed);

void editor_notif_send(Editor *ed, const char *msg, ...) EINA_PRINTF(2,3);

unsigned int editors_count(void);
void
editor_tb_sel_set(Editor *ed,
                  Editor_Sel       sel);

void editor_units_recount(Editor *ed);
void editor_handle_delete(Editor *ed);

Editor *editor_focused_get(void);
Evas_Object *
editor_icon_image_new(Evas_Object *parent,
                      Pud_Icon     icon,
                      Pud_Era      era,
                      Pud_Player   color);

Eina_Bool editor_units_list_update(Editor *ed);

Eina_Bool
editor_player_switch_race(Editor     *ed,
                          Pud_Player  player);


Evas_Object *
editor_file_selector_add(Editor    *ed,
                         Eina_Bool  save);

Evas_Object *editor_inwin_add(Editor *ed);
void
editor_inwin_set(Editor        *ed,
                 Evas_Object   *obj,
                 const char    *style,
                 const char    *ok_label,
                 Evas_Smart_Cb  ok_smart_cb,
                 const char    *cancel_label,
                 Evas_Smart_Cb  cancel_smart_cb);
void editor_inwin_dismiss(Editor *ed);

void editor_partial_load(Editor *ed);

Eina_Bool editor_tiles_sync(Editor *ed);
#define EDITOR_ERROR(ed_, msg_, ...) \
   do { \
      CRI(msg_, ## __VA_ARGS__); \
      editor_error(ed_, msg_, ## __VA_ARGS__); \
   } while (0)

#endif /* ! _EDITOR_H_ */

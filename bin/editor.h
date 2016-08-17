/*
 * editor.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "inwin.h"

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
   Evas_Object  *menu;
   struct {
      Evas_Object  *obj;
      Inwin         id;
   } inwin;
   Evas_Object  *mainbox;
   Evas_Object  *scroller;
   Evas_Object  *fs; /* File selector */
   Evas_Object  *menu_swamp_radio;
   Evas_Object  *menu_map_radio_group;
   Evas_Object  *rpanel;
   Evas_Object  *units_genlist;

   Cell        **cells;

   /* === Toolbar === */
   Evas_Object  *actions;
   Editor_Sel    tb_sel;
   Pud_Unit      sel_unit;
   Pud_Player    sel_player;
   Evas_Object  *runner;
   char *edje_file;

   Bitmap  bitmap;

   // XXX Should minimap be shared for all instances??
   struct {
      Evas_Object    *win;
      Evas_Object    *map;
      Evas_Object    *rect;
      unsigned char **data;
      unsigned int    w;
      unsigned int    h;
      unsigned int    ratio;
   } minimap;

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

   Eina_Array  *orc_menus;
   Eina_Array  *human_menus;

   Eina_Array *undo;

   Elm_Object_Item *gen_group_players[8];
   Elm_Object_Item *gen_group_neutral;
   Eina_Inlist *player_units[8];
   Eina_Inlist *neutral_units;

   /* === PUD specific === */
   Pud        *pud;
   uint8_t     sides[8]; /* Orc, Human */
   Evas_Point  start_locations[8];

   unsigned int    debug;
   double zoom;

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
void editor_view_update(Editor *ed);

void
editor_tb_sel_set(Editor *ed,
                  Editor_Sel       sel);

void editor_handle_delete(Editor *ed);

Editor *editor_focused_get(void);

#define EDITOR_ERROR(ed_, msg_, ...) \
   do { \
      CRI(msg_, ## __VA_ARGS__); \
      editor_error(ed_, msg_, ## __VA_ARGS__); \
   } while (0)

#endif /* ! _EDITOR_H_ */

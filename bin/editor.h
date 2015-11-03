/*
 * editor.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _EDITOR_H_
#define _EDITOR_H_

typedef enum /* 16 bits are required for this enum */
{
   EDITOR_SEL_NONE                      = 0,

   /* xxxxxxxxxxxxxxSS */
   EDITOR_SEL_SPREAD_NORMAL             = (0x00 << 0),
   EDITOR_SEL_SPREAD_CIRCLE             = (0x01 << 0),
   EDITOR_SEL_SPREAD_RANDOM             = (0x02 << 0),
#define EDITOR_SEL_SPREAD_MASK (0x03 << 0)

   /* xxxxxxxxxxxxRRxx */
   EDITOR_SEL_RADIUS_SMALL              = (0x00 << 2),
   EDITOR_SEL_RADIUS_MEDIUM             = (0x01 << 2),
   EDITOR_SEL_RADIUS_BIG                = (0x02 << 2),
#define EDITOR_SEL_RADIUS_MASK (0x03 << 2)

   /* xxxxxxxxxxxTxxxx */
   EDITOR_SEL_TINT_LIGHT                = (0x00 << 4),
   EDITOR_SEL_TINT_DARK                 = (0x01 << 4),
#define EDITOR_SEL_TINT_MASK (0x01 << 4)

   /* xxxxxxxAAAAxxxxx */
   EDITOR_SEL_ACTION_NONE               = (0x01 << 5),
   EDITOR_SEL_ACTION_SELECTION          = (0x02 << 5),
   EDITOR_SEL_ACTION_WATER              = (0x03 << 5),
   EDITOR_SEL_ACTION_NON_CONSTRUCTIBLE  = (0x04 << 5),
   EDITOR_SEL_ACTION_CONSTRUCTIBLE      = (0x05 << 5),
   EDITOR_SEL_ACTION_TREES              = (0x06 << 5),
   EDITOR_SEL_ACTION_ROCKS              = (0x07 << 5),
   EDITOR_SEL_ACTION_HUMAN_WALLS        = (0x08 << 5),
   EDITOR_SEL_ACTION_ORCS_WALLS         = (0x09 << 5)
#define EDITOR_SEL_ACTION_MASK (0x0f << 5)

} Editor_Sel;



struct _Editor
{

   /* === GUI === */
   Evas_Object  *win;
   Evas_Object  *menu;
   Evas_Object  *inwin;
   Evas_Object  *mainbox;
   Evas_Object  *scroller;
   Evas_Object  *fs; /* File selector */
   Evas_Object  *bitmap;

   Cell        **cells;

   /* === Toolbar === */
   Editor_Sel    tb_sel;
   Pud_Unit      sel_unit;
   Pud_Player    sel_player;
   struct {
      Evas_Object *tint[2];
      Evas_Object *spread[3];
      Evas_Object *radius[3];
      Evas_Object *action[8];
   } tb;

   struct {
      Evas_Object    *win;
      Evas_Object    *map;
      Evas_Object    *rect;
      unsigned char **data;
      unsigned int    ratio;
      unsigned int    w;
      unsigned int    h;
   } minimap;

   struct {
      Evas_Object *obj;
      int          x;
      int          y;
      Eina_Bool    active;
   } sel;

   /* === Mainconfig === */
   // FIXME Dynamic? Overhead when starting the config, but
   // less memory used throughout the whole program....
   struct _mainconfig {
      Evas_Object *container;
      Evas_Object *img;
      Evas_Object *menu_size;
      Evas_Object *menu_era;
   } mainconfig;

   Evas_Object *radio_units_reset;

   /* === PUD specific === */
   Pud        *pud;
   uint8_t     sides[8]; /* Orc, Human */
   Evas_Point  start_locations[8];

};


static inline void
editor_sel_spread_set(Editor *restrict ed,
                      Editor_Sel       sel)
{
   ed->tb_sel &= ~EDITOR_SEL_SPREAD_MASK;
   ed->tb_sel |= sel;
}

static inline void
editor_sel_tint_set(Editor *restrict ed,
                    Editor_Sel       sel)
{
   ed->tb_sel &= ~EDITOR_SEL_TINT_MASK;
   ed->tb_sel |= sel;
}

static inline void
editor_sel_radius_set(Editor *restrict ed,
                      Editor_Sel       sel)
{
   ed->tb_sel &= ~EDITOR_SEL_RADIUS_MASK;
   ed->tb_sel |= sel;
}

static inline void
editor_sel_action_set(Editor *restrict ed,
                      Editor_Sel       sel)
{
   ed->tb_sel &= ~EDITOR_SEL_ACTION_MASK;
   ed->tb_sel |= sel;
}

static inline Editor_Sel
editor_sel_spread_get(const Editor *restrict ed)
{
   return ed->tb_sel & EDITOR_SEL_SPREAD_MASK;
}

static inline Editor_Sel
editor_sel_tint_get(const Editor *restrict ed)
{
   return ed->tb_sel & EDITOR_SEL_TINT_MASK;
}

static inline Editor_Sel
editor_sel_radius_get(const Editor *restrict ed)
{
   return ed->tb_sel & EDITOR_SEL_RADIUS_MASK;
}

static inline Editor_Sel
editor_sel_action_get(const Editor *restrict ed)
{
   return ed->tb_sel & EDITOR_SEL_ACTION_MASK;
}

Eina_Bool editor_init(void);
void editor_shutdown(void);

void editor_free(Editor *ed);
Editor *editor_new(const char *pud_file);
Eina_Bool editor_load(Editor * restrict ed, const char *file);
Eina_Bool editor_save(Editor * restrict ed, const char *file);
void editor_error(Editor *ed, const char *msg);
unsigned char *editor_texture_tile_access(const Editor * restrict ed, unsigned int x, unsigned int y);
void editor_name_set(Editor * restrict ed, const char *name);
Eina_Bool editor_unit_ref(Editor * restrict ed);
Eina_Bool editor_unit_unref(Editor * restrict ed);
uint16_t
editor_alter_defaults_get(const Editor * restrict ed,
                          const Pud_Unit          unit);
Eina_Bool editor_sync(Editor * restrict ed);
void editor_view_update(Editor *restrict ed);

void
editor_tb_sel_set(Editor *restrict ed,
                  Editor_Sel       sel);

#define EDITOR_ERROR_RET(ed_, msg_, ...) \
   do { \
      CRI(msg_); \
      editor_error(ed_, msg_); \
      return __VA_ARGS__; \
   } while (0)

#define EDITOR_ERROR_GOTO(ed_, msg_, label_) \
   do { \
      CRI(msg_); \
      editor_error(ed_, msg_); \
      goto label_; \
   } while (0)

#endif /* ! _EDITOR_H_ */


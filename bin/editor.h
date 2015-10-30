/*
 * editor.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _EDITOR_H_
#define _EDITOR_H_

typedef enum
{
   EDITOR_SPREAD_NORMAL,
   EDITOR_SPREAD_CIRCLE,
   EDITOR_SPREAD_RANDOM
} Editor_Spread;

typedef enum
{
   EDITOR_RADIUS_SMALL,
   EDITOR_RADIUS_MEDIUM,
   EDITOR_RADIUS_BIG
} Editor_Radius;

typedef enum
{
   EDITOR_ACTION_NONE,
   EDITOR_ACTION_SELECTION,
   EDITOR_ACTION_WATER,
   EDITOR_ACTION_NON_CONSTRUCTIBLE,
   EDITOR_ACTION_CONSTRUCTIBLE,
   EDITOR_ACTION_TREES,
   EDITOR_ACTION_ROCKS,
   EDITOR_ACTION_HUMAN_WALLS,
   EDITOR_ACTION_ORCS_WALLS
} Editor_Action;

typedef enum
{
   EDITOR_TINT_LIGHT,
   EDITOR_TINT_DARK
} Editor_Tint;


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
   Editor_Action action;
   Editor_Spread spread;
   Editor_Radius radius;
   Editor_Tint   tint;
   Pud_Unit      sel_unit;
   Pud_Player    sel_player;

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
   // FIXME tooooooooo many duplicates!!!!!!
   Pud             *pud;
   uint8_t          sides[8]; /* Orc, Human */
   Evas_Point start_locations[8];

};

Eina_Bool editor_init(void);
void editor_shutdown(void);

void editor_free(Editor *ed);
Editor *editor_new(const char *pud_file);
Eina_Bool editor_load(Editor * restrict ed, const char *file);
Eina_Bool editor_save(Editor * restrict ed, const char *file);
void editor_error(Editor *ed, const char *msg);
unsigned char *editor_texture_tile_access(const Editor * restrict ed, unsigned int x, unsigned int y);
void editor_name_set(Editor * restrict ed, const char *name);

uint16_t
editor_alter_defaults_get(const Editor * restrict ed,
                          const Pud_Unit          unit);

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


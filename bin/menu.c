/*
 * menu.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

static Eina_Hash *_values = NULL;
#define PUD_DATA "war2edit/pud"

typedef uint32_t (*Prescalor_Cb)(uint32_t val);

typedef struct
{
   Prescalor_Cb operation;
   Prescalor_Cb inverse;
} Prescalor;

typedef enum
{
   POINTER_TYPE_BYTE,
   POINTER_TYPE_WORD,
   POINTER_TYPE_LONG
} Pointer_Type;

typedef struct
{
   union {
      uint8_t      *byte_ptr;
      uint16_t     *word_ptr;
      uint32_t     *long_ptr;
   } ptr;

   Pointer_Type type;

} Pointer;

typedef struct
{
   uint32_t     start;
   uint32_t     end;
} Range;

typedef struct
{
   Elm_Validator_Regexp *re;
   Pointer               bind;
   Range                 range;
   Prescalor             prescalor;
} Validator;


static void _validator_init(Validator *val);

static Validator *
_validator_new(void)
{
   Validator *val;

   val = calloc(1, sizeof(*val));
   if (EINA_UNLIKELY(!val))
     {
        CRI("Failed to allocate memory");
        return NULL;
     }

   _validator_init(val);

   return val;
}

static void
_validator_init(Validator *val)
{
   /* Digits only, and at least one */
   val->re = elm_validator_regexp_new("^[0-9]+$", NULL);
}

static void EINA_UNUSED
_validator_free(Validator *val)
{
   elm_validator_regexp_free(val->re);
   free(val);
}


/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/

static Evas_Object *
_radio_add(Editor          *ed,
           Evas_Object     *group,
           unsigned int     object,
           Elm_Object_Item *parent,
           const char      *label,
           Evas_Smart_Cb    func,
           Eina_Array      *storage)
{
   Evas_Object *o;
   Elm_Object_Item *eoi;

   o = elm_radio_add(ed->menu);
   eo_do(o, elm_obj_radio_state_value_set(object));
   if (group != NULL)
     eo_do(o, elm_obj_radio_group_add(group));

   if (label) elm_object_text_set(o, label);
   if (parent)
     {
        eoi = elm_menu_item_add(ed->menu, parent, NULL, NULL, func, o);
        elm_object_item_content_set(eoi, o);
        if (storage)
          eina_array_push(storage, eoi);

     }
   return o;
}

static void
_radio_changed_common_do(Evas_Object *radio,
                         int         *bind)
{
   int val;

   val = elm_radio_state_value_get(radio);
   elm_radio_value_set(radio, val);
   *bind = val;
}


/*============================================================================*
 *                                  Callbacks                                 *
 *============================================================================*/

static void
_win_del_cb(void        *data,
            Evas_Object *obj   EINA_UNUSED,
            void        *event EINA_UNUSED)
{
   Editor *ed = data;
   editor_free(ed);
}

static void
_win_new_cb(void        *data  EINA_UNUSED,
            Evas_Object *obj   EINA_UNUSED,
            void        *event EINA_UNUSED)
{
}

static void
_win_open_cb(void        *data,
             Evas_Object *obj   EINA_UNUSED,
             void        *event EINA_UNUSED)
{
   Editor *ed = data;
   file_load_prompt(ed);
}

static void
_win_save_cb(void        *data,
             Evas_Object *obj   EINA_UNUSED,
             void        *event EINA_UNUSED)
{
   Editor *ed = data;
   if (!ed->pud->filename)
     file_save_prompt(ed);
   else
     editor_save(ed, ed->pud->filename);
}

static void
_win_save_as_cb(void        *data,
                Evas_Object *obj   EINA_UNUSED,
                void        *event EINA_UNUSED)
{
   Editor *ed = data;
   file_save_prompt(ed);
}

static void
_map_properties_cb(void        *data,
                   Evas_Object *obj   EINA_UNUSED,
                   void        *event EINA_UNUSED)
{
   Editor *ed = data;

   if (inwin_id_is(ed, INWIN_MAP_PROPERTIES))
     inwin_activate(ed);
   else
     {
        inwin_set(ed, menu_map_properties_new(ed, ed->inwin.obj),
                  INWIN_MAP_PROPERTIES,
                  "Close", NULL, NULL, NULL);
     }
}

static void
_player_properties_cb(void        *data,
                      Evas_Object *obj   EINA_UNUSED,
                      void        *event EINA_UNUSED)
{
   Editor *ed = data;

   if (inwin_id_is(ed, INWIN_PLAYER_PROPERTIES))
     inwin_activate(ed);
   else
     {
        inwin_set(ed, menu_player_properties_new(ed, ed->inwin.obj),
                  INWIN_PLAYER_PROPERTIES,
                  "Close", NULL, NULL, NULL);
     }
}

static void
_starting_properties_cb(void        *data,
                        Evas_Object *obj   EINA_UNUSED,
                        void        *event EINA_UNUSED)
{
   Editor *ed = data;

   if (inwin_id_is(ed, INWIN_STARTING_PROPERTIES))
     inwin_activate(ed);
   else
     {
        inwin_set(ed, menu_starting_properties_new(ed, ed->inwin.obj),
                  INWIN_STARTING_PROPERTIES,
                  "Close", NULL, NULL, NULL);
     }
}

static void
_units_properties_cb(void        *data  EINA_UNUSED,
                     Evas_Object *obj   EINA_UNUSED,
                     void        *event EINA_UNUSED)
{
// TODO Uncomment when perf issue will be resolved
//   Editor *ed = data;
//
//   if (inwin_id_is(ed, INWIN_UNITS_PROPERTIES))
//     inwin_activate(ed);
//   else
//     {
//        inwin_set(ed, menu_units_properties_new(ed, ed->inwin.obj),
//                  INWIN_UNITS_PROPERTIES,
//                  "Close", NULL, NULL, NULL);
//     }
}

static void
_upgrades_properties_cb(void        *data  EINA_UNUSED,
                        Evas_Object *obj   EINA_UNUSED,
                        void        *event EINA_UNUSED)
{
}

static void
_radio_units_changed_cb(void        *data,
                        Evas_Object *obj,
                        void        *event EINA_UNUSED)
{
   unsigned int w, h;
   Editor *ed = evas_object_data_get(obj, "editor");
   _radio_changed_common_do(data, (int *)(&(ed->sel_unit)));
   DBG("Units selection changed: <%s>", pud_unit2str(ed->sel_unit));

   sprite_tile_size_get(ed->sel_unit, &w, &h);
   eo_do(
      ed->bitmap,
      elm_obj_bitmap_cursor_size_set(w, h),
      elm_obj_bitmap_cursor_visibility_set(EINA_TRUE)
   );
   toolbar_actions_segment_unselect(ed);
   editor_sel_action_set(ed, 0);
}

static void
_radio_players_changed_cb(void        *data,
                          Evas_Object *obj,
                          void        *event EINA_UNUSED)
{
   Editor *ed;

   ed = evas_object_data_get(obj, "editor");
   _radio_changed_common_do(data, (int *)(&(ed->sel_player)));
   DBG("Player selection changed: <%s>", pud_color2str(ed->sel_player));

   menu_units_side_enable(ed, ed->pud->side.players[ed->sel_player]);
}

static void
_delete_cb(void        *data,
           Evas_Object *obj  EINA_UNUSED,
           void        *evt  EINA_UNUSED)
{
   Editor *ed = data;
   editor_handle_delete(ed);
}

static void
_undo_cb(void        *data EINA_UNUSED,
         Evas_Object *obj  EINA_UNUSED,
         void        *evt  EINA_UNUSED)
{
   // TODO
}

static void
_redo_cb(void        *data EINA_UNUSED,
         Evas_Object *obj  EINA_UNUSED,
         void        *evt  EINA_UNUSED)
{
   // TODO
}

static void
_minimap_show_cb(void        *data,
                 Evas_Object *obj  EINA_UNUSED,
                 void        *evt  EINA_UNUSED)
{
   Editor *ed = data;
   minimap_show(ed);
}


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
menu_add(Editor *ed)
{
   Elm_Object_Item *itm, *i;
   Evas_Object *rd;

   ed->menu = elm_win_main_menu_get(ed->win);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed->menu, EINA_FALSE);

   evas_object_data_set(ed->menu, "editor", ed);

   /*==== FILE MENU ====*/
   itm = elm_menu_item_add(ed->menu, NULL, NULL,  "File", NULL, NULL);
   elm_menu_item_add(ed->menu, itm, NULL, "New...", _win_new_cb, NULL);
   elm_menu_item_add(ed->menu, itm, NULL, "Open...", _win_open_cb, ed);
   elm_menu_item_add(ed->menu, itm, NULL, "Save", _win_save_cb, ed);
   elm_menu_item_add(ed->menu, itm, NULL, "Save As...", _win_save_as_cb, ed);
   elm_menu_item_separator_add(ed->menu, itm);
   elm_menu_item_add(ed->menu, itm, NULL, "Close", _win_del_cb, ed);

   /*==== EDIT MENU ====*/
   itm = elm_menu_item_add(ed->menu, NULL, NULL, "Edit", NULL, NULL);
   elm_menu_item_add(ed->menu, itm, NULL, "Undo", _undo_cb, ed);
   elm_menu_item_add(ed->menu, itm, NULL, "Redo", _redo_cb, ed);
   elm_menu_item_separator_add(ed->menu, itm);
   elm_menu_item_add(ed->menu, itm, NULL, "Delete", _delete_cb, ed);

   /*==== VIEW MENU ====*/
   itm = elm_menu_item_add(ed->menu, NULL, NULL, "View", NULL, NULL);
   elm_menu_item_add(ed->menu, itm, NULL, "Show Minimap", _minimap_show_cb, ed);


   /*==== TOOLS MENU ====*/
   i = itm = elm_menu_item_add(ed->menu, NULL, NULL, "Tools", NULL, NULL);

#define RADIO_ADD_COMMON(unit_, label_, storage_) \
   _radio_add(ed, rd, unit_, i, label_, _radio_units_changed_cb, storage_)

#define RADIO_ADD_HUMAN(unit_, label_) RADIO_ADD_COMMON(unit_, label_, ed->human_menus)
#define RADIO_ADD_ORC(unit_, label_) RADIO_ADD_COMMON(unit_, label_, ed->orc_menus)
#define RADIO_ADD(unit_, label_) RADIO_ADD_COMMON(unit_, label_, NULL)

   rd = NULL; /* Unset radio group */
   rd = RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_START, "Human Start Location");

   i = elm_menu_item_add(ed->menu, itm, NULL, "Human Air", NULL, NULL);
   eina_array_push(ed->human_menus, i);
   RADIO_ADD_HUMAN(PUD_UNIT_GNOMISH_FLYING_MACHINE, "Gnomish Flying Machine");
   RADIO_ADD_HUMAN(PUD_UNIT_GRYPHON_RIDER, "Gryphon Rider");

   i = elm_menu_item_add(ed->menu, itm, NULL, "Human Land", NULL, NULL);
   eina_array_push(ed->human_menus, i);
   RADIO_ADD_HUMAN(PUD_UNIT_PEASANT, "Peasant");
   RADIO_ADD_HUMAN(PUD_UNIT_INFANTRY, "Footman");
   RADIO_ADD_HUMAN(PUD_UNIT_ARCHER, "Elven Archer");
   RADIO_ADD_HUMAN(PUD_UNIT_KNIGHT, "Knight");
   RADIO_ADD_HUMAN(PUD_UNIT_BALLISTA, "Balista");
   RADIO_ADD_HUMAN(PUD_UNIT_DWARVES, "Dwarven Demolition Squad");
   RADIO_ADD_HUMAN(PUD_UNIT_MAGE, "Mage");

   i = elm_menu_item_add(ed->menu, itm, NULL, "Human Water", NULL, NULL);
   eina_array_push(ed->human_menus, i);
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_TANKER, "Human Tanker");
   RADIO_ADD_HUMAN(PUD_UNIT_ELVEN_DESTROYER, "Elven Destroyer");
   RADIO_ADD_HUMAN(PUD_UNIT_BATTLESHIP, "Battleship");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_TRANSPORT, "Human Transport");
   RADIO_ADD_HUMAN(PUD_UNIT_GNOMISH_SUBMARINE, "Gnomish Submarine");

   i = elm_menu_item_add(ed->menu, itm, NULL, "Human Buildings", NULL, NULL);
   eina_array_push(ed->human_menus, i);
   RADIO_ADD_HUMAN(PUD_UNIT_FARM, "Farm");
   RADIO_ADD_HUMAN(PUD_UNIT_TOWN_HALL, "Town Hall");
   RADIO_ADD_HUMAN(PUD_UNIT_KEEP, "Keep");
   RADIO_ADD_HUMAN(PUD_UNIT_CASTLE, "Castle");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_BARRACKS, "Human Barracks");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_SHIPYARD, "Human Shipyard");
   RADIO_ADD_HUMAN(PUD_UNIT_ELVEN_LUMBER_MILL, "Elven Lumber Mill");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_FOUNDRY, "Human Foundry");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_REFINERY, "Human Refinery");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_OIL_WELL, "Human Oil Platform");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_BLACKSMITH, "Human Blacksmith");
   RADIO_ADD_HUMAN(PUD_UNIT_STABLES, "Stables");
   RADIO_ADD_HUMAN(PUD_UNIT_CHURCH, "Church");
   RADIO_ADD_HUMAN(PUD_UNIT_GNOMISH_INVENTOR, "Gnomish Inventor");
   RADIO_ADD_HUMAN(PUD_UNIT_GRYPHON_AVIARY, "Gryphon Aviary");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_SCOUT_TOWER, "Human Scout Tower");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_GUARD_TOWER, "Human Guard Tower");
   RADIO_ADD_HUMAN(PUD_UNIT_HUMAN_CANNON_TOWER, "Human Cannon Tower");
   RADIO_ADD_HUMAN(PUD_UNIT_MAGE_TOWER, "Mage Tower");

   elm_menu_item_separator_add(ed->menu, itm);

   i = itm;
   RADIO_ADD_ORC(PUD_UNIT_ORC_START, "Orc Start Location");

   i = elm_menu_item_add(ed->menu, itm, NULL, "Orc Air", NULL, NULL);
   eina_array_push(ed->orc_menus, i);
   RADIO_ADD(PUD_UNIT_GOBLIN_ZEPPLIN, "Goblin Zepplin");
   RADIO_ADD(PUD_UNIT_DRAGON, "Dragon");

   i = elm_menu_item_add(ed->menu, itm, NULL, "Orc Land", NULL, NULL);
   eina_array_push(ed->orc_menus, i);
   RADIO_ADD_ORC(PUD_UNIT_PEON, "Peon");
   RADIO_ADD_ORC(PUD_UNIT_GRUNT, "Grunt");
   RADIO_ADD_ORC(PUD_UNIT_AXETHROWER, "Troll Axethrower");
   RADIO_ADD_ORC(PUD_UNIT_OGRE, "Ogre");
   RADIO_ADD_ORC(PUD_UNIT_CATAPULT, "Catapult");
   RADIO_ADD_ORC(PUD_UNIT_GOBLIN_SAPPER, "Goblin Sapper");
   RADIO_ADD_ORC(PUD_UNIT_DEATH_KNIGHT, "Death Knight");

   i = elm_menu_item_add(ed->menu, itm, NULL, "Orc Water", NULL, NULL);
   eina_array_push(ed->orc_menus, i);
   RADIO_ADD_ORC(PUD_UNIT_ORC_TANKER, "Orc Tanker");
   RADIO_ADD_ORC(PUD_UNIT_TROLL_DESTROYER, "Troll Destroyer");
   RADIO_ADD_ORC(PUD_UNIT_JUGGERNAUGHT, "Juggernaught");
   RADIO_ADD_ORC(PUD_UNIT_ORC_TRANSPORT, "Orc Transport");
   RADIO_ADD_ORC(PUD_UNIT_GIANT_TURTLE, "Giant Turtle");

   i = elm_menu_item_add(ed->menu, itm, NULL, "Orc Buildings", NULL, NULL);
   eina_array_push(ed->orc_menus, i);
   RADIO_ADD_ORC(PUD_UNIT_PIG_FARM, "Pig Farm");
   RADIO_ADD_ORC(PUD_UNIT_GREAT_HALL, "Great Hall");
   RADIO_ADD_ORC(PUD_UNIT_STRONGHOLD, "Stronghold");
   RADIO_ADD_ORC(PUD_UNIT_FORTRESS, "Fortress");
   RADIO_ADD_ORC(PUD_UNIT_ORC_BARRACKS, "Orc Barracks");
   RADIO_ADD_ORC(PUD_UNIT_ORC_SHIPYARD, "Orc Shipyard");
   RADIO_ADD_ORC(PUD_UNIT_TROLL_LUMBER_MILL, "Troll Lumber Mill");
   RADIO_ADD_ORC(PUD_UNIT_ORC_FOUNDRY, "Orc Foundry");
   RADIO_ADD_ORC(PUD_UNIT_ORC_REFINERY, "Orc Refinery");
   RADIO_ADD_ORC(PUD_UNIT_ORC_OIL_WELL, "Orc Oil Platform");
   RADIO_ADD_ORC(PUD_UNIT_ORC_BLACKSMITH, "Orc Blacksmith");
   RADIO_ADD_ORC(PUD_UNIT_OGRE_MOUND, "Ogre Mound");
   RADIO_ADD_ORC(PUD_UNIT_ALTAR_OF_STORMS, "Altar of Storms");
   RADIO_ADD_ORC(PUD_UNIT_GOBLIN_ALCHEMIST, "Goblin Alchemist");
   RADIO_ADD_ORC(PUD_UNIT_DRAGON_ROOST, "Dragon Roost");
   RADIO_ADD_ORC(PUD_UNIT_ORC_SCOUT_TOWER, "Orc Scout Tower");
   RADIO_ADD_ORC(PUD_UNIT_ORC_GUARD_TOWER, "Orc Guard Tower");
   RADIO_ADD_ORC(PUD_UNIT_ORC_CANNON_TOWER, "Orc Cannon Tower");
   RADIO_ADD_ORC(PUD_UNIT_TEMPLE_OF_THE_DAMNED, "Temple of the Damned");

   elm_menu_item_separator_add(ed->menu, itm);

   i = itm;
   RADIO_ADD(PUD_UNIT_GOLD_MINE, "Gold Mine");
   RADIO_ADD(PUD_UNIT_OIL_PATCH, "Oil Patch");
   RADIO_ADD(PUD_UNIT_CRITTER, "Critter");
   RADIO_ADD(PUD_UNIT_CIRCLE_OF_POWER, "Circle of Power");
   RADIO_ADD(PUD_UNIT_DARK_PORTAL, "Dark Portal");
   RADIO_ADD(PUD_UNIT_RUNESTONE, "Runestone");

   elm_menu_item_separator_add(ed->menu, itm);

   i = elm_menu_item_add(ed->menu, itm, NULL, "NPC's", NULL, NULL);

   RADIO_ADD_ORC(PUD_UNIT_CHO_GALL, "Cho'Gall");
   RADIO_ADD_ORC(PUD_UNIT_ZULJIN, "Zuljin");
   RADIO_ADD_ORC(PUD_UNIT_GUL_DAN, "Gul'Dan");
   RADIO_ADD_ORC(PUD_UNIT_GROM_HELLSCREAM, "Grom Hellscream");
   RADIO_ADD_ORC(PUD_UNIT_KHORGATH_BLADEFIST, "Khorgath Bladefist");
   RADIO_ADD_ORC(PUD_UNIT_DENTARG, "Dentarg");
   RADIO_ADD_ORC(PUD_UNIT_TERON_GOREFIEND, "Teron Gorefiend");
   RADIO_ADD_ORC(PUD_UNIT_DEATHWING, "Deathwing");

   elm_menu_item_separator_add(ed->menu, i);

   RADIO_ADD_HUMAN(PUD_UNIT_LOTHAR, "Lothar");
   RADIO_ADD_HUMAN(PUD_UNIT_UTHER_LIGHTBRINGER, "Uther Lightbringer");
   RADIO_ADD_HUMAN(PUD_UNIT_TURALYON, "Turalyon");
   RADIO_ADD_HUMAN(PUD_UNIT_ALLERIA, "Alleria");
   RADIO_ADD_HUMAN(PUD_UNIT_DANATH, "Danath");
   RADIO_ADD_HUMAN(PUD_UNIT_KHADGAR, "Khadgar");
   RADIO_ADD_HUMAN(PUD_UNIT_KURDAN_AND_SKY_REE, "Kurdan and Sky'Ree");

   elm_menu_item_separator_add(ed->menu, i);

   RADIO_ADD(PUD_UNIT_SKELETON, "Skeleton");
   RADIO_ADD(PUD_UNIT_DAEMON, "Daemon");

   /* Add a fictive radio which will be used to reset the units selection */
   ed->radio_units_reset = _radio_add(ed, rd, PUD_UNIT_NONE, NULL, NULL,
                                      _radio_units_changed_cb, EINA_FALSE);
   menu_unit_selection_reset(ed);
#undef RADIO_ADD
#undef RADIO_ADD_HUMAN
#undef RADIO_ADD_ORC
#undef RADIO_ADD_COMMON

   itm = elm_menu_item_add(ed->menu, NULL, NULL, "Players", NULL, NULL);

#define RADIO_ADD(unit_, label_) \
   _radio_add(ed, rd, unit_, itm, label_, _radio_players_changed_cb, EINA_FALSE)

   rd = NULL; /* Reset the radio group */
   rd = RADIO_ADD(PUD_PLAYER_RED, STR_PLAYER_1);
   RADIO_ADD(PUD_PLAYER_BLUE,     STR_PLAYER_2);
   RADIO_ADD(PUD_PLAYER_GREEN,    STR_PLAYER_3);
   RADIO_ADD(PUD_PLAYER_VIOLET,   STR_PLAYER_4);
   RADIO_ADD(PUD_PLAYER_ORANGE,   STR_PLAYER_5);
   RADIO_ADD(PUD_PLAYER_BLACK,    STR_PLAYER_6);
   RADIO_ADD(PUD_PLAYER_WHITE,    STR_PLAYER_7);
   RADIO_ADD(PUD_PLAYER_YELLOW,   STR_PLAYER_8);

#undef RADIO_ADD

   elm_menu_item_separator_add(ed->menu, itm);
   elm_menu_item_add(ed->menu, itm, NULL, "Map Properties...", _map_properties_cb, ed);
   elm_menu_item_add(ed->menu, itm, NULL, "Player Properties...", _player_properties_cb, ed);
   elm_menu_item_add(ed->menu, itm, NULL, "Starting Properties...", _starting_properties_cb, ed);
   i = elm_menu_item_add(ed->menu, itm, NULL, "Units Properties...", _units_properties_cb, ed);
   elm_object_item_disabled_set(i, EINA_TRUE);
   i = elm_menu_item_add(ed->menu, itm, NULL, "Upgrades Properties...", _upgrades_properties_cb, ed);
   elm_object_item_disabled_set(i, EINA_TRUE);

   itm = elm_menu_item_add(ed->menu, NULL, NULL, "Help", NULL, NULL);
   elm_object_item_disabled_set(itm, EINA_TRUE);


   return EINA_TRUE;
}

void
menu_units_side_enable(Editor   *ed,
                       Pud_Side  enable)
{
   Elm_Object_Item *menu;
   Eina_Array_Iterator iterator;
   unsigned int i;
   const Eina_Bool disable_orcs = (enable == PUD_SIDE_HUMAN);
   const Eina_Bool disable_humans = !disable_orcs;

   EINA_ARRAY_ITER_NEXT(ed->orc_menus, i, menu, iterator)
      elm_object_item_disabled_set(menu, disable_orcs);

   EINA_ARRAY_ITER_NEXT(ed->human_menus, i, menu, iterator)
      elm_object_item_disabled_set(menu, disable_humans);
}

void
menu_enabled_set(Editor    *ed,
                 Eina_Bool  set)
{
   Eina_List *mi, *l;
   Elm_Object_Item *eoi;

   /* Because this is used for disabled_set() */
   set = !set;

   mi = (Eina_List *)elm_menu_items_get(ed->menu);
   EINA_LIST_FOREACH(mi, l, eoi)
      elm_object_item_disabled_set(eoi, set);
}

void
menu_unit_selection_reset(Editor *ed)
{
   elm_radio_value_set(ed->radio_units_reset, PUD_UNIT_NONE);
   ed->sel_unit = PUD_UNIT_NONE;
}


/*============================================================================*
 *                               Map Properties                               *
 *============================================================================*/

static Evas_Object *
_table_add(Evas_Object *obj)
{
   Evas_Object *t;

   t = elm_table_add(obj);
   evas_object_size_hint_weight_set(t, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(t, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_homogeneous_set(t, EINA_TRUE);
   elm_table_padding_set(t, 20, 6);
   elm_object_content_set(obj, t);
   evas_object_show(t);

   return t;
}

static Evas_Object *
_frame_add(Evas_Object *parent,
           const char  *text)
{
   Evas_Object *f;

   f = elm_frame_add(parent);
   elm_object_text_set(f, text);
   evas_object_size_hint_weight_set(f, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(f, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(f);

   return f;
}

static Evas_Object *
_scroller_add(Evas_Object *parent)
{
   Evas_Object *s;

   s = elm_scroller_add(parent);
   evas_object_size_hint_weight_set(s, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(s, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(s);

   return s;
}


static void
_era_changed_cb(void        *data,
                Evas_Object *obj,
                void        *event_info EINA_UNUSED)
{
   Editor *ed = data;
   pud_era_set(ed->pud, elm_radio_value_get(obj));
}

Evas_Object *
menu_map_properties_new(Editor      *ed,
                        Evas_Object *parent)
{
   Evas_Object *f, *b, *o, *grp;

   /* Frame for map era */
   f = elm_frame_add(parent);
   elm_object_text_set(f, "Map Properties");
   evas_object_size_hint_weight_set(f, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(f, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(f);
   b = elm_box_add(f); /* Box */
   elm_object_content_set(f, b);
   elm_box_align_set(b, 0.0f, 0.0f);
   evas_object_show(b);

   /* Tileset item 1 (Forest) */
   o = elm_radio_add(b);
   elm_radio_state_value_set(o, PUD_ERA_FOREST);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, "Forest");
   elm_box_pack_end(b, o);
   evas_object_show(o);
   grp = o;
   evas_object_smart_callback_add(o, "changed", _era_changed_cb, ed);

   /* Tileset item 2 (Winter) */
   o = elm_radio_add(b);
   elm_radio_state_value_set(o, PUD_ERA_WINTER);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, "Winter");
   elm_box_pack_end(b, o);
   evas_object_show(o);
   elm_radio_group_add(o, grp);
   evas_object_smart_callback_add(o, "changed", _era_changed_cb, ed);

   /* Tileset item 3 (Wasteland) */
   o = elm_radio_add(b);
   elm_radio_state_value_set(o, PUD_ERA_WASTELAND);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, "Wasteland");
   elm_box_pack_end(b, o);
   evas_object_show(o);
   elm_radio_group_add(o, grp);
   evas_object_smart_callback_add(o, "changed", _era_changed_cb, ed);

   /* Tileset item 4 (Swamp) */
   o = elm_radio_add(b);
   elm_radio_state_value_set(o, PUD_ERA_SWAMP);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, "Swamp");
   elm_box_pack_end(b, o);
   evas_object_show(o);
   evas_object_smart_callback_add(o, "changed", _era_changed_cb, ed);
   elm_radio_group_add(o, grp);

   /* Default value */
   elm_radio_value_set(grp, PUD_ERA_FOREST);

   return f;
}

Eina_Bool
menu_init(void)
{
   static uint8_t values[] = {
      PUD_SIDE_ORC,
      PUD_SIDE_HUMAN,
      PUD_OWNER_HUMAN,
      PUD_OWNER_COMPUTER,
      PUD_OWNER_RESCUE_PASSIVE,
      PUD_OWNER_RESCUE_ACTIVE,
      PUD_AI_LAND_ATTACK,
      PUD_AI_PASSIVE,
      PUD_AI_SEA_ATTACK,
      PUD_AI_AIR_ATTACK
   };
   Eina_Bool chk;
   Eina_Stringshare *shr;

   _values = eina_hash_stringshared_new(NULL);
   if (EINA_UNLIKELY(!_values))
     {
        CRI("Failed to create hash of stringshares");
        return EINA_FALSE;
     }

#define ADD(str, val) \
   do { \
      shr = eina_stringshare_add_length(str, sizeof(str) - 1); \
      if (EINA_UNLIKELY(!shr)) { \
         CRI("Failed to create stringshare for string \"" str "\""); \
         return EINA_FALSE; \
      } \
      chk = eina_hash_add(_values, shr, val); \
      if (EINA_UNLIKELY(!chk)) { \
         CRI("Failed to add value <0x%x> for stringshare \"%s\"", *val, shr); \
         eina_stringshare_del(shr); \
         eina_hash_free(_values); \
         return EINA_FALSE; \
      } \
   } while (0)

   ADD(STR_PUD_SIDE_ORC            , &(values[0]));
   ADD(STR_PUD_SIDE_HUMAN          , &(values[1]));
   ADD(STR_PUD_OWNER_HUMAN         , &(values[2]));
   ADD(STR_PUD_OWNER_COMPUTER      , &(values[3]));
   ADD(STR_PUD_OWNER_RESCUE_PASSIVE, &(values[4]));
   ADD(STR_PUD_OWNER_RESCUE_ACTIVE , &(values[5]));
   ADD(STR_PUD_AI_LAND_ATTACK      , &(values[7]));
   ADD(STR_PUD_AI_PASSIVE          , &(values[7]));
   ADD(STR_PUD_AI_SEA_ATTACK       , &(values[8]));
   ADD(STR_PUD_AI_AIR_ATTACK       , &(values[9]));

#undef ADD

   return EINA_TRUE;
}

void
menu_shutdown(void)
{
   eina_hash_free(_values);
}

/*============================================================================*
 *                             Players Properties                             *
 *============================================================================*/

static void
_hoversel_selected_cb(void        *data EINA_UNUSED,
                      Evas_Object *obj,
                      void        *info)
{
   elm_object_text_set(obj, elm_object_item_text_get(info));
}

static inline void
_hoversel_item_add(Evas_Object   *hoversel,
                   const char    *label,
                   Evas_Smart_Cb  func,
                   void          *bind)
{
   elm_hoversel_item_add(hoversel, label, NULL, ELM_ICON_NONE, func, bind);
}

static void
_pack_label(Evas_Object  *table,
            unsigned int  row,
            unsigned int  col,
            const char   *label)
{
   Evas_Object *o;

   o = elm_label_add(table);
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   elm_object_text_set(o, label);
   evas_object_show(o);
   elm_table_pack(table, o, col, row, 1, 1);
}

static void
_bind_cb(void        *data,
         Evas_Object *obj  EINA_UNUSED,
         void        *evt)
{
   uint8_t *bind = data;
   uint8_t *val;
   Eina_Stringshare *text;

   eo_do(evt, text = elm_wdg_item_part_text_get("default"));

   val = eina_hash_find(_values, text);
   if (EINA_UNLIKELY(!val))
     {
        CRI("Failed to get bind value for text \"%s\"", text);
        return;
     }
   *bind = *val;
}

static void
_bind_side_cb(void        *data,
              Evas_Object *obj,
              void        *evt)
{
   Editor *ed;

   _bind_cb(data, obj, evt);

   /* This callback is called from UI exclusively.
    * It is therefore safe to call editor_focused_get() */
   ed = editor_focused_get();
   menu_units_side_enable(ed, ed->pud->side.players[ed->sel_player]);
}

static Evas_Object *
_hoversel_add(Evas_Object *parent,
              const char  *init_label)
{
   Evas_Object *o;

   o = elm_hoversel_add(parent);
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "selected", _hoversel_selected_cb, NULL);
   elm_hoversel_hover_parent_set(o, parent);
   elm_object_text_set(o, init_label);
   evas_object_show(o);

   return o;
}

static void
_pack_race_selector(Evas_Object  *table,
                    unsigned int  row,
                    unsigned int  col,
                    uint8_t      *bind)
{
   Evas_Object *o;
   const char *human_race = STR_PUD_SIDE_HUMAN;
   const char *orc_race = STR_PUD_SIDE_ORC;
   const char *race = (*bind == PUD_SIDE_HUMAN) ? human_race : orc_race;

   o = _hoversel_add(table, race);
   _hoversel_item_add(o, human_race, _bind_side_cb, bind);
   _hoversel_item_add(o, orc_race, _bind_side_cb, bind);

   elm_table_pack(table, o, col, row, 1, 1);
}

static const char *
_owner_to_string(uint8_t owner)
{
   switch (owner)
     {
      case PUD_OWNER_COMPUTER:       return STR_PUD_OWNER_COMPUTER;
      case PUD_OWNER_HUMAN:          return STR_PUD_OWNER_HUMAN;
      case PUD_OWNER_RESCUE_PASSIVE: return STR_PUD_OWNER_RESCUE_PASSIVE;
      case PUD_OWNER_RESCUE_ACTIVE:  return STR_PUD_OWNER_RESCUE_ACTIVE;

      default:
         CRI("Unhandled value %x", owner);
         return NULL;
     }
}

static void
_pack_owner_selector(Evas_Object  *table,
                     unsigned int  row,
                     unsigned int  col,
                     uint8_t      *bind)
{
   uint8_t values[] = {
      PUD_OWNER_COMPUTER,
      PUD_OWNER_HUMAN,
      PUD_OWNER_RESCUE_PASSIVE,
      PUD_OWNER_RESCUE_ACTIVE,
   };
   Evas_Object *o;
   unsigned int i;

   o = _hoversel_add(table, _owner_to_string(*bind));
   for (i = 0; i < EINA_C_ARRAY_LENGTH(values); ++i)
     _hoversel_item_add(o, _owner_to_string(values[i]), _bind_cb, bind);
   elm_table_pack(table, o, col, row, 1, 1);
}

static const char *
_ai_to_string(uint8_t ai)
{
   static char buf[32];

   switch (ai)
     {
      case PUD_AI_LAND_ATTACK: return STR_PUD_AI_LAND_ATTACK;
      case PUD_AI_SEA_ATTACK:  return STR_PUD_AI_SEA_ATTACK;
      case PUD_AI_AIR_ATTACK:  return STR_PUD_AI_AIR_ATTACK;
      case PUD_AI_PASSIVE:     return STR_PUD_AI_PASSIVE;

      default:
         break;
     }

   if ((ai >= PUD_AI_ORC_3) && (ai <= PUD_AI_HUMAN_13))
     {
        if (ai % 2 == 0) /* orc */
          snprintf(buf, sizeof(buf), STR_PUD_AI_HUMAN_FMT, ai / 2);
        else /* human */
          snprintf(buf, sizeof(buf), STR_PUD_AI_ORC_FMT, (ai / 2) + 1);
     }
   else if ((ai >= PUD_AI_EXPANSION_1) && (ai <= PUD_AI_EXPANSION_51))
     {
        snprintf(buf, sizeof(buf), STR_PUD_AI_EXPANSION_FMT,
                 ai - PUD_AI_EXPANSION_1 + 1);
     }
   else
     {
        CRI("Unhandled AI value %x", ai);
        return NULL;
     }

   return buf;
}

static void
_pack_ai_selector(Evas_Object  *table,
                  unsigned int  row,
                  unsigned int  col,
                  uint8_t      *bind)
{
   Evas_Object *o;
   unsigned int i;
   uint8_t values[] = {
      PUD_AI_LAND_ATTACK,
      PUD_AI_PASSIVE,
      PUD_AI_SEA_ATTACK,
      PUD_AI_AIR_ATTACK
   };

   o = _hoversel_add(table, _ai_to_string(*bind));

   for (i = 0; i < EINA_C_ARRAY_LENGTH(values); ++i)
     _hoversel_item_add(o, _ai_to_string(values[i]), _bind_cb, bind);

   // More AIs. Slow!!
   //for (i = PUD_AI_ORC_4; i <= PUD_AI_HUMAN_6/*PUD_AI_ORC_13*/; ++i)
   //  ITEM_ADD(i);
   //for (i = PUD_AI_EXPANSION_1; i < PUD_AI_EXPANSION_51; ++i)
   //  ITEM_ADD(i);

   elm_table_pack(table, o, col, row, 1, 1);
}

Evas_Object *
menu_player_properties_new(Editor      *ed,
                           Evas_Object *parent)
{
   Evas_Object *f, *t;
   unsigned int i;

   f = _frame_add(parent, "Players Properties");
   t = _table_add(f);

   /* Players */
   _pack_label(t, 0, 0, "Player");
   _pack_label(t, 1, 0, STR_PLAYER_1);
   _pack_label(t, 2, 0, STR_PLAYER_2);
   _pack_label(t, 3, 0, STR_PLAYER_3);
   _pack_label(t, 4, 0, STR_PLAYER_4);
   _pack_label(t, 5, 0, STR_PLAYER_5);
   _pack_label(t, 6, 0, STR_PLAYER_6);
   _pack_label(t, 7, 0, STR_PLAYER_7);
   _pack_label(t, 8, 0, STR_PLAYER_8);

   /* Race */
   _pack_label(t, 0, 1, "Race");
   for (i = 0; i < 8; ++i)
     _pack_race_selector(t, i + 1, 1, &(ed->pud->side.players[i]));

   /* Owner */
   _pack_label(t, 0, 2, "Owner");
   for (i = 0; i < 8; ++i)
     _pack_owner_selector(t, i + 1, 2, &(ed->pud->owner.players[i]));

   /* AI */
   _pack_label(t, 0, 3, "AI");
   for (i = 0; i < 8; ++i)
     _pack_ai_selector(t, i + 1, 3, &(ed->pud->ai.players[i]));


   return f;
}


/*============================================================================*
 *                             Starting Properties                            *
 *============================================================================*/

static Eina_Bool
_validator_cb(void                       *data,
              Eo                         *obj,
              const Eo_Event_Description *desc,
              void                       *info)
{
   Eina_Bool status;
   Validator *val = data;
   Elm_Validate_Content *vc = info;
   unsigned long int numeric;
   const char *str;

   status = elm_validator_regexp_helper(val->re, obj, desc, info);
   if (status == EO_CALLBACK_CONTINUE)
     {
        status = EO_CALLBACK_STOP;

        /* Trim leading zeros */
        str = vc->text;
        while ((*str == '0') && (*str != '\0')) str++;

        /* Avoid overflows */
        if (strlen(str) <= 10)
          {
             numeric = strtoul(str, NULL, 10);
             if ((numeric >= val->range.start) && (numeric <= val->range.end))
               {
                  if (val->prescalor.inverse)
                    numeric = val->prescalor.inverse(numeric);
                  switch (val->bind.type)
                    {
                     case POINTER_TYPE_BYTE:
                        *(val->bind.ptr.byte_ptr) = (uint8_t)numeric;
                        break;

                     case POINTER_TYPE_WORD:
                        *(val->bind.ptr.word_ptr) = (uint16_t)numeric;
                        break;

                     case POINTER_TYPE_LONG:
                        *(val->bind.ptr.long_ptr) = (uint32_t)numeric;
                        break;
                    }
                  status = EO_CALLBACK_CONTINUE;
               }
          }
     }
   return status;
}

static void
_pack_range_entry(Evas_Object  *table,
                  unsigned int  row,
                  unsigned int  col,
                  uint32_t      start,
                  uint32_t      end,
                  void         *bind,
                  Pointer_Type  type,
                  Prescalor_Cb  operation,
                  Prescalor_Cb  inverse)
{
   Evas_Object *o;
   Validator *val;
   char buf[8];
   uint32_t value;

   o = elm_entry_add(table);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_single_line_set(o, EINA_TRUE);
   elm_entry_scrollable_set(o, EINA_TRUE);
   elm_entry_editable_set(o, EINA_TRUE);
   evas_object_show(o);

   // FIXME Leak val
   val = _validator_new();
   val->range.start = start;
   val->range.end = end;
   val->bind.type = type;
   val->bind.ptr.byte_ptr = bind;
   val->prescalor.operation = operation;
   val->prescalor.inverse = inverse;

   eo_do(o, eo_event_callback_add(ELM_ENTRY_EVENT_VALIDATE,
                                  _validator_cb, val));

   switch (val->bind.type)
     {
      case POINTER_TYPE_BYTE: value = *(val->bind.ptr.byte_ptr); break;
      case POINTER_TYPE_WORD: value = *(val->bind.ptr.word_ptr); break;
      case POINTER_TYPE_LONG: value = *(val->bind.ptr.long_ptr); break;
     }

   if (val->prescalor.operation) value = val->prescalor.operation(value);
   snprintf(buf, sizeof(buf), "%u", value);
   elm_entry_entry_set(o, buf);

   elm_table_pack(table, o, col, row, 1, 1);
}

static inline void
_pack_byte_entry(Evas_Object *table,
                 unsigned int  row,
                 unsigned int  col,
                 uint8_t      *bind)
{
   _pack_range_entry(table, row, col, 0, UINT8_MAX, bind, POINTER_TYPE_BYTE,
                     NULL, NULL);
}

static inline void
_pack_word_entry(Evas_Object *table,
                 unsigned int  row,
                 unsigned int  col,
                 uint16_t     *bind)
{
   _pack_range_entry(table, row, col, 0, UINT16_MAX, bind, POINTER_TYPE_WORD,
                     NULL, NULL);
}

static uint32_t _x10(uint32_t val) { return val * 10; }
static uint32_t _d10(uint32_t val) { return val / 10; }

static inline void
_pack_build_cost_entry(Evas_Object  *table,
                       unsigned int  row,
                       unsigned int  col,
                       uint8_t      *bind)
{
   _pack_range_entry(table, row, col, 0, UINT8_MAX, bind, POINTER_TYPE_BYTE,
                     _x10, _d10);
}


Evas_Object *
menu_starting_properties_new(Editor      *ed,
                             Evas_Object *parent)
{
   Evas_Object *f, *t;
   unsigned int i;

   f = _frame_add(parent, "Starting Properties");
   t = _table_add(f);

   /* Players */
   _pack_label(t, 0, 0, "Player");
   _pack_label(t, 1, 0, "Player 1 (RED)");
   _pack_label(t, 2, 0, "Player 2 (BLUE)");
   _pack_label(t, 3, 0, "Player 3 (GREEN)");
   _pack_label(t, 4, 0, "Player 4 (VIOLET)");
   _pack_label(t, 5, 0, "Player 5 (ORANGE)");
   _pack_label(t, 6, 0, "Player 6 (BLACK)");
   _pack_label(t, 7, 0, "Player 7 (WHITE)");
   _pack_label(t, 8, 0, "Player 8 (YELLOW)");

   /* Gold */
   _pack_label(t, 0, 1, "Gold");
   for (i = 0; i < 8; ++i)
     _pack_word_entry(t, i + 1, 1, &(ed->pud->sgld.players[i]));

   /* Lumber */
   _pack_label(t, 0, 2, "Lumber");
   for (i = 0; i < 8; ++i)
     _pack_word_entry(t, i + 1, 2, &(ed->pud->slbr.players[i]));

   /* Oil */
   _pack_label(t, 0, 3, "Oil");
   for (i = 0; i < 8; ++i)
     _pack_word_entry(t, i + 1, 3, &(ed->pud->soil.players[i]));

   return f;
}


// FIXME Units properties is waaaaay to slow.
// FIXME There is nothing much I can do, ELM does not scale well for massive UI
//
///*============================================================================*
// *                              Units Properties                              *
// *============================================================================*/
//
//static Evas_Object *
//_menu_prop_genlist_get_cb(void        *data,
//                          Evas_Object *obj,
//                          const char  *part)
//{
//   Evas_Object *t;
//   struct _unit_data *udta = data;
//   unsigned int unit;
//   const Pud *pud;
//
//   printf("Part: %s\n", part);
//
//   pud = evas_object_data_get(obj, PUD_DATA);
//
//   t = elm_table_add(obj);
//   evas_object_size_hint_weight_set(t, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//   evas_object_size_hint_align_set(t, EVAS_HINT_FILL, EVAS_HINT_FILL);
//   elm_table_homogeneous_set(t, EINA_TRUE);
//
//
//   unit = udta - &(pud->unit_data[0]);
//   printf("Unit is %u\n", unit);
//
//   _pack_label(t, 0, 0, pud_unit2str(unit));
//   _pack_range_entry(t, 0, 1, 0, 9, &(udta->sight),
//                     POINTER_TYPE_LONG, NULL, NULL);
//   _pack_word_entry(t, 0, 2, &(udta->hp));
//   _pack_byte_entry(t, 0, 3, &(udta->build_time));
//   _pack_build_cost_entry(t, 0, 4, &(udta->gold_cost));
//   _pack_build_cost_entry(t, 0, 5, &(udta->lumber_cost));
//   _pack_build_cost_entry(t, 0, 6, &(udta->oil_cost));
//   _pack_byte_entry(t, 0, 7, &(udta->range));
//   _pack_byte_entry(t, 0, 8, &(udta->computer_react_range));
//   _pack_byte_entry(t, 0, 9, &(udta->human_react_range));
//   _pack_byte_entry(t, 0, 10, &(udta->armor));
//   _pack_byte_entry(t, 0, 11, &(udta->basic_damage));
//   _pack_byte_entry(t, 0, 12, &(udta->piercing_damage));
//   _pack_byte_entry(t, 0, 13, &(udta->decay_rate));
//   _pack_byte_entry(t, 0, 14, &(udta->annoy));
//    //    _pack_checkbox(t, 15, i + 1, &(udta->has_magic));
//    //    _pack_checkbox(t, 16, i + 1, &(udta->weapons_upgradable));
//    //    _pack_checkbox(t, 17, i + 1, &(udta->armor_upgradable));
//
//   evas_object_show(t);
//   return t;
//}
//
//static void
//_unselect_cb(void        *data EINA_UNUSED,
//             Evas_Object *obj,
//             void        *evt)
//{
//   elm_genlist_item_selected_set(evt, EINA_FALSE);
//}
//
//Evas_Object *
//menu_units_properties_new(Editor      *ed,
//                          Evas_Object *parent)
//{
//   Evas_Object *f, *gen, *s;
//   Elm_Genlist_Item_Class *itc;
//   Pud *pud = ed->pud;
//   const unsigned int units = 110;
//   unsigned int i;
//   struct _unit_data *udta;
//   Validator *validators, *v;
//
//   f = _frame_add(parent, "Units Properties");
//   gen = elm_genlist_add(f);
//   evas_object_size_hint_weight_set(gen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//   evas_object_size_hint_align_set(gen, EVAS_HINT_FILL, EVAS_HINT_FILL);
//   evas_object_show(gen);
//   elm_object_content_set(f, gen);
//
//   itc = elm_genlist_item_class_new();
//   itc->item_style = "full";
//   itc->func.text_get = NULL;
//   itc->func.content_get = _menu_prop_genlist_get_cb;
//   itc->func.state_get = NULL;
//   itc->func.del = NULL;
//
//   evas_object_data_set(gen, PUD_DATA, pud);
//
//
//   /* Settings column */
//   //_pack_label(t,  1, 0, "Sight");
//   //_pack_label(t,  2, 0, "Hit Points");
//   //_pack_label(t,  3, 0, "Build Time");
//   //_pack_label(t,  4, 0, "Gold Cost");
//   //_pack_label(t,  5, 0, "Lumber Cost");
//   //_pack_label(t,  6, 0, "Oil Cost");
//   //_pack_label(t,  7, 0, "Range");
//   //_pack_label(t,  8, 0, "Computer Reaction Range");
//   //_pack_label(t,  9, 0, "Human Reaction Range");
//   //_pack_label(t, 10, 0, "Armor");
//   //_pack_label(t, 11, 0, "Basic Damage");
//   //_pack_label(t, 12, 0, "Piercing Damage");
//   //_pack_label(t, 13, 0, "Decay Rate");
//   //_pack_label(t, 14, 0, "Annoyance");
//   //_pack_label(t, 15, 0, "Has Magic");
//   //_pack_label(t, 16, 0, "Weapons Upgradable");
//   //_pack_label(t, 17, 0, "Armor Upgradable");
//
//   for (i = 0; i < 11; ++i)
//     {
//        elm_genlist_item_append(gen, itc, &(pud->unit_data[i]), NULL,
//                                ELM_GENLIST_ITEM_NONE, _unselect_cb, NULL);
// //       udta = &(pud->unit_data[i]);
//     }
//
//   elm_genlist_item_class_free(itc);
//
//   return f;
//}


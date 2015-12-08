/*
 * tile.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

uint16_t
tile_solid_mask_get(Editor_Sel action,
                    Editor_Sel tint)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((tint != EDITOR_SEL_TINT_LIGHT) &&
                                  (tint != EDITOR_SEL_TINT_DARK), 0x0000);
   uint16_t mask;

   switch (action)
     {
      case EDITOR_SEL_ACTION_WATER:
         if (tint  == EDITOR_SEL_TINT_LIGHT) mask = 0x0010;
         else                                mask = 0x0020;
         break;

      case EDITOR_SEL_ACTION_NON_CONSTRUCTIBLE:
         if (tint  == EDITOR_SEL_TINT_LIGHT) mask = 0x0030;
         else                                mask = 0x0040;
         break;

      case EDITOR_SEL_ACTION_CONSTRUCTIBLE:
         if (tint  == EDITOR_SEL_TINT_LIGHT) mask = 0x0050;
         else                                mask = 0x0060;
         break;

      case EDITOR_SEL_ACTION_TREES:
         mask = 0x0070;
         break;

      case EDITOR_SEL_ACTION_ROCKS:
         mask = 0x0080;
         break;

      case EDITOR_SEL_ACTION_HUMAN_WALLS:
         mask = 0x0090;
         break;

      case EDITOR_SEL_ACTION_ORCS_WALLS:
         mask = 0x00a0;
         break;

      default:
         CRI("Unhandled action %x", action);
         return 0x0000;
     }

   return mask;
}


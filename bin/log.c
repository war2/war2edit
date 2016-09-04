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

#include "war2edit.h"

typedef struct
{
   Evas_Object *win;
   Evas_Object *lay;
   Evas_Object *entry;
} Console;

static Console _console;

int _war2edit_log_dom = -1;

static const char *
_level_to_prefix(Eina_Log_Level lvl)
{
   switch (lvl)
     {
      case EINA_LOG_LEVEL_CRITICAL:
         return "<failure>CRITICAL</failure> ";
      case EINA_LOG_LEVEL_ERR:
         return "<warning>ERROR</warning> ";
      case EINA_LOG_LEVEL_WARN:
         return "<warning>WARNING</warning> ";
      case EINA_LOG_LEVEL_INFO:
         return "<success>INFO</success> ";
      case EINA_LOG_LEVEL_DBG:
         return "<info>DEBUG</info> ";

      default:
         return "";
     }
}

static void
_log_cb(const Eina_Log_Domain *d,
        Eina_Log_Level         level,
        const char            *file,
        const char            *fnc,
        int                    line,
        const char            *fmt,
        void                  *data,
        va_list                args)
{
   char log[4096];
   char *ptr = log;
   int bytes, len = sizeof(log);
   va_list args_copy;

   bytes = snprintf(ptr, len, "%s (%s:%i) <b>",
                    _level_to_prefix(level), fnc, line);
   ptr += bytes;
   len -= bytes;

   va_copy(args_copy, args);
   bytes = vsnprintf(ptr, len, fmt, args_copy);
   va_end(args_copy);

   ptr += bytes;
   len -= bytes;

   strncat(ptr, "</b><br>", len);

   log[sizeof(log) - 1] = '\0';
   elm_entry_entry_append(_console.entry, log);
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, data, args);
}

static void
_hide_win_cb(void        *data EINA_UNUSED,
             Evas_Object *obj,
             void        *evt  EINA_UNUSED)
{
   evas_object_hide(obj);
}

Eina_Bool
log_init(void)
{
   Evas_Object *o;
   const char group[] = "war2edit/logconsole";
   const char part[] = "war2edit.logconsole.contents";
   Eina_Bool chk;

   _war2edit_log_dom = eina_log_domain_register("war2edit", EINA_COLOR_GREEN);
   if (EINA_UNLIKELY(_war2edit_log_dom < 0))
     {
        EINA_LOG_CRIT("Failed to create log domain");
        goto fail;
     }

   o = _console.win = elm_win_util_standard_add("logconsole", "Log Console");
   evas_object_smart_callback_add(o, "delete,request", _hide_win_cb, NULL);

   o = _console.lay = elm_layout_add(_console.win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   chk = elm_layout_file_set(o, main_edje_file_get(), group);
   if (EINA_UNLIKELY(!chk))
     {
        CRI("Failed to set layout from group %s", group);
        goto ui_fail;
     }
   elm_win_resize_object_add(_console.win, o);
   evas_object_resize(_console.win, 620, 480);

   o = _console.entry = elm_entry_add(_console.lay);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_policy_set(o, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_ON);
   elm_entry_editable_set(o, EINA_FALSE);
   elm_entry_scrollable_set(o, EINA_TRUE);
   elm_entry_single_line_set(o, EINA_FALSE);
   elm_entry_line_wrap_set(o, ELM_WRAP_WORD);
   elm_entry_input_panel_enabled_set(o, EINA_FALSE);

   elm_layout_content_set(_console.lay, part, o);

   eina_log_print_cb_set(_log_cb, NULL);

   return EINA_TRUE;

ui_fail:
   evas_object_del(_console.win);
fail:
   log_shutdown();
   return EINA_FALSE;
}

void
log_shutdown(void)
{
   if (_war2edit_log_dom >= 0)
     {
        eina_log_domain_unregister(_war2edit_log_dom);
        _war2edit_log_dom = -1;
     }
}

void
log_console_show(void)
{
   evas_object_show(_console.win);
   evas_object_show(_console.lay);
   evas_object_show(_console.entry);
}

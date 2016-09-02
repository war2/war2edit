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
#include <Ecore_Getopt.h>

static Eina_Bool _in_tree = EINA_FALSE;

static const Ecore_Getopt _options =
{
   "war2edit",
   "%prog [options] [PUD file(s)]",
   "0.1",
   "2014-2016 © Jean Guyomarc'h",
   "MIT",
   "A modest clone of Warcraft II World Map Editor",
   EINA_TRUE,
   {
      ECORE_GETOPT_STORE_TRUE('C', "clouseau", "Enable clouseau debug"),
      ECORE_GETOPT_STORE_TRUE('d', "debug", "Enable graphical debug"),
      ECORE_GETOPT_HELP ('h', "help"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_SENTINEL
   }
};

typedef struct
{
   const char *name;
   Eina_Bool (*init)(void);
   void (*shutdown)(void);
} Module;


static const Module _modules[] =
{
#define MODULE(name_) { #name_, name_ ## _init, name_ ## _shutdown }
   MODULE(log),
   MODULE(ipc),
   MODULE(atlas),
   MODULE(sprite),
   MODULE(menu),
   MODULE(prefs),
   MODULE(editor)
#undef MODULE
};

EAPI_MAIN int
elm_main(int    argc,
         char **argv)
{
   int ret = EXIT_FAILURE;
   int args;
   int i;
   Editor *ed;
   unsigned int ed_count = 0;
   Eina_Bool quit_opt = EINA_FALSE;
   Eina_Bool debug = EINA_FALSE;
   Eina_Bool clouseau = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_BOOL(clouseau),
      ECORE_GETOPT_VALUE_BOOL(debug),
      ECORE_GETOPT_VALUE_BOOL(quit_opt),
      ECORE_GETOPT_VALUE_BOOL(quit_opt)
   };
   const Module *mod_ptr;
   const Module *mod_end = &(_modules[EINA_C_ARRAY_LENGTH(_modules)]);
   const char *env;
   unsigned int debug_flags = 0;

   args = ecore_getopt_parse(&_options, values, argc, argv);
   if (args < 0)
     {
        EINA_LOG_CRIT("Getopt failed");
        goto end;
     }

   /* Quit option requested? End now, with success */
   if (quit_opt)
     {
        ret = EXIT_SUCCESS;
        goto end;
     }

   if (debug)
     debug_flags = ~0U;

   /*
    * Clouseau not pleased with ecore_event_handlers
    */
   if (clouseau)
     ipc_disable();


   /* Are we running in tree? */
   env = getenv("WAR2EDIT_IN_TREE");
   _in_tree = (env) ? !!atoi(env) : EINA_FALSE;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_language_set("");
   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
   elm_app_info_set(elm_main, "war2edit", "sprites/units/units.eet");

   for (mod_ptr = _modules; mod_ptr != mod_end; ++mod_ptr)
     {
       if (EINA_UNLIKELY(EINA_FALSE == mod_ptr->init()))
         {
            EINA_LOG_CRIT("Failed to initialize module \"%s\"", mod_ptr->name);
            goto modules_shutdown;
         }
     }

   /* Open editors for each specified files */
   for (i = args; i < argc; ++i)
     {
        /* If an editor fails to open, don't close now */
        ed = editor_new(argv[i], debug_flags);
        if (!ed)
          ERR("Failed to create editor with file \"%s\"", argv[i]);
        else
          ++ed_count;
     }

   if (ed_count == 0)
     {
        ed = editor_new(NULL, debug_flags);
        if (EINA_UNLIKELY(!ed))
          {
             CRI("Failed to create editor");
             goto modules_shutdown;
          }
     }

   /* === Main loop === */
   elm_run();
   ret = EXIT_SUCCESS;

modules_shutdown:
   for (--mod_ptr; mod_ptr >= _modules; --mod_ptr)
     mod_ptr->shutdown();
end:
   return ret;
}
ELM_MAIN()

Eina_Bool
main_in_tree_is(void)
{
   return _in_tree;
}

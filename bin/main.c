/*
 * main.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"
#include <Ecore_Getopt.h>

static const Ecore_Getopt _options =
{
   "war2edit",
   "%prog [options] [PUD file(s)]",
   "0.1",
   "2014-2015 © Jean Guyomarc'h",
   "MIT",
   "A modest clone of Warcraft II Wolrd Map Editor",
   EINA_TRUE,
   {
      ECORE_GETOPT_STORE_TRUE('x', "xdebug", "Enable graphical debug"),
      ECORE_GETOPT_HELP ('h', "help"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_SENTINEL
   }
};

EAPI_MAIN int
elm_main(int    argc,
         char **argv)
{
   int ret = EXIT_SUCCESS;
   int args;
   int i;
   Editor *ed;
   unsigned int ed_count = 0;
   Eina_Bool quit_opt = EINA_FALSE;
   Eina_Bool xdebug = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_BOOL(xdebug),
      ECORE_GETOPT_VALUE_BOOL(quit_opt),
      ECORE_GETOPT_VALUE_BOOL(quit_opt)
   };

   args = ecore_getopt_parse(&_options, values, argc, argv);
   if (args < 0)
     {
        EINA_LOG_CRIT("Getopt failed");
        ret = EXIT_FAILURE;
        goto end;
     }

   /* Quit option requested? End now, with success */
   if (quit_opt) goto end;


   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
 //  elm_language_set("");
  // elm_app_compile_bin_dir_set(PACKAGE_COMPILE_BIN_DIR);
  // elm_app_compile_data_dir_set(PACKAGE_COMPILE_DATA_DIR);
  // elm_app_info_set(elm_main, "war2edit", "themes/default.edj");

   /* Init log module */
   if (EINA_UNLIKELY(!log_init()))
     {
        EINA_LOG_CRIT("Failed to init log module");
        ret = EXIT_FAILURE;
        goto end;
     }
   DBG("Size of a single Cell is %zu", sizeof(Cell));

   /* Init texture module */
   if (EINA_UNLIKELY(!texture_init()))
     {
        CRI("Failed to init texture module");
        ret = EXIT_FAILURE;
        goto log_done;
     }

   /* Init texture module */
   if (EINA_UNLIKELY(!sprite_init()))
     {
        CRI("Failed to init sprite module");
        ret = EXIT_FAILURE;
        goto texture_done;
     }

   if (EINA_UNLIKELY(!menu_init()))
     {
        CRI("Failed to init menu module");
        ret = EXIT_FAILURE;
        goto sprite_done;
     }

   /* Init editor module */
   if (EINA_UNLIKELY(!editor_init()))
     {
        CRI("Failed to init editor module");
        ret = EXIT_FAILURE;
        goto menu_done;
     }

   /* Open editors for each specified files */
   for (i = args; i < argc; ++i)
     {
        /* If an editor fails to open, don't close now */
        ed = editor_new(argv[i], xdebug);
        if (!ed)
          ERR("Failed to create editor with file \"%s\"", argv[i]);
        else
          ++ed_count;
     }

   if (ed_count == 0)
     {
        ed = editor_new(NULL, xdebug);
        if (EINA_UNLIKELY(!ed))
          {
             CRI("Failed to create editor");
             ret = EXIT_FAILURE;
             goto editor_done;
          }
     }

   /* === Main loop === */
   elm_run();

editor_done:
   editor_shutdown();
menu_done:
   menu_shutdown();
sprite_done:
   sprite_shutdown();
texture_done:
   texture_shutdown();
log_done:
   log_shutdown();
end:
   return ret;
}
ELM_MAIN()


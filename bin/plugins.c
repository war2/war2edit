/*
 * Copyright (c) 2016 Jean Guyomarc'h
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

static Eina_Hash *_plugins = NULL;
static Eina_Prefix *_prefix = NULL;

static void
_module_close_cb(void *mod)
{
   Eina_Module *const m = mod;
   eina_module_free(m);
}

Eina_Bool
plugins_init(void)
{
   _prefix = eina_prefix_new("war2edit", NULL, "WAR2EDIT", "war2edit",
                             "default.edj", PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                             PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
   if (EINA_UNLIKELY(!_prefix))
     {
        CRI("Failed to create prefix");
        goto fail;
     }

   _plugins = eina_hash_stringshared_new(_module_close_cb);
   if (EINA_UNLIKELY(!_plugins))
     {
        CRI("Failed to create hash table");
        goto fail;
     }
   return EINA_TRUE;

fail:
   plugins_shutdown();
   return EINA_FALSE;
}

void
plugins_shutdown(void)
{
   if (_plugins)
     {
        eina_hash_free(_plugins);
        _plugins = NULL;
     }
   if (_prefix)
     {
        eina_prefix_free(_prefix);
        _prefix = NULL;
     }
}

const Eina_Module *
plugins_request(const char *type,
                const char *name)
{
   Eina_Stringshare *shr;
   Eina_Module *mod = NULL;
   char path[PATH_MAX];
   int bytes;
   Eina_Bool chk;

   if (main_in_tree_is())
     {
        bytes = snprintf(path, sizeof(path), "%s/modules/%s/%s.so",
                         PACKAGE_BUILD_DIR, type, name);
     }
   else
     {
        bytes = snprintf(path, sizeof(path), "%s/war2edit/%s/%s/%s.so",
                         eina_prefix_lib_get(_prefix), type, PACKAGE_VERSION, name);
     }
   path[sizeof(path) - 1] = '\0';

   shr = eina_stringshare_add_length(path, bytes);
   if (EINA_UNLIKELY(!shr))
     {
        CRI("Failed to create stringshare");
        goto fail;
     }

   mod = eina_hash_find(_plugins, shr);
   if (mod)
     {
        DBG("Plugin %s is already registered", shr);
        eina_stringshare_del(shr);
        return mod;
     }

   mod = eina_module_new(shr);
   if (EINA_UNLIKELY(!mod))
     {
        CRI("Failed to create plugin for path \"%s\"", shr);
        goto fail;
     }

   chk = eina_module_load(mod);
   if (EINA_UNLIKELY(!chk))
     {
        CRI("Failed to load module (path = %s)", shr);
        goto fail;
     }

   chk = eina_hash_direct_add(_plugins, shr, mod);
   if (EINA_UNLIKELY(!chk))
     {
        ERR("Failed to add module %p in hash for key \"%s\"", mod, shr);
        goto fail;
     }

   return mod;
fail:
   if (mod) eina_module_free(mod);
   if (shr) eina_stringshare_del(shr);
   return NULL;
}

Plugin_Generator_Func
plugins_generator_func_get(const Eina_Module *mod)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(mod, NULL);
   return eina_module_symbol_get(mod, PLUGIN_GENERATOR_FUNC_SYMBOL);
}

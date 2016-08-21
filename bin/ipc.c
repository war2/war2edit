/*
 * ipc.c
 *
 * Copyright (c) 2015 - 2016 Jean Guyomarc'h
 */

#include "war2edit.h"

#define HANDLERS_COUNT 4u

static Eina_Bool _disable = EINA_FALSE;
static Ecore_Event_Handler *_handlers[HANDLERS_COUNT];
static unsigned int _spawns = 0u;
static Ipc_Cb _spawns_cb = NULL;
static void *_spawns_cb_data = NULL;

static Eina_Bool
_notify_cb(void *data  EINA_UNUSED,
           int   event,
           void *info)
{
   if (event == ECORE_EXE_EVENT_ADD)
     {
        Ecore_Exe_Event_Add *add = info;
        INF("Starting PID: %u, command: %s",
            ecore_exe_pid_get(add->exe),
            ecore_exe_cmd_get(add->exe));
        _spawns++;
     }
   else if (event == ECORE_EXE_EVENT_DEL)
     {
        Ecore_Exe_Event_Del *del = info;
        INF("Exiting PID: %u with code %i", del->pid, del->exit_code);
        _spawns--;
     }
   if (_spawns_cb) _spawns_cb(_spawns_cb_data, _spawns);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_run_cb(void *udat  EINA_UNUSED,
        int   event,
        void *info)
{
   Ecore_Exe_Event_Data *data = info;
   Ecore_Exe_Event_Data_Line *l = data->lines;

   INF("Incoming data (%s) from PID: %u",
       (event == ECORE_EXE_EVENT_DATA) ? "stdout" : "stderr",
       ecore_exe_pid_get(data->exe));

   while ((l++) != NULL)
     printf("%s\n", l->line);

   return ECORE_CALLBACK_PASS_ON;
}


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
ipc_init(void)
{
   struct {
      int                       event;
      Ecore_Event_Handler_Cb    func;
   } handlers[] = {
        { ECORE_EXE_EVENT_ADD,   _notify_cb },
        { ECORE_EXE_EVENT_DEL,   _notify_cb },
        { ECORE_EXE_EVENT_DATA,  _run_cb },
        { ECORE_EXE_EVENT_ERROR, _run_cb }
   };
   unsigned int i;

   if (_disable) return EINA_TRUE;

   for (i = 0; i < HANDLERS_COUNT; ++i)
     {
        _handlers[i] = ecore_event_handler_add(handlers[i].event,
                                               handlers[i].func,
                                               NULL);
        if (EINA_UNLIKELY(!_handlers[i]))
          {
             CRI("Failed to create event handler <%u,%i>", i, handlers[i].event);
             ipc_shutdown();
             return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

void
ipc_shutdown(void)
{
   unsigned int i;

   if (!_disable)
     {
        for (i = 0; i < HANDLERS_COUNT; ++i)
          if (_handlers[i])
            ecore_event_handler_del(_handlers[i]);
     }
}

unsigned int
ipc_spawns_count(void)
{
   return _spawns;
}

Ipc_Cb
ipc_spawns_cb_set(Ipc_Cb      cb,
                  const void *data)
{
   const Ipc_Cb old = _spawns_cb;

   _spawns_cb = cb;
   _spawns_cb_data = (void *)data;

   return old;
}

void
ipc_disable(void)
{
   _disable = EINA_TRUE;
}

Eina_Bool
ipc_disabled_get(void)
{
   return _disable;
}

/*
 * ipc.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef __IPC_H__
#define __IPC_H__

typedef void (*Ipc_Cb)(void *data, unsigned int spawns);

Eina_Bool ipc_init(void);
void ipc_shutdown(void);
unsigned int ipc_spawns_count(void);
Ipc_Cb ipc_spawns_cb_set(Ipc_Cb cb, const void *data);
void ipc_disable(void);
Eina_Bool ipc_disabled_get(void);

#endif /* ! __IPC_H__ */


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

#define SNAPSHOT_MAX 16

typedef struct
{
   EINA_INLIST;

   uint8_t *mem;
   size_t   size;
} Snapshot;

static Snapshot *
snapshot_new(uint8_t *buf,
             size_t   size)
{
   Snapshot *shot;

   shot = malloc(sizeof(*shot) + size);
   if (EINA_UNLIKELY(!shot))
     {
        CRI("Failed to allocate memory");
        return NULL;
     }

   INF("Size is %zu, sizeof shot is %zu", size, sizeof(*shot));
   shot->mem = (uint8_t *)shot + sizeof(*shot);
   INF("shot is %p, shot->mem is %p", shot, shot->mem);
   memcpy(shot->mem, buf, size);
   shot->size = size;
   return shot;
}

static void
snapshot_free(Snapshot *shot)
{
   WRN("Freeing snapshot %p", shot);
   free(shot);
}

static Eina_Bool
_snapshot_delayed_cb(void *data)
{
   Editor *const ed = data;
   Snapshot *shot;

   DBG("Doing snapshot");
   snapshot_force_push(ed);
   if (ed->snapshot.redos)
     {
        INF("Purging old redos");
        EINA_INLIST_FREE(ed->snapshot.redos, shot)
          {
             ed->snapshot.redos = eina_inlist_remove(ed->snapshot.redos,
                                                     EINA_INLIST_GET(shot));
             snapshot_free(shot);
          }
     }
   ed->snapshot.requests = 0;

   // TODO ENABLE undo menu

   return ECORE_CALLBACK_CANCEL;
}

Eina_Bool
snapshot_force_push(Editor *ed)
{
   lzma_stream stream = LZMA_STREAM_INIT;
   lzma_ret ret;
   size_t size = 0;

   ret = lzma_easy_encoder(&stream, 1, LZMA_CHECK_CRC64);
   if (EINA_UNLIKELY(ret != LZMA_OK))
     {
        CRI("Failed to initialize LZMA stream");
        return EINA_FALSE;
     }

   /*
    * FIXME use dynamic buffer to avoid overflowing
    */
   stream.next_in = (const uint8_t *)(ed->cells[0]);
   stream.avail_in = sizeof(Cell) * ed->pud->map_w * ed->pud->map_h;
   stream.next_out = ed->snapshot.buffer;
   stream.avail_out = ed->snapshot.buf_len;

   ret = lzma_code(&stream, LZMA_RUN);
   if (stream.avail_out == 0)
     {
        CRI("No more memory available for compression");
        goto fail;
     }
   else if (ret == LZMA_OK)
     {
        /* Done, say to lzma we are finished */
        ret = lzma_code(&stream, LZMA_FINISH);
        if ((ret == LZMA_OK) || (ret == LZMA_STREAM_END))
          {
             size = ed->snapshot.buf_len - stream.avail_out;
             INF("Created compressed image with size %zu bytes", size);
          }
        else
          {
             CRI("Something went wrong: 0x%x", ret);
             goto fail;
          }
     }
   else
     {
        CRI("Something went wrong: 0x%x", ret);
        goto fail;
     }
   lzma_end(&stream);

   /* There is a snapshot */
   if (size != 0)
     {
        Snapshot *shot;
        unsigned int count;

        shot = snapshot_new(ed->snapshot.buffer, size);
        if (EINA_UNLIKELY(!shot))
          {
             CRI("Failed to create snapshot");
             return EINA_FALSE;
          }

        count = eina_inlist_count(ed->snapshot.items);
        if (count >= SNAPSHOT_MAX)
          {
             Eina_Inlist *l;

             DBG("Too many snapshot snapshots. Removing the oldest.");
             l = eina_inlist_first(ed->snapshot.items);
             // XXX???       snapshot_free(EINA_INLIST_CONTAINER_GET(l, Snapshot));
             ed->snapshot.items = eina_inlist_remove(ed->snapshot.items, l);
          }
        ed->snapshot.items = eina_inlist_append(ed->snapshot.items, EINA_INLIST_GET(shot));

        DBG("snapshot count is now %u. New: %p",
            eina_inlist_count(ed->snapshot.items), shot);
     }

   return EINA_TRUE;

fail:
   lzma_end(&stream);
   return EINA_FALSE;
}

Eina_Bool
snapshot_add(Editor *ed)
{
   ed->snapshot.items = NULL;

   /* FIXME meh */
   ed->snapshot.buf_len = (1 << 14); /* 16KiB */
   ed->snapshot.buffer = malloc(ed->snapshot.buf_len);

   // TODO Set UNDO menu to DISABLED

   /* Create the initial snapshot */
   return snapshot_force_push(ed);
}

void
snapshot_del(Editor *ed)
{
   Snapshot *shot;

   free(ed->snapshot.buffer);
   EINA_INLIST_FREE(ed->snapshot.items, shot)
     {
        ed->snapshot.items = eina_inlist_remove(ed->snapshot.items, EINA_INLIST_GET(shot));
        snapshot_free(shot);
     }
}

void
snapshot_push(Editor *ed EINA_UNUSED)
{
   /* Nothing to do */
}

void
snapshot_push_done(Editor *ed)
{
   /*
    * I have the feeling one day there will be synchro plroblem...
    * And this function might help
    */
   if (++ed->snapshot.requests == 1)
     {
        ecore_timer_add(3.0, _snapshot_delayed_cb, ed);
     }
   DBG("Trigerring snapshot");
}

Eina_Bool
snapshot_rollback(Editor *ed,
                  int offset)
{
   unsigned int count;
   int i;
   int magic;
   Eina_Inlist **ptr;
   Eina_Inlist *l;
   Snapshot *shot;
   lzma_stream stream = LZMA_STREAM_INIT;
   lzma_ret ret;
   Eina_Bool redo;

   /*
    * Offset > 0 is used to go back to the future :)
    * That means it is a redo operation: restore a state that has been
    * undone.
    *
    * Offset < 0 is used to rollback
    * That's an undo operation
    *
    * If we rollback ZERO versions, we don't have to do anything
    */
   if (offset > 0)
     {
        ptr = &(ed->snapshot.redos);
        redo = EINA_TRUE;
        magic = 0;
     }
   else if (offset < 0)
     {
        ptr = &(ed->snapshot.items);
        redo = EINA_FALSE;
        magic = 1;
     }
   else
     return EINA_TRUE;

   /* Cannot snapshot when the list is empty */
   count = eina_inlist_count(*ptr);
   DBG("Elements in stack: %u", count);
   if (count <= (unsigned int)magic)
     {
        INF("No elements in stack. Cannot rollback");
        return EINA_FALSE;
     }
   DBG("Snapshots stack with %u elements", count);
   DBG("Offset is %i, abs() -> %i", offset, abs(offset));

   l = eina_inlist_last(*ptr);

   /* Pop compressed blob */
   for (i = 0; i < abs(offset); i++)
     {
        if (count > 1)
          {
             *ptr = eina_inlist_remove(*ptr, l);
          }

        if (!redo)
          {
             ed->snapshot.redos = eina_inlist_append(ed->snapshot.redos, l);
             WRN("Pushing snapshot in redo stack (list %p, shot %p). count is now %u",
                 l, EINA_INLIST_CONTAINER_GET(l, Snapshot),eina_inlist_count(ed->snapshot.redos));
          }

        DBG("Deleting element N-%i", i);
        l = eina_inlist_last(*ptr);
        count--;
     }

   if (count == (unsigned int)magic)
     {
        // TODO Set menu undo to DISABLED
     }

   shot = EINA_INLIST_CONTAINER_GET(l, Snapshot);
   *ptr = eina_inlist_remove(*ptr, l);
   DBG("Using snapshot %p from list %p. There is now %u elements", shot, l, count);

   ret = lzma_stream_decoder(&stream, UINT32_MAX, LZMA_CONCATENATED);
   if (ret != LZMA_OK)
     {
        CRI("Failed to create LZMA decoder");
        return EINA_FALSE;
     }

   stream.avail_in = shot->size;
   stream.next_in = shot->mem;
   stream.avail_out = ed->pud->map_w * ed->pud->map_h * sizeof(Cell);
   stream.next_out = (uint8_t *)(ed->cells[0]);

   ret = lzma_code(&stream, LZMA_RUN);
   if (ret == LZMA_OK)
     {
        ret = lzma_code(&stream, LZMA_FINISH);
        if ((ret == LZMA_OK) || (ret == LZMA_STREAM_END))
          {
             INF("Ok, done");
          }
        else
          {
             CRI("Something went wrong: 0x%x", ret);
          }
     }
   else
     CRI("Something went wrong: 0x%x", ret);
   lzma_end(&stream);

   editor_units_recount(ed);
   editor_units_list_update(ed);
   bitmap_refresh(ed, NULL);
   minimap_reload(ed);
   editor_changed(ed);

   return EINA_TRUE;
}

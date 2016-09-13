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
   free(shot);
}

static inline void
_undo_menu_disabled_set(Editor    *ed,
                        Eina_Bool  disable)
{
   elm_object_item_disabled_set(ed->snapshot.menu_undo, disable);
}

static Eina_Bool
_snapshot_delayed_cb(void *data)
{
   Editor *const ed = data;

   DBG("Doing snapshot");
   snapshot_force_push(ed);
   ed->snapshot.requests = 0;

   _undo_menu_disabled_set(ed, EINA_FALSE);

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

   _undo_menu_disabled_set(ed, EINA_TRUE);

   /* Create the initial snapshot */
   return snapshot_force_push(ed);

}

void
snapshot_del(Editor *ed)
{
//   Snapshot *shot;

   free(ed->snapshot.buffer);
//XXX   EINA_INLIST_FREE(ed->snapshot.items, shot)
//XXX      snapshot_free(shot);
}

void
snapshot_menu_connect(Editor          *ed   EINA_UNUSED,
                  Elm_Object_Item *snapshot EINA_UNUSED,
                  Elm_Object_Item *redo EINA_UNUSED)
{
}

void
snapshot_push(Editor *ed EINA_UNUSED)
{
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

   if (offset >= 0)
     {
        CRI("Rollback means get back in version, forever. "
            "You can advance in a rollback.");
        return EINA_FALSE;
     }

   /* Cannot snapshot when the list is empty */
   count = eina_inlist_count(ed->snapshot.items);
   if (count <= 1)
     {
        DBG("No elements in stack. Cannot snapshot");
        return EINA_FALSE;
     }

   Eina_Inlist *l;
   Snapshot *shot;
   lzma_stream stream = LZMA_STREAM_INIT;
   lzma_ret ret;

   DBG("snapshot with %u elements", count);

   l = eina_inlist_last(ed->snapshot.items);
   /* Pop compressed blob */
   for (i = 0; i < abs(offset); i++)
     {
        ed->snapshot.items = eina_inlist_remove(ed->snapshot.items, l);
        DBG("Deleting element N-%i", i); // FIXME FREE DATA????
        l = eina_inlist_last(ed->snapshot.items);
        count--;
     }

   if (count == 1)
     _undo_menu_disabled_set(ed, EINA_TRUE);

   shot = EINA_INLIST_CONTAINER_GET(l, Snapshot);
   DBG("Using snapshot %p. There is now %u elements", shot, count);

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
   return EINA_TRUE;
}

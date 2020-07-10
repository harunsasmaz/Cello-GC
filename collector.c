#include "collector.h"

static size_t gc_hash(void *ptr) 
{
    return ((uintptr_t)ptr) >> 3;
}

static size_t gc_probe(gc_t* gc, size_t i, size_t h)
{
    long v = i - (h-1);
    if (v < 0) { v = gc->nslots + v; }
    return v;
}

static gc_ptr_t *gc_get_ptr(gc_t *gc, void *ptr) 
{
    size_t i, j, h;
    i = gc_hash(ptr) % gc->nslots; j = 0;
    while (1) 
    {
        h = gc->items[i].hash;
        if (h == 0 || j > gc_probe(gc, i, h)) return NULL;
        if (gc->items[i].ptr == ptr) return &gc->items[i];
        i = (i+1) % gc->nslots; 
        j++;
    }
    return NULL;
}

static void gc_add_ptr(gc_t *gc, void *ptr, size_t size, 
    int flags, void(*dtor)(void*)) 
{
    gc_ptr_t item, tmp;
    size_t h, p, i, j;

    i = gc_hash(ptr) % gc->nslots; j = 0;

    item.ptr = ptr;
    item.flags = flags;
    item.size = size;
    item.hash = i+1;
    item.dtor = dtor;

    while (1) 
    {
        h = gc->items[i].hash;
        if (h == 0) 
        { 
            gc->items[i] = item; 
            return; 
        }

        if (gc->items[i].ptr == item.ptr) return; 
        p = gc_probe(gc, i, h);

        if (j >= p) 
        {
            tmp = gc->items[i];
            gc->items[i] = item;
            item = tmp;
            j = p;
        }

        i = (i+1) % gc->nslots; 
        j++;
    }
}

static void gc_rem_ptr(gc_t *gc, void *ptr) 
{
    size_t i, j, h, nj, nh;

    if (gc->nitems == 0) { return; }

    for (i = 0; i < gc->nfrees; i++) {
    if (gc->frees[i].ptr == ptr) { gc->frees[i].ptr = NULL; }
    }

    i = gc_hash(ptr) % gc->nslots; j = 0;

    while (1) {
        h = gc->items[i].hash;
        if (h == 0 || j > gc_probe(gc, i, h)) return;

        if (gc->items[i].ptr == ptr) 
        {
            memset(&gc->items[i], 0, sizeof(gc_ptr_t));
            j = i;
            while (1) { 
                nj = (j+1) % gc->nslots;
                nh = gc->items[nj].hash;
                if (nh != 0 && gc_probe(gc, nj, nh) > 0) {
                    memcpy(&gc->items[ j], &gc->items[nj], sizeof(gc_ptr_t));
                    memset(&gc->items[nj],              0, sizeof(gc_ptr_t));
                    j = nj;
                } else {
                    break;
                }  
            }
            gc->nitems--;
            return;
        }
        i = (i+1) % gc->nslots; 
        j++;
    }
}


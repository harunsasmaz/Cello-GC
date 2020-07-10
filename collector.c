#include "collector.h"

#define GC_PRIMES_COUNT 24

static const size_t gc_primes[GC_PRIMES_COUNT] = {
  0,       1,       5,       11,
  23,      53,      101,     197,
  389,     683,     1259,    2417,
  4733,    9371,    18617,   37097,
  74093,   148073,  296099,  592019,
  1100009, 2200013, 4400021, 8800019
};

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

static size_t gc_ideal_size(gc_t* gc, size_t size) 
{
    size_t i, last;
    size = (size_t)((double)(size+1) / gc->loadfactor);

    for (i = 0; i < GC_PRIMES_COUNT; i++)
        if (gc_primes[i] >= size) return gc_primes[i]; 

    last = gc_primes[GC_PRIMES_COUNT - 1];

    for (i = 0;; i++)
        if (last * i >= size) return last * i;

    return 0;
}

static int gc_rehash(gc_t* gc, size_t new_size)
{
    size_t i;
    gc_ptr_t *old_items = gc->items;
    size_t old_size = gc->nslots;

    gc->nslots = new_size;
    gc->items = calloc(gc->nslots, sizeof(gc_ptr_t));

    if (gc->items == NULL) {
        gc->nslots = old_size;
        gc->items = old_items;
        return 0;
    }

    for (i = 0; i < old_size; i++) {
        if (old_items[i].hash != 0) {
            gc_add_ptr(gc, 
            old_items[i].ptr,   old_items[i].size, 
            old_items[i].flags, old_items[i].dtor);
        }
    }

    free(old_items);    
    return 1;
}

static int gc_resize_more(gc_t *gc) {
    size_t new_size = gc_ideal_size(gc, gc->nitems);  
    size_t old_size = gc->nslots;
    return (new_size > old_size) ? gc_rehash(gc, new_size) : 1;
}

static int gc_resize_less(gc_t *gc) {
    size_t new_size = gc_ideal_size(gc, gc->nitems);  
    size_t old_size = gc->nslots;
    return (new_size < old_size) ? gc_rehash(gc, new_size) : 1;
}


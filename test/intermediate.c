#include "../collector.h"

typedef struct object {
    char *b;  
    int a;
    struct object *first;
    struct object *second;
    struct object **others;
} Object;

static gc_t gc;

static void waste(void) {
    Object *x = gc_alloc(&gc, sizeof(Object));
}

static Object *object_new(void) 
{
    Object *x = gc_alloc(&gc, sizeof(Object));
    waste();
    x->a = 1;
    x->b = gc_calloc(&gc, 1, 100);
    x->b[0] = 'a';
    x->first = gc_alloc(&gc, sizeof(Object));
    waste();
    x->second = gc_alloc(&gc, sizeof(Object));
    waste();
    x->others = NULL;
    waste();
    return x;
}

static void object_resize(Object *x, int num) 
{
    x->others = gc_realloc(&gc, x->others, sizeof(Object*) * num);
    waste();
}

static void example_function(int depth) 
{
    Object *x = object_new();
    Object *y = object_new();

    object_resize(x, 100);
    object_resize(y, 50);

    object_resize(x, 75);
    object_resize(y, 75);

    if (depth < 10) {example_function(depth+1); }

    x->others[10] = object_new();
    y->others[10] = object_new();

    x->others[25] = object_new();
    object_resize(x->others[25], 30);
}

int main(int argc, char **argv) 
{
    gc_start(&gc, &argc);
    example_function(0);
    gc_stop(&gc);
    return 0;
}
#include "../collector.h"

static gc_t gc;

static void example_function() {
    void *memory = gc_alloc(&gc, 1024);
}

int main(int argc, char **argv) 
{
    gc_start(&gc, &argc);
    example_function();
    gc_stop(&gc);
    return 0;
}
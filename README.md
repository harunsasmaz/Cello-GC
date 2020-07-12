# Cello-GC

A mark-sweep style, conservative and thread local garbage collector based on [Cello Garbage Collector](http://libcello.org/learn/garbage-collection).

It provides automatical removal of pointers allocated by tgc_alloc() and friends of those pointers whenever they become unreachable.

<h3> When is a pointer reachable? </h3>

* A pointer points to it, inside memory allocated by <code>tgc_alloc</code> and friends.

* A pointer points to it, located on the stack at least one function call deeper than the call to <code>tgc_start</code>

> Otherwise, that memory segment is considered as <i>unreachable</i>

<h3> What does not qualify a memory segment as reachable? </h3>

* a pointer points to an address inside of it, but not at the start of it.

* a pointer points to it from inside the static data segment.

* a pointer points to it from memory allocated by malloc, calloc, realloc or any other non-tgc allocation methods.

* a pointer points to it from a different thread.

* a pointer points to it from any other unreachable location.


## API

```c
void tgc_start(tgc_t *gc, void *stk);
```

Start the garbage collector on the current thread, beginning at the stack 
location given by the `stk` variable.

```c
void tgc_stop(tgc_t *gc);
```

Stop the garbage collector and free its internal memory.

```c
void tgc_run(tgc_t *gc);
```

Run an iteration of the garbage collector, freeing any unreachable memory.

```c
void tgc_pause(tgc_t *gc);
void tgc_resume(tgc_t *gc);
```

Pause or resume the garbage collector. While paused the garbage collector will
not run during any allocations made.

```c
void *tgc_alloc(gc_t *gc, size_t size);
```

Allocate memory via the garbage collector to be automatically freed once it
becomes unreachable.

```c
void *tgc_calloc(gc_t *gc, size_t num, size_t size);
```

Allocate memory via the garbage collector and initalise it to zero.

```c
void *tgc_realloc(gc_t *gc, void *ptr, size_t size);
```

Reallocate memory allocated by the garbage collector.

```c
void tgc_free(gc_t *gc, void *ptr);
```

Manually free an allocation made by the garbage collector. Runs any destructor if registered.

```c
void *tgc_alloc_opt(tgc_t *gc, size_t size, int flags, void(*dtor)(void*));
```

Allocate memory via the garbage collector with the given flags and destructor.

```c
void *tgc_calloc_opt(tgc_t *gc, size_t num, size_t size, int flags, void(*dtor)(void*));
```

Allocate memory via the garbage collector with the given flags and destructor and initalise to zero.

```c
void tgc_set_dtor(tgc_t *gc, void *ptr, void(*dtor)(void*));
```

Register a destructor function to be called after the memory allocation `ptr` becomes unreachable, and just before it is freed by the garbage collector.

```c
void tgc_set_flags(tgc_t *gc, void *ptr, int flags);
```

Set the flags associated with a memory allocation, for example the value `TGC_ROOT` can be used to specify that an allocation is a garbage collection root.

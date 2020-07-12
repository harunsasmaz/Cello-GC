# Cello-GC

A mark-sweep style, conservative and thread local garbage collector based on [Cello Garbage Collector](http://libcello.org/learn/garbage-collection).

It provides automatical removal of pointers allocated by gc_alloc() and friends of those pointers whenever they become unreachable.

* This garbage collector also supports destructors and flags.

<h4> What is a destructor? </h4>

> A destructor is a function that is called when an unreachable memory is freed by garbage collector. This functions can be set when a memory is allocated by using memory allocation functions defined in this API.

<h3> Resources for Mark-and-Sweep memory allocation algorithm </h3>

* [GeeksforGeeks](https://www.geeksforgeeks.org/mark-and-sweep-garbage-collection-algorithm/).

* [Animesh Gaitonde's Medium Blog](https://medium.com/@animeshgaitonde/garbage-collection-algorithm-mark-sweep-ed874272702d).

* [Dmitri Soshnikov's Blog](http://dmitrysoshnikov.com/compilers/writing-a-mark-sweep-garbage-collector/).

<h3> When is a pointer reachable? </h3>

* A pointer points to it, inside memory allocated by <code>gc_alloc</code> and friends.

* A pointer points to it, located on the stack at least one function call deeper than the call to <code>gc_start</code>

> Otherwise, that memory segment is considered as <i>unreachable</i>

<h3> What does not qualify a memory segment as reachable? </h3>

* a pointer points to an address inside of it, but not at the start of it.

* a pointer points to it from inside the static data segment.

* a pointer points to it from memory allocated by malloc, calloc, realloc or any other non-gc allocation methods.

* a pointer points to it from a different thread.

* a pointer points to it from any other unreachable location.


## API

```c
void gc_start(gc_t *gc, void *stk);
```

Start the garbage collector on the current thread, beginning at the stack 
location given by the `stk` variable.

```c
void gc_stop(gc_t *gc);
```

Stop the garbage collector and free its internal memory.

```c
void gc_run(gc_t *gc);
```

Run an iteration of the garbage collector, freeing any unreachable memory.

```c
void gc_pause(gc_t *gc);
void gc_resume(gc_t *gc);
```

Pause or resume the garbage collector. While paused the garbage collector will not run during any allocations made.

```c
void *gc_alloc(gc_t *gc, size_t size);
```

Allocate memory via the garbage collector to be automatically freed once it
becomes unreachable.

```c
void *gc_calloc(gc_t *gc, size_t num, size_t size);
```

Allocate memory via the garbage collector and initalise it to zero.

```c
void *gc_realloc(gc_t *gc, void *ptr, size_t size);
```

Reallocate memory allocated by the garbage collector.

```c
void gc_free(gc_t *gc, void *ptr);
```

Manually free an allocation made by the garbage collector. Runs any destructor if registered.

```c
void *gc_alloc_opt(gc_t *gc, size_t size, int flags, void(*dtor)(void*));
```

Allocate memory via the garbage collector with the given flags and destructor.

```c
void *gc_calloc_opt(gc_t *gc, size_t num, size_t size, int flags, void(*dtor)(void*));
```

Allocate memory via the garbage collector with the given flags and destructor and initalise to zero.

```c
void gc_set_dtor(gc_t *gc, void *ptr, void(*dtor)(void*));
```

Register a destructor function to be called after the memory allocation `ptr` becomes unreachable, and just before it is freed by the garbage collector.

```c
void gc_set_flags(gc_t *gc, void *ptr, int flags);
```

Set the flags associated with a memory allocation, for example the value `GC_ROOT` can be used to specify that an allocation is a garbage collection root.

## Notes

* Cello-GC does not work when incrementing pointers.

* Cello-GC does not work when compiler optimisations are enabled.
  * Such as inline functions.

* Cello-GC does not work with <code>setjmp</code> and <code>longjmp</code>.

## Acknowlegments

This repository is created for practical purposes under my personal interests in compiler design and interpeters. There is no commercial use of this code and I am aware of license agreements. I want to give special thanks, and of course, credit for the work here to both:

* [libCello](http://libcello.org/home)
* [Daniel Holden](https://github.com/orangeduck)
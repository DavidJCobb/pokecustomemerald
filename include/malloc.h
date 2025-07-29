#ifndef GUARD_ALLOC_H
#define GUARD_ALLOC_H


#define FREE_AND_SET_NULL(ptr)          \
{                                       \
    Free(ptr);                          \
    ptr = NULL;                         \
}

#define TRY_FREE_AND_SET_NULL(ptr) if (ptr != NULL) FREE_AND_SET_NULL(ptr)

#define HEAP_SIZE 0x1C000
extern u8 gHeap[HEAP_SIZE];

#if __GNUC__
   #if __STDC_VERSION__ < 202311L
      #define MALLOC_ANNOTATION __attribute__((malloc, malloc(Free, 1), alloc_size(1)))
   #else
      #define MALLOC_ANNOTATION [[gnu::malloc, gnu::malloc(Free, 1), gnu::alloc_size(1)]]
   #endif
#else
   #define MALLOC_ANNOTATION
#endif

void Free(void *pointer);
MALLOC_ANNOTATION void *Alloc(u32 size);
MALLOC_ANNOTATION void *AllocZeroed(u32 size);
void InitHeap(void *heapStart, u32 heapSize);

#undef MALLOC_ANNOTATION

#endif // GUARD_ALLOC_H

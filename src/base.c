#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base.h"

static uintptr_t align_memory_forward(uintptr_t address, size_t align) {
    // CHECKS IF ALIGN IS POWERS OF TWO (1, 2, 4, 8, 16....)
    assert((align & (align - 1)) == 0);

    uintptr_t u_align = align;  
    uintptr_t mod = address & (u_align - 1); // same as (address % align), but faster as power of 2

    if (mod == 0)
        return address;

    address += align - mod;

    return address;
};

void arena_init(Arena* arena, void* buffer, size_t total_size) {
    arena->buffer = (unsigned char*)buffer;
    arena->total_size = total_size;
    arena->previous_offset = 0;
    arena->current_offset = 0;
};


void* arena_allocate_align(Arena* arena, size_t size, size_t align) {
    if (arena == NULL) {
        fprintf(stderr, "arena provided is not valid");
        return NULL;
    };

    uintptr_t new_allocation = align_memory_forward((uintptr_t)arena->buffer + arena->current_offset, align);
    if (new_allocation + size > (uintptr_t)arena->buffer + arena->total_size) {
        fprintf(stderr, "don't have enough memory in arena");
        return NULL;
    };

    arena->current_offset = new_allocation - (uintptr_t)arena->buffer + size;
    return (void*)new_allocation;
};

void* arena_allocate(Arena* arena, size_t size) {
    return arena_allocate_align(arena, size, 2 * sizeof(void*));
};

void arena_free_all(Arena *arena) {
	arena->current_offset = 0;
	arena->previous_offset = 0;
}


void arena_release(Arena* arena) {
    free(arena);
};

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "base.h"

internal bool is_power_of_two(uintptr_t x) {
	return (x & (x-1)) == 0;
}

internal uintptr_t align_forward(uintptr_t ptr, size_t align) {
	uintptr_t p, a, modulo;

	assert(is_power_of_two(align));

	p = ptr;
	a = (uintptr_t)align;
	modulo = p & (a-1);

	if (modulo != 0) { p += a - modulo; }
	return p;
}

void arena_init(Arena *a, void *backing_buffer, size_t backing_buffer_length) {
	a->buf = (unsigned char *)backing_buffer;
	a->buf_len = backing_buffer_length;
	a->curr_offset = 0;
	a->prev_offset = 0;
}


void *arena_alloc_align(Arena *a, size_t size, size_t align) {
	uintptr_t curr_ptr = (uintptr_t)a->buf + (uintptr_t)a->curr_offset;
	uintptr_t offset = align_forward(curr_ptr, align);
	offset -= (uintptr_t)a->buf;
	if (offset+size <= a->buf_len) {
		void *ptr = &a->buf[offset];
		a->prev_offset = offset;
		a->curr_offset = offset+size;
		memset(ptr, 0, size);
		return ptr;
	}
	return NULL;
}

void *arena_alloc(Arena *a, size_t size) {
	return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

void arena_free(Arena *a, void *ptr) { }

void *arena_resize_align(Arena *a, void *old_memory, size_t old_size, size_t new_size, size_t align) {
	unsigned char *old_mem = (unsigned char *)old_memory;

	assert(is_power_of_two(align));

	if (old_mem == NULL || old_size == 0) {
		return arena_alloc_align(a, new_size, align);
	} else if (a->buf <= old_mem && old_mem < a->buf+a->buf_len) {
		if (a->buf+a->prev_offset == old_mem) {
			a->curr_offset = a->prev_offset + new_size;
			if (new_size > old_size) {
				memset(&a->buf[a->curr_offset], 0, new_size-old_size);
			}
			return old_memory;
		} else {
			void *new_memory = arena_alloc_align(a, new_size, align);
			size_t copy_size = old_size < new_size ? old_size : new_size;
			memmove(new_memory, old_memory, copy_size);
			return new_memory;
		}

	} else {
		assert(0 && "Memory is out of bounds of the buffer in this arena");
		return NULL;
	}
}

void *arena_resize(Arena *a, void *old_memory, size_t old_size, size_t new_size) {
	return arena_resize_align(a, old_memory, old_size, new_size, DEFAULT_ALIGNMENT);
}

void arena_free_all(Arena *a) {
	a->curr_offset = 0;
	a->prev_offset = 0;
}

Temp_Arena_Memory temp_arena_memory_begin(Arena *a) {
	Temp_Arena_Memory temp;
	temp.arena = a;
	temp.prev_offset = a->prev_offset;
	temp.curr_offset = a->curr_offset;
	return temp;
}

void temp_arena_memory_end(Temp_Arena_Memory temp) {
	temp.arena->prev_offset = temp.prev_offset;
	temp.arena->curr_offset = temp.curr_offset;
}

String8 str_init(size_t len, Arena* arena) {
    String8 s = { .len = len, .data = (char*)arena_alloc(arena, len), };
    s.data[len] = 0;
    return s;
}
String8 str_from_cstr(const char* cstr, Arena* arena) {
    String8 s;
    s.len = strlen(cstr);
    s.data = (char*)arena_alloc(arena, s.len + 1);  // No null terminator included
    if (s.data != NULL) memcpy(s.data, cstr, s.len);
    return s;
};
String8 str_concat(String8 s1, String8 s2, Arena *a) {
    size_t len = s1.len + s2.len;
    String8 s = str_init(len, a);
    memcpy(s.data, s1.data, s1.len);
    memcpy(&s.data[s1.len], s2.data, s2.len);
    return s;
}
String8 str_substring(String8 s, size_t start, size_t end, Arena *a) {
    String8 r = {0};
    if (end <= s.len && start < end) {
        r = str_init(end - start, a);
        memcpy(r.data, &s.data[start], r.len);
    }
    return r;
}
bool str_contains(String8 haystack, String8 needle) {
    bool found = false;
    for (size_t i = 0, j = 0; i < haystack.len && !found; i += 1) {
        while (haystack.data[i] == needle.data[j]) {
            j += 1;
            i += 1;
            if (j == needle.len) {
                found = true;
                break;
            }
        }
    }
    return found;
}
size_t str_index_of(String8 haystack, String8 needle) {
    for (size_t i = 0; i < haystack.len; i += 1) {
        size_t j = 0;
        size_t start = i;
        while (haystack.data[i] == needle.data[j]) {
            j += 1;
            i += 1;
            if (j == needle.len) {
                return start;
            }
        }
    }
    return (size_t)-1;
}
String8 str_substring_view(String8 haystack, String8 needle) {
    String8 r = {0};
    size_t start_index = str_index_of(haystack, needle);
    if (start_index < haystack.len) {
        r.data = &haystack.data[start_index];
        r.len = needle.len;
    }
    return r;
}
bool str_equal(String8 a, String8 b) {
    if (a.len != b.len) {
        return false;
    }
    return memcmp(a.data, b.data, a.len) == 0;
}
String8 str_view(String8 s, size_t start, size_t end) {
    if (end < start || end - start > s.len) {
        return (String8){0};
    }
    return (String8){end - start, s.data + start};
}

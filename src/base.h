#ifndef BASE 
#define BASE 

/*
 *   CONTEXT CRACKING SECTION
 */
// INITIAL OPERTING SYSTEM MACRO SETUP 
#if defined(__APPLE__) && defined(__MACH__)
#   define OS_MAC 1
#endif
#if defined(__gnu_linux__)
#   define OS_LINUX 1
#endif
#if defined(__FreeBSD__)
#   define OS_FREEBSD 1
#endif
#if defined(_WIN64) || defined(_WIN32) || defined(_WIN16)
#   define OS_WINDOWS 1
#endif

// INITIAL COMPILER MACRO SETUP
#if defined(__clang__)
#   define COMPILER_CLANG 1
#   define CLANG_VERSION __clang_version__
#endif
#if defined(__clang__) && defined(__apple_build_version__)
#   define COMPILER_APPLE_CLANG 1
#endif
#if defined(__GNUC__)
#   define COMPILER_GCC 1 
#   define GCC_VERSION __GNUC_PATCHLEVEL__
#endif
#if defined (__MINGW32__) || defined(__MINGW64__)
#   define COMPILER_MINGW 1
#   define MINGW_VERSION __MINGW64_VERSION_MAJOR
#endif
#if defined (_MSC_VER)
#   define COMPILER_MSV 1 
#   define MSV_VERSION _MSC_VER
#endif


// INITIAL COMPILER MACRO SETUP
#if defined(__amd64) || defined(_M_AMD64)
#   define ARCH_x86 1
#endif
#if defined(__i386) || defined(_M_IX86)
#   define ARCH_x64 1 
#endif
#if defined(__aarch64__)
#   define ARCH_ARM64 1
#endif
#if defined(__arm__)
#   define ARCH_ARM 1 
#endif

// SETTING ALL UNDEFINED MACROS TO ZEROS 
#if !defined (OS_MAC) 
#   define OS_MAC 0 
#endif
#if !defined (OS_LINUX) 
#   define OS_LINUX 0 
#endif
#if !defined (OS_FREEBSD) 
#   define OS_FREEBSD 0 
#endif
#if !defined (OS_WINDOWS) 
#   define OS_WINDOWS 0 
#endif

#if !defined (COMPILER_CLANG) 
#   define COMPILER_CLANG 0 
#endif
#if !defined (COMPILER_APPLE_CLANG) 
#   define COMPILER_APPLE_CLANG 0 
#endif
#if !defined (COMPILER_GCC) 
#   define COMPILER_GCC 0 
#endif
#if !defined (COMPILER_MINGW) 
#   define COMPILER_MINGW 0 
#endif
#if !defined (COMPILER_MSV) 
#   define COMPILER_MSV 0 
#endif

#if !COMPILER_CLANG
#   define CLANG_VERSION 0
#endif
#if !COMPILER_GCC
#   define GCC_VERSION 0
#endif
#if !COMPILER_MSV
#   define MSV_VERSION 0
#endif
#if !COMPILER_MINGW
#   define MINGW_VERSION 0
#endif

#if !defined (ARCH_x64) 
#   define ARCH_x64 0 
#endif
#if !defined (ARCH_x86) 
#   define ARCH_x86 0 
#endif
#if !defined (ARCH_ARM) 
#   define ARCH_ARM 0 
#endif
#if !defined (ARCH_ARM64) 
#   define ARCH_ARM64 0 
#endif


/*
 *   HELPERS SECTIONS
 */
#define MIN(a,b)       (((a)<(b))?(a):(b))
#define MAX(a,b)       (((a)>(b))?(a):(b))
#define CLAMP(a,b,c)   MAX(a,MIN(b,c))
#define CLAMP_TOP(a,b) MIN(a,b)
#define CLAMP_BOT(a,b) MAX(a,b)

#include <string.h>
#define MEM_ZERO(p,s) memset((p),0,(s));
#define MEM_ARRAY_ZERO(p) MEM_ZERO(p,sizeof(p))

/*
 *  BASIC TYPES
 */
#include <stdint.h>
typedef int8_t   S8;
typedef int16_t  S16;
typedef int32_t  S32;
typedef int64_t  S64;
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef float    F32;
typedef double   F64;
typedef U8       B8;

#define internal static 
#define global   static 

/*
 * LINKED LIST
 */
#define DLLPushBack_NP(f,l,n,next,prev) ((f)==0?\
    ((f)=(l)=(n),(n)->next=(n)->prev=0):\
    ((n)->prev=(l),(l)->next=(n),(l)=(n),(n)->next=0))
#define DLLPushBack(f,l,n) DLLPushBack_NP(f,l,n,next,prev)

#define DLLPushFront(f,l,n) DLLPushBack_NP(l,f,n,prev,next)

#define DLLRemove_NP(f,l,n,next,prev) ((f)==(n)?\
    ((f)==(l)?\
    ((f)=(l)=(0)):\
    ((f)=(f)->next,(f)->prev=0)):\
    (l)==(n)?\
    ((l)=(l)->prev,(l)->next=0):\
    ((n)->next->prev=(n)->prev,\
    (n)->prev->next=(n)->next))
#define DLLRemove(f,l,n) DLLRemove_NP(f,l,n,next,prev)


/*
 * ARENA
 *
 */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2*sizeof(void *))
#endif

typedef struct Arena Arena;
struct Arena {
	unsigned char *buf;
	size_t         buf_len;
	size_t         prev_offset;
	size_t         curr_offset;
};
void arena_init(Arena *a, void *backing_buffer, size_t backing_buffer_length);
void *arena_alloc_align(Arena *a, size_t size, size_t align);
void *arena_alloc(Arena *a, size_t size);
void arena_free(Arena *a, void *ptr);
void *arena_resize_align(Arena *a, void *old_memory, size_t old_size, size_t new_size, size_t align);
void *arena_resize(Arena *a, void *old_memory, size_t old_size, size_t new_size);
void arena_free_all(Arena *a);

typedef struct Temp_Arena_Memory Temp_Arena_Memory;
struct Temp_Arena_Memory { Arena *arena; size_t prev_offset; size_t curr_offset; };
Temp_Arena_Memory temp_arena_memory_begin(Arena *a);
void temp_arena_memory_end(Temp_Arena_Memory temp);


#define DeferLoop(begin, end)        for(int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))

// GENREIC STACKS
#define DECLARE_STACK(name, type) typedef struct {S16 top; type items[4];} type##Stack;\
    internal bool is_##name##_stack_empty(type##Stack* stack) { return (stack->top == -1); };\
    internal type* get_##name##_stack_top(type##Stack* stack) { return (stack->top == -1) ? NULL: &stack->items[stack->top]; };\
    internal type* pop_##name##_stack(type##Stack* stack) { return (stack->top == -1) ? NULL: &stack->items[stack->top--]; };\
    internal void push_##name##_stack(type##Stack* stack, type item) { stack->items[++stack->top] = item; };\



typedef struct { size_t len; char *data; } String8;
#define String8(x) (String8){strlen(x), x}
String8 str_init(size_t len, Arena* arena);
String8 str_from_cstr(const char* string, Arena* arena);
String8 str_concat(String8 s1, String8 s2, Arena *a);
String8 str_substring(String8 s, size_t start, size_t end, Arena *a);
bool str_contains(String8 haystack, String8 needle);
size_t str_index_of(String8 haystack, String8 needle);
String8 str_substring_view(String8 haystack, String8 needle);
bool str_equal(String8 a, String8 b);
String8 str_view(String8 s, size_t start, size_t end);


#endif

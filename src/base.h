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



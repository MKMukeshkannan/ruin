#include "base.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

typedef U32 ruin_Id; 


typedef enum { RUIN_SIZEKIND_NULL, RUIN_SIZEKIND_TEXTCONTENT, RUIN_SIZEKIND_PARENTPERCENTAGE, RUIN_SIZEKIND_CHILDRENSUM } ruin_SizeKind;
typedef enum { RUIN_WIDGETFLAGS_CLICKABLE = (1<<0), RUIN_WIDGETFLAGS_HOVERABLE = (1<<1) }                                 ruin_WidgetFlags;
typedef enum { ruin_WINDOWFLAGS_DRAGABLE }                                                                                ruin_WindowFlags;
        enum { RUIN_AXISX, RUIN_AXISY, RUIN_AXISCOUNT, };

typedef struct ruin_Size { ruin_SizeKind kind; F32 value; F32 strictness; } ruin_Size;
typedef struct ruin_Vec2 { U32 x, y; }                                      ruin_Vec2;
typedef struct ruin_Rect { U32 x, y, h, w; }                                ruin_Rect;
typedef struct ruin_Color { U8 r, g, b, a; }                                ruin_Color;

typedef struct ruin_Widget                     ruin_Widget;
struct ruin_Widget {
    ruin_Id id;

    ruin_Widget *first;
    ruin_Widget *last;
    ruin_Widget *next;
    ruin_Widget *prev;
    ruin_Widget *parent;

    ruin_Size size[RUIN_AXISCOUNT];
    ruin_Rect draw_coords;
    ruin_WidgetFlags flags;

    char* text;
};

typedef struct ruin_Window                     ruin_Window;
struct ruin_Window {
    ruin_Id id;

    ruin_Rect window_rect;
    ruin_Widget* root_widget;
    ruin_WindowFlags window_flags;

    const char* title;

    ruin_Window* next;
    ruin_Window* prev;
};
typedef struct ruin_WindowList { ruin_Window* first; ruin_Window* last; } ruin_WindowList;

typedef struct {
    Arena arena;

    ruin_Id hot;
    ruin_Id active;
    ruin_Vec2 mouse_position;

    ruin_WindowList windows;
    ruin_Window* current_window;
} ruin_Context;


ruin_Context* create_ruin_context() {
    const U32 ARENA_SIZE = 1024;
    Arena arena = {0};
    unsigned char* buffer = (unsigned char*) malloc(ARENA_SIZE);
    arena_init(&arena, buffer, ARENA_SIZE);
    ruin_Context* ctx = arena_alloc(&arena, sizeof(ruin_Context));

    MEM_ZERO(ctx, sizeof(ruin_Context));
    ctx->arena = arena;

    return ctx;
};

ruin_Window* get_window_by_id(ruin_Context* ctx, ruin_Id id) {
    for (ruin_Window* tmp = ctx->windows.first; tmp != NULL; tmp = tmp->next) {
        if (tmp->id == id) return tmp;
    };

    return NULL;
};

internal ruin_Id hash_string(const char *str) {
    ruin_Id hash = 5381; int c;
    while ((c = *str++))  hash = ((hash << 5) + hash) + c;
    return hash;
};

void ruin_BeginWindow(ruin_Context* ctx, const char* title, ruin_Rect rect, ruin_WindowFlags flags) {
    ruin_Id id = hash_string(title);

    ruin_Window* window = get_window_by_id(ctx, id);
    if (window == NULL) {
        window = arena_alloc(&ctx->arena, sizeof(ruin_Window));
        window->id = id;
        window->title = title;
        window->window_rect = rect;
        window->window_flags = flags;

        DLLPushFront(ctx->windows.first, ctx->windows.last, window);

        ruin_Widget* root = arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        MEM_ZERO(root, sizeof(ruin_Widget));

        root->id = hash_string("root##default");
        root->size[0] = (ruin_Size) { .kind = RUIN_SIZEKIND_CHILDRENSUM, .value = 0, .strictness=1 }; ;
        root->size[1] = (ruin_Size) { .kind = RUIN_SIZEKIND_CHILDRENSUM, .value = 0, .strictness=1 }; ;
        window->root_widget = root;

    };

    ctx->current_window = window;
};

void ruin_EndWindow(ruin_Context* ctx) {
    ctx->current_window = NULL;
};


void ruin_ComputeLayout() { };

void ruin_SameLine() { };

B8 ruin_Button(ruin_Context* ctx, const char* label) {


    ruin_Widget* root = ctx->current_window->root_widget;
    if (root->first == NULL) {
        ruin_Widget* rect = arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        MEM_ZERO(rect, sizeof(ruin_Widget));

        rect->size[0] = (ruin_Size) {.kind = RUIN_SIZEKIND_NULL, .value = 100, .strictness = 0};
        rect->size[1] = (ruin_Size) {.kind = RUIN_SIZEKIND_NULL, .value = 100, .strictness = 0};
        root->first = rect;
    } else {
        // ruin_Widget* temp = root->first;
        // while (temp->next != NULL) temp = temp->next;
        // temp->next = rect;
    };

    return false;
};

static ruin_Rect  make_rect(U32 x_pos, U32 y_pos, U32 height, U32 width);
static ruin_Vec2  make_vec2(U32 x_pos, U32 y_pos);
static ruin_Color make_color(U8 red, U8 green, U8 blue, U8 alpha);

static B8 is_point_over_rect(ruin_Rect rect, ruin_Vec2 point);
static ruin_Rect overlap_rect(ruin_Rect rect1, ruin_Rect rect2);
static ruin_Rect expand_rect(ruin_Rect rect, int n);

ruin_Rect make_rect(U32 x_pos, U32 y_pos, U32 height, U32 width) {
  ruin_Rect res;
  res.x = x_pos;
  res.y = y_pos;
  res.h = height;
  res.w = width;

  return res;
};

ruin_Vec2 make_vec2(U32 x_pos, U32 y_pos) {
  ruin_Vec2 res;
  res.x = x_pos;
  res.y = y_pos;

  return res;
};

ruin_Color make_color(U8 red, U8 green, U8 blue, U8 alpha) {
  ruin_Color res;
  res.r = red;
  res.b = blue;
  res.g = green;
  res.a = alpha;

  return res;
};


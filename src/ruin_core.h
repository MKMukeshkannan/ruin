#ifndef RUIN_CORE 
#define RUIN_CORE 

#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include <assert.h>

#define RUIN_TRANSIENT_ID    (U64)-1
#define WIDGET_ARRAY    40
#define WINDOW_ARRAY    40
#define DRAW_QUEUE_SIZE 40
#define ROW_SPACING 5

// HEADER MESS
#define ruin_WidgetOptions U32

typedef size_t ruin_FontID;
typedef U64    ruin_Id; 
typedef enum   ruin_SizeKind    { 
    RUIN_SIZEKIND_PIXEL,
    RUIN_SIZEKIND_TEXTCONTENT,
    RUIN_SIZEKIND_PARENTPERCENTAGE,
    RUIN_SIZEKIND_CHILDRENSUM,
    RUIN_SIZEKIND_GROW 
} ruin_SizeKind;
typedef enum ruin_MouseButtonClickType { 
    RUIN_MOUSE_BUTTON_CLICK_LEFT             = (1<<0),
    RUIN_MOUSE_BUTTON_CLICK_MIDDLE           = (1<<1),
    RUIN_MOUSE_BUTTON_CLICK_RIGHT            = (1<<2),
} ruin_MouseButtonClickType;
#define ruin_MouseButtonClick U8 
typedef enum   ruin_WidgetFlags { 
    RUIN_WIDGETFLAGS_NO_FLAGS         =  (1<<0),
    RUIN_WIDGETFLAGS_DRAW_BACKGROUND  =  (1<<1),
    RUIN_WIDGETFLAGS_DRAW_BORDER      =  (1<<2),
    RUIN_WIDGETFLAGS_DRAW_TEXT        =  (1<<3),
    RUIN_WIDGETFLAGS_CLICKABLE        =  (1<<4),
    RUIN_WIDGETFLAGS_HOVERABLE        =  (1<<5),
} ruin_WidgetFlags;
typedef enum   ruin_WindowFlags { 
    RUIN_WINDOWFLAGS_DRAGABLE = (1<<0),
    RUIN_WINDOWFLAGS_NOMENU   = (1<<1),
    RUIN_WINDOWFLAGS_NOTITLE  = (1<<2) 
}  ruin_WindowFlags;
typedef enum   ruin_Axis      { 
    RUIN_AXISX,
    RUIN_AXISY,
    RUIN_AXISCOUNT
}                                                                    ruin_Axis;
typedef enum   ruin_DrawType    { 
    RUIN_DRAWTYPE_RECT,
    RUIN_DRAWTYPE_CLIP,
    RUIN_DRAWTYPE_TEXT 
} ruin_DrawType;

typedef struct ruin_Size            { ruin_SizeKind kind; F32 value; F32 strictness; }                                                           ruin_Size;
typedef struct ruin_Vec2            { F32 x, y; }                                                                                                ruin_Vec2;
typedef struct ruin_Rect            { F32 x, y, h, w; }                                                                                          ruin_Rect;
typedef struct ruin_Color           { U8 r, g, b, a; }                                                                                           ruin_Color;
typedef struct ruin_RectSide        { U8 left, right, top, bottom; }                                                                         ruin_RectSide;

typedef struct ruin_Bitmap {
    U32 width;
    S64 height;
    U32 rows;
    S32 bearingX;
    S32 bearingY;
    S64 advance;
    S64 pitch;
    U8* buffer;
} ruin_Bitmap;
typedef struct ruin_FontInfo {
    String8         font_name;
    U32             font_size;
    ruin_Bitmap     bitmap[128];
} ruin_FontInfo;

// DECLARE_ARRAY(fontInfo, ruin_FontInfo);

ruin_Color make_color_hex(U32 color);

typedef struct ruin_DrawCall {
    ruin_DrawType type;
    union {
        struct ruin_DrawRect { ruin_Rect rect; ruin_Color color; U8 border_width; } draw_rect;
        struct ruin_DrawClip { ruin_Rect rect; } draw_clip;
        struct ruin_DrawText { const char* text; ruin_Vec2 pos; ruin_FontID font_id; } draw_text;
    } draw_info_union;
} ruin_DrawCall;

typedef struct ruin_Widget                                                   ruin_Widget;
struct ruin_Widget {
    ruin_Size size[RUIN_AXISCOUNT];
    ruin_Vec2 fixed_size;
    ruin_Vec2 partially_offset;
    struct ruin_DrawCoords {
        ruin_Rect bbox;
        ruin_Vec2 text_pos;
    } draw_coords;

    ruin_Widget *first_child;
    ruin_Widget *last_child;
    ruin_Widget *next_sibling;
    ruin_Widget *prev_sibling;
    ruin_Widget *parent;

    ruin_WidgetOptions flags;
    String8 display_text;
    String8 widget_name;
    ruin_Id id;
    ruin_RectSide padding;
    ruin_RectSide border_width;

    ruin_Axis child_layout_axis;

    ruin_Color  background_color;
    ruin_Color  foreground_color;
    ruin_Color  hover_color;
    ruin_Color  active_color;
    ruin_Color  border_color;
    ruin_FontID font;
    U32 child_count;
};

typedef struct ruin_Window ruin_Window;
struct ruin_Window {
    ruin_Rect window_rect;
    ruin_Widget* root_widget;  // window tree, donot persist across frames
    const char* title;
    char window_flags;
    ruin_Id id;
};


// CURENTLY PUSHABLE IS THE INDEX
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define DEFINE_ARRAY_CACHES(type) typedef struct { U32 index; U32 capacity; type* items; } type##Array; \
    internal type* type##Array__Get(type##Array* array, U32 index) { \
        if (array == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID ARRAY POINTER ON %s\n", STR(type##Array__Get)); return NULL; }; \
        if (array->items == NULL ) { fprintf(stderr, "[RUIN_ERROR]: INVALID ARRAY ITEMS POINTER ON %s\n", STR(type##Array__Get)); return NULL; }; \
        if (index > array->index) { fprintf(stderr, "[RUIN_ERROR]: INVALID INDEX ACCESS ON %s\n", STR(type##Array__Get)); return NULL; }; \
        return &array->items[index]; \
    }; \
    internal type* type##Array__Push(type##Array *array, type element) { \
        if (array == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID ARRAY POINTER ON %s\n", STR(type##Array__Push)); return NULL; }; \
        if (array->items == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID ARRAY ITEMS POINTER ON %s\n", STR(type##Array__Push)); return NULL; }; \
        if (array->index >= array->capacity) { fprintf(stderr, "[RUIN_ERROR]: ARRAY CAPACITY EXCEEDED ON %s\n", STR(type##Array__Push)); return NULL; }; \
        array->items[array->index++] = element; \
        return &array->items[array->index - 1];\
    }; \
    internal type##Array type##Array__Init(Arena* arena, U32 capacity) { \
        type##Array res; res.index = 0; res.capacity = capacity;\
        if (arena == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID ARENA POINTER on %s\n", STR(type##Array__Init)); return res; }; \
        res.items = (type*)arena_alloc(arena, sizeof(type) * capacity); \
        if (res.items == NULL) { fprintf(stderr, "[RUIN_ERROR]: UNABLE TO ALLOCATE %s -> items\n", STR(type##Array)); }; \
        return res; \
    }; \

#define DEFINE_STACKS(type) typedef struct { U32 top; U32 capacity; type* items; } type##Stack; \
    internal bool type##Stack__IsEmpty(type##Stack* stack) { \
        if (stack == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(type##Stack__IsEmpty)); return false; }; \
        if (stack->items == NULL ) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK ITEMS POINTER ON %s\n", STR(type##Stack__IsEmpty)); return false; }; \
        return (stack->top == 0); \
    }; \
    internal type* type##Stack__GetTop(type##Stack* stack) { \
        if (stack == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(type##Stack__Get)); return NULL; }; \
        if (stack->items == NULL ) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK ITEMS POINTER ON %s\n", STR(type##Stack__Get)); return NULL; }; \
        if (stack->top == 0 || stack->top > stack->capacity) { fprintf(stderr, "[RUIN_ERROR]: INVALID TOP ACCESS ON %s\n", STR(type##Stack__Get)); return NULL; }; \
        return &stack->items[stack->top - 1]; \
    }; \
    internal type* type##Stack__Push(type##Stack *stack, type element) { \
        if (stack == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(type##Stack__Push)); return NULL; }; \
        if (stack->items == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK ITEMS POINTER ON %s\n", STR(type##Stack__Push)); return NULL; }; \
        if (stack->top >= stack->capacity) { fprintf(stderr, "[RUIN_ERROR]: STACK CAPACITY EXCEEDED ON %s, top:%u cap:%u\n", STR(type##Stack__Push), stack->top, stack->capacity); return NULL; }; \
        stack->items[stack->top++] = element; \
        return &stack->items[stack->top - 1];\
    }; \
    internal type* type##Stack__Pop(type##Stack *stack) {\
        if (stack == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(type##Stack__Pop)); return NULL; }; \
        if (stack->items == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK ITEMS POINTER ON %s\n", STR(type##Stack__Pop)); return NULL; }; \
        if (stack->top == 0) { fprintf(stderr, "[RUIN_ERROR]: STACK ALREADY EMPTY %s\n", STR(type##Stack__Pop)); return NULL; }; \
        return &stack->items[stack->top--];\
    };\
    internal type##Stack type##Stack__Init(Arena* arena, U32 capacity) { \
        type##Stack res; res.top = 0; res.capacity = capacity;\
        if (arena == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID ARENA POINTER on %s\n", STR(type##Stack__Init)); return res; }; \
        res.items = (type*)arena_alloc(arena, sizeof(type) * capacity); \
        if (res.items == NULL) { fprintf(stderr, "[RUIN_ERROR]: UNABLE TO ALLOCATE %s -> items\n", STR(type##Stack)); }; \
        return res; \
    }; \
    internal void type##Stack__Clear(type##Stack* stack) { \
        if (stack == NULL) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(type##Stack__Clear)); return; }; \
        if (stack->items == NULL ) { fprintf(stderr, "[RUIN_ERROR]: INVALID STACK ITEMS POINTER ON %s\n", STR(type##Stack__Clear)); return; }; \
        stack->top = 0; \
    }; \

DEFINE_ARRAY_CACHES(ruin_Widget);
DEFINE_ARRAY_CACHES(ruin_Window);
DEFINE_ARRAY_CACHES(ruin_FontInfo);

DEFINE_STACKS(ruin_Color);
DEFINE_STACKS(ruin_Axis);
DEFINE_STACKS(ruin_RectSide);
DEFINE_STACKS(ruin_FontID);
// DEFINE_STACKS(ruin_FontInfo);

// DEFINE_STACKS(ruin_Widget);
typedef struct {
    U32 top;
    U32 capacity;
    ruin_Widget* items[20];
} ruin_WidgetStack;
internal bool ruin_WidgetStack__IsEmpty(ruin_WidgetStack* stack) {
    if (stack == NULL) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(ruin_WidgetStack__IsEmpty));
        return false;
    };
    return (stack->top == 0);
};

internal ruin_Widget* ruin_WidgetStack__GetTop(ruin_WidgetStack* stack) {
    if (stack == NULL) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(ruin_WidgetStack__Get));
        return NULL;
    };
    if (stack->top == 0 || stack->top > stack->capacity) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID TOP ACCESS ON %s, stack_top:%u and stack_capacity:%u\n", STR(ruin_WidgetStack__Get), stack->top, stack->capacity);
        return NULL;
    };
    if (stack->items[stack->top - 1] == NULL ) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID STACK ITEMS POINTER ON %s\n", STR(ruin_WidgetStack__Get));
        return NULL;
    };
    return stack->items[stack->top - 1];
};

internal ruin_Widget* ruin_WidgetStack__Push(ruin_WidgetStack* stack, ruin_Widget* element) {
    if (stack == NULL) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(ruin_WidgetStack__Push));
        return NULL;
    };
    if (stack->top >= stack->capacity) {
        fprintf(stderr, "[RUIN_ERROR]: STACK CAPACITY EXCEEDED ON %s\n", STR(ruin_WidgetStack__Push));
        return NULL;
    };
    stack->items[stack->top++] = element;
    return stack->items[stack->top - 1];
};

internal ruin_Widget* ruin_WidgetStack__Pop(ruin_WidgetStack* stack) {
    if (stack == NULL) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(ruin_WidgetStack__Pop));
        return NULL;
    };
    if (stack->top == 0) {
        fprintf(stderr, "[RUIN_ERROR]: STACK ALREADY EMPTY %s\n", STR(ruin_WidgetStack__Pop));
        return NULL;
    };
    if (stack->items[stack->top - 1] == NULL) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID STACK ITEMS POINTER ON %s\n", STR(ruin_WidgetStack__Pop));
        return NULL;
    };
    return stack->items[--stack->top];
};

internal ruin_WidgetStack* ruin_WidgetStack__Init(Arena* arena) {
    ruin_WidgetStack* res = (ruin_WidgetStack*)arena_alloc(arena, sizeof(ruin_WidgetStack));
    res->top = 0;
    res->capacity = 20;
    if (arena == NULL) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID ARENA POINTER on %s\n", STR(ruin_WidgetStack__Init));
        return res;
    };
    return res;
};

internal void ruin_WidgetStack__Clear(ruin_WidgetStack* stack) {
    if (stack == NULL) {
        fprintf(stderr, "[RUIN_ERROR]: INVALID STACK POINTER ON %s\n", STR(ruin_WidgetStack__Clear));
        return;
    };
    stack->top = 0;
};


typedef struct {
    ruin_WidgetArray widgets; // caches -> list of all widgets in that current window, persisted across frames
    ruin_WindowArray windows;
    struct {
        size_t index;
        ruin_DrawCall items[DRAW_QUEUE_SIZE]; // emptied every frame
    } draw_queue;

    Arena arena;       // persisted arena, across frames
    Arena temp_arena;  // reseted every frames, holds transient widgets

    Arena font_bitmap_arena;  // happens once, stores all glyph, bitmap and codespace information
    ruin_FontInfoArray fonts;
                       
    ruin_Window* current_window;

    // USED FOR INTERACTIVITY
    ruin_Vec2 mouse_position;
    ruin_MouseButtonClick mouse_action;
    ruin_Id hot;
    ruin_Id active;
    U64 frame;

    // STYLE STACKS => push to the stacks to apply style to upcoming widgets
    ruin_ColorStack    background_color_stack;
    ruin_ColorStack    hover_color_stack;
    ruin_ColorStack    foreground_color_stack;
    ruin_ColorStack    active_color_stack;
    ruin_RectSideStack padding_stack; 
    ruin_AxisStack     child_direction_stack;
    ruin_FontIDStack   font_stack;

    ruin_WidgetStack* parent_stack;
} ruin_Context;

ruin_Id hash_string(ruin_Context* ctx, const char* str);
// void ruin_UpdateIO(ruin_Context* ctx);
void ruin_SetFontCount(ruin_Context* ctx, size_t number_of_font_sizes);
ruin_FontID ruin_LoadFont(ruin_Context* ctx, const char* path, const char* name, U32 font_size);
ruin_Context* create_ruin_context();
ruin_Widget* ruin_create_widget_ex(ruin_Context* ctx, const char* full_name, ruin_Id id, ruin_WidgetOptions opt);

ruin_Window* get_window_by_id(ruin_Context* ctx, ruin_Id id);
ruin_Widget* get_widget_by_id(ruin_Context* ctx, ruin_Id id);

void ruin_BeginWindow(ruin_Context* ctx, const char* title, ruin_Rect rect, ruin_WindowFlags flags);
void ruin_EndWindow(ruin_Context* ctx);
void ruin_ComputeLayout(ruin_Context* ctx);
void ruin_PushWidgetNArray(ruin_Widget* root_widget, ruin_Widget* new_widget);


#ifdef __cplusplus
}
#endif

#endif


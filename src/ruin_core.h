#ifndef RUIN_CORE 
#define RUIN_CORE 

#ifdef __cplusplus
extern "C" {
#endif


#include "base.h"

#define RUIN_TRANSIENT_ID    (U64)-1
#define WIDGET_ARRAY    40
#define WINDOW_ARRAY    40
#define DRAW_QUEUE_SIZE 40
#define ROW_SPACING 5

// HEADER MESS
#define ruin_WidgetOptions U32

typedef U64    ruin_Id; 
typedef enum   ruin_SizeKind    { 
    RUIN_SIZEKIND_PIXEL,
    RUIN_SIZEKIND_TEXTCONTENT,
    RUIN_SIZEKIND_PARENTPERCENTAGE,
    RUIN_SIZEKIND_CHILDRENSUM,
    RUIN_SIZEKIND_GROW 
} ruin_SizeKind;
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
    RUIN_WINDOWFLAGS_NOMENU = (1<<1),
    RUIN_WINDOWFLAGS_NOTITLE = (1<<2) 
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
}                                               ruin_DrawType;

typedef struct ruin_Size            { ruin_SizeKind kind; F32 value; F32 strictness; }                                                           ruin_Size;
typedef struct ruin_Vec2            { F32 x, y; }                                                                                                ruin_Vec2;
typedef struct ruin_Rect            { F32 x, y, h, w; }                                                                                          ruin_Rect;
typedef struct ruin_Color           { U8 r, g, b, a; }                                                                                           ruin_Color;
typedef struct ruin_RectSide        { U8 left, right, top, bottom; }                                                                         ruin_RectSide;

DECLARE_ARRAY(u32, U32);

typedef struct ruin_Bitmap {
    U32 width;
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

DECLARE_ARRAY(fontInfo, ruin_FontInfo);


ruin_Color make_color_hex(U32 color);

typedef struct ruin_DrawCall {
    ruin_DrawType type;
    union {
        struct ruin_DrawRect { ruin_Rect rect; ruin_Color color; U8 border_width; } draw_rect;
        struct ruin_DrawClip { ruin_Rect rect; } draw_clip;
        struct ruin_DrawText { const char* text; ruin_Vec2 pos; } draw_text;
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

    ruin_Color background_color;
    ruin_Color foreground_color;
    ruin_Color hover_color;
    ruin_Color active_color;
    ruin_Color border_color;
    U32 child_count;
};

typedef struct { S16 top; ruin_Widget* items[100]; } ruin_WidgetStack;
internal ruin_WidgetStack* create_stack(Temp_Arena_Memory temp) { ruin_WidgetStack* stack = (ruin_WidgetStack*)arena_alloc(temp.arena, sizeof(ruin_WidgetStack)); MEM_ZERO(stack, sizeof(ruin_WidgetStack)); stack->top = -1; return stack; };
internal ruin_Widget* get_top(ruin_WidgetStack* stack) { if (stack->top == -1) { return NULL; }; return stack->items[stack->top]; };
internal ruin_Widget* pop(ruin_WidgetStack* stack) { if (stack->top == -1) return NULL; return stack->items[stack->top--]; };
internal void push(ruin_WidgetStack* stack, ruin_Widget* widget) { stack->items[++stack->top] = widget; };
internal bool is_stack_empty(ruin_WidgetStack* stack) { return (stack->top == -1); };
internal void clear_stack(ruin_WidgetStack* stack) {stack->top = -1;};

typedef struct ruin_Window                     ruin_Window;
struct ruin_Window {
    ruin_Rect window_rect;
    ruin_Widget* root_widget;  // window tree, donot persist across frames
    const char* title;

    ruin_Window* next;
    ruin_Window* prev;
    char window_flags;
    ruin_Id id;
};

DECLARE_STACK(color, ruin_Color);
DECLARE_STACK(axis, ruin_Axis);
DECLARE_STACK(rectsides, ruin_RectSide);

typedef struct {
    struct {
        size_t index;
        ruin_Widget* items[WIDGET_ARRAY];      // caches -> list of all widgets in that current window, persisted across frames
    } widgets;
    struct {
        size_t index;
        ruin_Window* items[WINDOW_ARRAY];      // caches -> list of all widgets in that current window, persisted across frames
    } windows;
    struct {
        size_t index;
        ruin_DrawCall items[DRAW_QUEUE_SIZE]; // emptied every frame
    } draw_queue;

    Arena arena;       // persisted arena, across frames
    Arena temp_arena;  // reseted every frames, holds transient widgets

    Arena font_build;  // happens once, stores all glyph, bitmap and codespace information
    ruin_FontInfoArray* fonts;
                       
    ruin_Window* current_window;

    ruin_Vec2 mouse_position;
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

    ruin_WidgetStack parent_stack;
} ruin_Context;

ruin_Id hash_string(ruin_Context* ctx, const char* str);
void ruin_SetFontCount(ruin_Context* ctx, size_t number_of_font_sizes);
void ruin_LoadFont(ruin_Context* ctx, const char* path, const char* name, U32 font_size);
ruin_Context* create_ruin_context();
ruin_Widget* ruin_create_widget_ex(ruin_Context* ctx, const char* full_name, ruin_Id id, ruin_WidgetOptions opt);

ruin_Window* get_window_by_id(ruin_Context* ctx, ruin_Id id);
ruin_Widget* get_widget_by_id(ruin_Context* ctx, ruin_Id id);

void ruin_BeginWindow(ruin_Context* ctx, const char* title, ruin_Rect rect, ruin_WindowFlags flags);
void ruin_EndWindow(ruin_Context* ctx);
void ruin_ComputeLayout(ruin_Context* ctx);
void push_widget_narry(ruin_Widget* root_widget, ruin_Widget* new_widget);


#ifdef __cplusplus
}
#endif

#endif


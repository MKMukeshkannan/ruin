#ifndef RUIN_HEADER
#define RUIN_HEADER

#include "base.h"

typedef U32 ruin_Id; 

typedef struct ruin_Vec2  ruin_Vec2;
typedef struct ruin_Rect  ruin_Rect;
typedef struct ruin_Color ruin_Color;
typedef struct ruin_Context ruin_Context;
typedef struct ruin_DrawCommand ruin_DrawCommand;
typedef struct ruin_Widget ruin_Widget;
typedef enum ruin_WidgetFlags ruin_WidgetFlags;

typedef struct ruin_Size ruin_Size;
typedef enum ruin_SizeKind ruin_SizeKind;

enum ruin_SizeKind {
    RUIN_SIZEKIND_NULL,
    RUIN_SIZEKIND_TEXTCONTENT,
    RUIN_SIZEKIND_PARENTPERCENTAGE,
    RUIN_SIZEKIND_CHILDRENSUM
};

enum ruin_WidgetFlags {
    RUIN_WIDGETFLAGS_CLICKABLE = (1<<0),
    RUIN_WIDGETFLAGS_HOVERABLE = (1<<1),
};

enum {
    RUIN_AXISX,
    RUIN_AXISY,
    RUIN_AXISCOUNT,
};

struct ruin_Size {
    ruin_SizeKind kind;
    F32 value;
    F32 strictness;
};

struct ruin_Vec2 { U32 x, y; };
struct ruin_Rect { U32 x, y, h, w; };
struct ruin_Color { U8 r, g, b, a; };


struct ruin_Context {
    Arena* arena;
    ruin_Id hot;
    ruin_Id active;
    ruin_Vec2 mouse_position;
};


struct ruin_Widget {
    ruin_Id key;

    ruin_Widget *first;
    ruin_Widget *last;
    ruin_Widget *next;
    ruin_Widget *prev;
    ruin_Widget *parent;

    ruin_Size size[RUIN_AXISCOUNT];
    ruin_Rect draw_coords;
    ruin_WidgetFlags flags;
};

static ruin_Rect  make_rect(U32 x_pos, U32 y_pos, U32 height, U32 width);
static ruin_Vec2  make_vec2(U32 x_pos, U32 y_pos);
static ruin_Color make_color(U8 red, U8 green, U8 blue, U8 alpha);

static B8 is_point_over_rect(ruin_Rect rect, ruin_Vec2 point);
static ruin_Rect overlap_rect(ruin_Rect rect1, ruin_Rect rect2);
static ruin_Rect expand_rect(ruin_Rect rect, int n);

ruin_Context* create_ruin_context();

#endif

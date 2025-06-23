#ifndef RUIN_HEADER
#define RUIN_HEADER

#include "base.h"

typedef U32 ruin_Id; 

typedef struct ruin_Vec2  ruin_Vec2;
typedef struct ruin_Rect  ruin_Rect;
typedef struct ruin_Color ruin_Color;
typedef struct ruin_Context ruin_Context;
typedef struct ruin_DrawCommand ruin_DrawCommand;

struct ruin_Vec2 { U32 x, y; };
struct ruin_Rect { U32 x, y, h, w; };
struct ruin_Color { U8 r, g, b, a; };
struct ruin_Context {
    ruin_Id hot;
    ruin_Id active;
    ruin_Vec2 mouse_position;
};

ruin_Rect  make_rect(U32 x_pos, U32 y_pos, U32 height, U32 width);
ruin_Vec2  make_vec2(U32 x_pos, U32 y_pos);
ruin_Color make_color(U8 red, U8 green, U8 blue, U8 alpha);


#endif

#include "ruin.h"
#include "base.h"
#include <stdlib.h>

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

ruin_Context* create_ruin_context() {
    const size_t TOTAL_SIZE = 1024 * 1024 * 5;

    Arena arena;
    unsigned char* buffer = (unsigned char*)malloc(TOTAL_SIZE);
    arena_init(&arena, buffer, TOTAL_SIZE);

    ruin_Context* rc = arena_allocate(&arena, sizeof(ruin_Context));
    rc->arena = &arena;


    return rc;
};

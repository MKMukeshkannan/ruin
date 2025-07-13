#ifndef RUIN_RAYLIB_RENDERER
#define RUIN_RAYLIB_RENDERER

#include "ruin.c"
#include <raylib.h>

void ruin_RaylibRender(ruin_Context* ctx) {


    for (int i = 0; i < ctx->draw_queue.index; ++i) {
        ruin_Rect rect = ctx->draw_queue.items[i].draw_rect.rect;
        ruin_Color color = ctx->draw_queue.items[i].draw_rect.color;

        DrawRectangle(rect.x, rect.y, rect.w, rect.h - rect.y, (Color) {
            .a=color.a,
            .r=color.r,
            .b=color.b,
            .g=color.g,
        });

        DrawRectangleLinesEx((Rectangle) {
            .x=rect.x, .y=rect.y, .width=rect.w, .height=rect.h - rect.y,
        }, 1, BLACK);
    };



};


#endif

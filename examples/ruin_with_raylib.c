#include "../src/ruin.c"
#include "../src/ruin_raylib_render.c"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

void CustomLog(int msgType, const char *text, va_list args) { return; }

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetTraceLogCallback(CustomLog); // call this before InitWindow()
    InitWindow(screenWidth, screenHeight, "THIS IS A WINDOW");

    ruin_Context* ctx = create_ruin_context();



    SetTargetFPS(60);
    while (!WindowShouldClose()) {







       ruin_BeginWindow(ctx, "Inspector", (ruin_Rect){0, 0, 200, 200}, 0);

       if (ruin_Button(ctx, "")) {};

       ruin_EndWindow(ctx);







        // ruin_ComputeLayout();
        // ruin_RaylibRender();


        BeginDrawing();
        EndDrawing();
    }
    CloseWindow();

    free(ctx);
    return 0;
};

void r_draw_rect(ruin_Rect *rect, ruin_Color color) {
    if (rect == NULL)
        return;

    DrawRectangle(rect->x, rect->y, rect->h, rect->w, (Color){ .r = color.r, .g = color.g, .b = color.b, .a = color.a, });
};

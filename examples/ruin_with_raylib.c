#include "../src/ruin.c"
#include "../src/ruin_raylib_render.c"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

void CustomLog(int msgType, const char *text, va_list args) { return; }

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 800;

    SetTraceLogCallback(CustomLog); // call this before InitWindow()
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "THIS IS A WINDOW");

    ruin_Context* ctx = create_ruin_context();
    ruin_RaylibInit(ctx);

    U32 from = 0, to = 100, current = 30;

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
       ClearBackground(RAYWHITE);


       ruin_BeginWindow(ctx, "Inspector", (ruin_Rect){0, 0, 400, 400}, 0);

        ruin_Label(ctx, "Inspector");
        ruin_Label(ctx, "Inspector2");
        ruin_Label(ctx, "Inspector3");
        ruin_Label(ctx, "Inspector4");
       //  if (ruin_Button(ctx, "Contact me by tonight 9pm")) {};
        if (ruin_Slider(ctx, "slider", &from, &to, &current)) {};

       //  if (ruin_Button(ctx, "Ok3")) {};
       //  if (ruin_Button(ctx, "Ok13")) {};
       ruin_EndWindow(ctx);




       ruin_ComputeLayout(ctx);

       ruin_RaylibRender(ctx);

    };
    CloseWindow();

    free(ctx);
    return 0;
};

void r_draw_rect(ruin_Rect *rect, ruin_Color color) {
    if (rect == NULL)
        return;

    DrawRectangle(rect->x, rect->y, rect->h, rect->w, (Color){ .r = color.r, .g = color.g, .b = color.b, .a = color.a, });
};

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

    F32 from = 0, to = 100, current = 30;
    SetTargetFPS(60);

    const char* x = "xx";
    bool y = false;
    while (!WindowShouldClose()) {

        Vector2 m_pos = GetMousePosition();
        ctx->mouse_position = (ruin_Vec2) { .x = m_pos.x, .y = m_pos.y };

        ClearBackground(RAYWHITE);

        if (IsKeyDown(KEY_SPACE)) y = true;
        if (IsKeyUp(KEY_SPACE)) y = false;

        if (IsKeyDown(KEY_SPACE)) x = "xx";
        if (IsKeyUp(KEY_SPACE)) x = "yy";
        if (IsKeyDown(KEY_UP)) current = MIN(to, current + 1); 
        if (IsKeyDown(KEY_DOWN)) current = MAX(from, current - 1); 

        //
        // START HIERARCHY
        ruin_BeginWindow(ctx, "Inspector", (ruin_Rect) {.x=100, .y = 10, .h = 400, .w = 400, }, RUIN_WINDOWFLAGS_DRAGABLE);

        ruin_Button(ctx, "Clicky");
        ruin_ButtonToggle(ctx, "Toggle", &y);
        ruin_Label(ctx, "Helloy");

        ruin_EndWindow(ctx);
        // ENDS HIERARCHY
        //


        ruin_ComputeLayout(ctx);
        ruin_RaylibRender(ctx);
        ctx->frame++;
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

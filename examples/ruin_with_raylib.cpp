#include "../src/ruin.c"
#include "../src/ruin_raylib_render.c"
#include "raylib.h"
#include <cstdio>
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

    int height;
    while (!WindowShouldClose()) {
        height = GetMonitorHeight(0);

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
        ruin_BeginWindow(ctx, "Inspector", (ruin_Rect) {.x=0, .y = 0, .h = 200, .w = 800, }, RUIN_WINDOWFLAGS_DRAGABLE);


        ruin_Label(ctx, "Image Editor");
        ruin_SameLine(ctx, "blur_group") {
             ruin_Label(ctx, "Blur");
             ruin_SpacerX(ctx, "blur space");
             ruin_Label(ctx, "90");
        }
        
        ruin_SameLine(ctx, "grain_group") {
            ruin_Label(ctx, "Grain");
            ruin_SpacerX(ctx, "grain space");
            ruin_Label(ctx, "70");
        };

        ruin_SameLine(ctx, "showeffect_group") {
            ruin_Label(ctx, "Show Effect");
            ruin_SpacerX(ctx, "showeffect space");
            ruin_Button(ctx, "ON");
        };

        ruin_SpacerY(ctx, "vertical_spacer2");
        ruin_Button(ctx, "Export");

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

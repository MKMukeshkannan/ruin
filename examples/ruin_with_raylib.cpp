#include "ruin_widget.h"
#include "ruin_core.h"
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

    ruin_SetFontCount(ctx, 3);
    // ruin_LoadFont(ctx, "resources/jetbrains.ttf", "JETBRAINS_800", 30);
//     ruin_LoadFont(ctx, "resources/inter_semi.ttf", "INTER_800", 16);
     ruin_LoadFont(ctx, "resources/inter.ttf", "INTER_500", 16);

    ruin_RaylibInit(ctx);

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        Vector2 m_pos = GetMousePosition();
        ctx->mouse_position = (ruin_Vec2) { .x = m_pos.x, .y = m_pos.y };

        ClearBackground(RAYWHITE);


        //
        // START HIERARCHY
        ruin_BeginWindow(ctx, "Inspector", (ruin_Rect) {.x=0, .y = 0, .h = 800, .w = 300, }, RUIN_WINDOWFLAGS_DRAGABLE);



        ruin_Label(ctx, "Image Editor");
       ruin_SameLine(ctx, "grain_group") {
           ruin_Label(ctx, "grain");
           ruin_SpacerFillX(ctx);
           ruin_Button(ctx, "20");
       };
       ruin_SameLine(ctx, "grain_group2") {
           ruin_Label(ctx, "grain");
           ruin_SpacerFillX(ctx);
           ruin_Button(ctx, "click connect");
       };
       

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

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

    SetTraceLogCallback(CustomLog);
    InitWindow(screenWidth, screenHeight, "THIS IS A WINDOW");

    ruin_Context* ctx = create_ruin_context();

    ruin_SetFontCount(ctx, 3);
    ruin_FontID jetbrains_20 = ruin_LoadFont(ctx, "resources/jetbrains.ttf", "JETBRAINS_20", 16);
    ruin_FontID jetbrains_24 = ruin_LoadFont(ctx, "resources/jetbrains.ttf", "JETBRAINS_24", 24);
    ruin_FontID inter_24 = ruin_LoadFont(ctx, "resources/inter.ttf", "INTER_24", 24);

    ruin_RaylibInit(ctx);

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
       Vector2 m_pos = GetMousePosition();
       ctx->mouse_position = (ruin_Vec2) { .x = m_pos.x, .y = m_pos.y };
       if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ctx->mouse_action |= RUIN_MOUSE_BUTTON_CLICK_LEFT;
       };
       if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            ctx->mouse_action &= ~RUIN_MOUSE_BUTTON_CLICK_LEFT;
       };
       printf("click: %u\n", ctx->mouse_action);
       ClearBackground(WHITE);


       //
       // START HIERARCHY
       ruin_BeginWindow(ctx, "Inspector", (ruin_Rect) {.x=0, .y = 0, .h = 800, .w = 300, }, RUIN_WINDOWFLAGS_DRAGABLE);

       ruin_FontIDStack__Push(&ctx->font_stack, inter_24);
       ruin_Label(ctx, "Image Editor");
       ruin_FontIDStack__Pop(&ctx->font_stack);


       ruin_SameLine(ctx, "grain_group") {
           ruin_Label(ctx, "grain");
           ruin_SpacerFillX(ctx);
           if (ruin_Button(ctx, "20")) {
               printf("helo\n");
           };
       };
       ruin_SameLine(ctx, "grain_group2") {
           ruin_Label(ctx, "grain");
           ruin_SpacerFillX(ctx);
           ruin_Button(ctx, "70");
       };

        ruin_EndWindow(ctx);
        // ENDS HIERARCHY
        //


        ruin_ComputeLayout(ctx);
        ruin_RaylibRender(ctx);
    };
    CloseWindow();

    free(ctx);
    return 0;
};

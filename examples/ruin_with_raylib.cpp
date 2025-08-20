#include "ruin_widget.h"
#include "ruin_core.h"
#include "../src/ruin_raylib_render.c"
#include "raylib.h"
#include <cstdio>
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
        ruin_RaylibUpdateIO(ctx);
        ruin_PrepareFrame(ctx);


        
        // BEGIN
        ruin_BeginWindow(ctx, "viewer", (ruin_Rect) {.x=400, .y = 100, .h = 200, .w = 200, }, RUIN_WINDOWFLAGS_DRAGABLE);
        ruin_Label(ctx, "Viewer");
        ruin_EndWindow(ctx);
        //
        

        // START HIERARCHY
        ruin_BeginWindow(ctx, "image_editor", (ruin_Rect) {.x=0, .y = 0, .h = 800, .w = 500, }, RUIN_WINDOWFLAGS_DRAGABLE);
        ruin_FontIDStack__Push(&ctx->font_stack, jetbrains_24);
        ruin_Label(ctx, "Image Editor");
        ruin_FontIDStack__Pop(&ctx->font_stack);

        ruin_SameLine(ctx, "grain_group") {
            ruin_Label(ctx, "grain");
            ruin_SpacerFillX(ctx);
            if (ruin_Button(ctx, "20")) { };
        };

        ruin_SameLine(ctx, "blur_group") {
            ruin_Label(ctx, "blur");
            ruin_SpacerFillX(ctx);
            ruin_Button(ctx, "60");
        };
        ruin_SameLine(ctx, "gamma_group") {
            ruin_Label(ctx, "gamma");
            ruin_SpacerFillX(ctx);
            ruin_Button(ctx, "70");
        };
        ruin_EndWindow(ctx);
        // ENDS HIERARCHY
        
        // BEGIN
        ruin_BeginWindow(ctx, "params", (ruin_Rect) {.x=370, .y = 200, .h = 200, .w = 200, }, RUIN_WINDOWFLAGS_DRAGABLE);
        ruin_Label(ctx, "params");
        ruin_EndWindow(ctx);
        //

        printf("\n");


        ruin_ComputeLayout(ctx);
        ruin_RaylibRender(ctx);
    };
    CloseWindow();

    free(ctx);
    return 0;
};

#include "../src/ruin.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "THIS IS A WINDOW");

  SetTargetFPS(60);

  ruin_Context* context = (ruin_Context*)malloc(sizeof(ruin_Context));

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    char buffer[200];
    snprintf(buffer, sizeof(buffer), "you mouse is at (%i, %i) position", GetMouseX(), GetMouseY());

    DrawText(buffer, 20, 20, 40, LIGHTGRAY);

    EndDrawing();
  }
  CloseWindow();

  return 0;
};

void r_draw_rect(ruin_Rect *rect, ruin_Color color) {
  if (rect == NULL)
    return;

  DrawRectangle(rect->x, rect->y, rect->h, rect->w, (Color){ .r = color.r, .g = color.g, .b = color.b, .a = color.a, });
};

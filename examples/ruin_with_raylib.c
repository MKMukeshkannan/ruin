#include "../src/ruin.h"
#include "raylib.h"
#include <stdio.h>

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "THIS IS A WINDOW");

  SetTargetFPS(60);
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
}

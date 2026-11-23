#include "raylib.h"

int main() {
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(720, 480, "LifeSim3D Setup");
  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(Color{12, 12, 18, 255});
    DrawText("LifeSim3D Setup — Stage 1 stub", 40, 40, 24, RAYWHITE);
    DrawText("Full installer UI arrives in Stage 2", 40, 80, 18, LIGHTGRAY);
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) break;
    EndDrawing();
  }
  CloseWindow();
  return 0;
}

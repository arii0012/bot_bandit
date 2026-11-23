#include "Core/Application.hpp"
#include "raylib.h"

int main() {
  SetTraceLogLevel(LOG_WARNING);
  Application app;
  app.Run();
  return 0;
}

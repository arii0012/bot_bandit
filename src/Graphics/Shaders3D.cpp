#include "Shaders3D.hpp"

// Compatible with raylib default vertex shader outputs
static const char* LIT_FS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main() {
  vec4 tex = texture(texture0, fragTexCoord);
  // soft vignette-style light on sprites
  float d = length(fragTexCoord - vec2(0.5));
  float light = 1.15 - d * 0.45;
  float rim = smoothstep(0.35, 0.75, d) * 0.2;
  vec3 rgb = tex.rgb * fragColor.rgb * light + vec3(rim * 0.6, rim * 0.4, rim * 0.3);
  finalColor = vec4(rgb, tex.a * fragColor.a);
}
)";

void Shaders3D::Init() {
  if (!IsWindowReady()) return;
  lit_ = LoadShaderFromMemory(0, LIT_FS);
  outline_ = {0};
  if (lit_.id > 0) {
    ready_ = true;
  }
}

void Shaders3D::Shutdown() {
  if (lit_.id > 0) UnloadShader(lit_);
  lit_ = {0};
  ready_ = false;
}

void Shaders3D::BeginLit() {
  if (ready_) BeginShaderMode(lit_);
}

void Shaders3D::EndLit() {
  if (ready_) EndShaderMode();
}

void Shaders3D::BeginOutline() {}
void Shaders3D::EndOutline() {}

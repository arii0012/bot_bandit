#include "Shaders3D.hpp"

// Vertex: pass position/color/texcoord; fragment: soft lighting + rim
static const char* LIT_VS = R"(#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
uniform mat4 mvp;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragPos;
void main() {
  fragTexCoord = vertexTexCoord;
  fragColor = vertexColor;
  fragPos = vertexPosition;
  gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

static const char* LIT_FS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPos;
uniform sampler2D texture0;
uniform vec3 lightDir;
uniform vec3 ambient;
out vec4 finalColor;
void main() {
  vec4 tex = texture(texture0, fragTexCoord);
  // faux normal from UV gradient for flat sprites
  vec3 n = normalize(vec3(0.15, -0.4, 0.9));
  float diff = max(dot(n, normalize(-lightDir)), 0.0);
  float rim = pow(1.0 - max(dot(n, vec3(0.0, 0.0, 1.0)), 0.0), 2.5) * 0.35;
  vec3 lit = ambient + vec3(diff) * 0.85 + vec3(rim, rim * 0.7, rim * 0.5);
  finalColor = vec4(tex.rgb * fragColor.rgb * lit, tex.a * fragColor.a);
}
)";

static const char* OUTLINE_FS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main() {
  vec4 c = texture(texture0, fragTexCoord) * fragColor;
  // slight contrast boost for "depth"
  float l = dot(c.rgb, vec3(0.299, 0.587, 0.114));
  vec3 boosted = mix(c.rgb, c.rgb * 1.15, 0.4);
  finalColor = vec4(boosted, c.a);
}
)";

void Shaders3D::Init() {
  if (!IsWindowReady()) return;
  lit_ = LoadShaderFromMemory(LIT_VS, LIT_FS);
  outline_ = LoadShaderFromMemory(nullptr, OUTLINE_FS); // default VS
  if (lit_.id > 0) {
    locLightDir_ = GetShaderLocation(lit_, "lightDir");
    locAmbient_ = GetShaderLocation(lit_, "ambient");
    float dir[3] = {-0.4f, -0.7f, -0.5f};
    float amb[3] = {0.35f, 0.35f, 0.42f};
    SetShaderValue(lit_, locLightDir_, dir, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit_, locAmbient_, amb, SHADER_UNIFORM_VEC3);
    ready_ = true;
  }
}

void Shaders3D::Shutdown() {
  if (lit_.id > 0) UnloadShader(lit_);
  if (outline_.id > 0) UnloadShader(outline_);
  ready_ = false;
}

void Shaders3D::BeginLit() {
  if (ready_) BeginShaderMode(lit_);
}

void Shaders3D::EndLit() {
  if (ready_) EndShaderMode();
}

void Shaders3D::BeginOutline() {
  if (outline_.id > 0) BeginShaderMode(outline_);
}

void Shaders3D::EndOutline() {
  if (outline_.id > 0) EndShaderMode();
}

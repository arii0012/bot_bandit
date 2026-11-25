#pragma once
#include "raylib.h"
#include <string>

class ShaderFX {
public:
  static ShaderFX& Get() {
    static ShaderFX inst;
    return inst;
  }

  void Init() {
    if (ready_) return;
    if (FileExists("shaders/postprocess.vs") && FileExists("shaders/postprocess.fs")) {
      post_ = LoadShader("shaders/postprocess.vs", "shaders/postprocess.fs");
      locTime_ = GetShaderLocation(post_, "time");
      locRain_ = GetShaderLocation(post_, "rain");
      locVig_ = GetShaderLocation(post_, "vignette");
      locBloom_ = GetShaderLocation(post_, "bloomProxy");
    }
    if (FileExists("shaders/character.vs") && FileExists("shaders/character.fs")) {
      character_ = LoadShader("shaders/character.vs", "shaders/character.fs");
    }
    if (FileExists("shaders/wetasphalt.vs") && FileExists("shaders/wetasphalt.fs")) {
      wet_ = LoadShader("shaders/wetasphalt.vs", "shaders/wetasphalt.fs");
      locWetTime_ = GetShaderLocation(wet_, "time");
      locWetness_ = GetShaderLocation(wet_, "wetness");
    }
    ready_ = true;
  }

  void Shutdown() {
    if (post_.id > 0) UnloadShader(post_);
    if (character_.id > 0) UnloadShader(character_);
    if (wet_.id > 0) UnloadShader(wet_);
    ready_ = false;
  }

  void BeginPost(float rain01, float vignette = 0.85f, float bloom = 0.35f) {
    if (post_.id == 0) return;
    float t = static_cast<float>(GetTime());
    SetShaderValue(post_, locTime_, &t, SHADER_UNIFORM_FLOAT);
    SetShaderValue(post_, locRain_, &rain01, SHADER_UNIFORM_FLOAT);
    SetShaderValue(post_, locVig_, &vignette, SHADER_UNIFORM_FLOAT);
    SetShaderValue(post_, locBloom_, &bloom, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(post_);
  }

  void EndPost() {
    if (post_.id == 0) return;
    EndShaderMode();
  }

  Shader& CharacterShader() { return character_; }
  Shader& WetShader() { return wet_; }

  void SetWet(float wetness) {
    if (wet_.id == 0) return;
    float t = static_cast<float>(GetTime());
    SetShaderValue(wet_, locWetTime_, &t, SHADER_UNIFORM_FLOAT);
    SetShaderValue(wet_, locWetness_, &wetness, SHADER_UNIFORM_FLOAT);
  }

private:
  Shader post_{};
  Shader character_{};
  Shader wet_{};
  int locTime_ = -1, locRain_ = -1, locVig_ = -1, locBloom_ = -1;
  int locWetTime_ = -1, locWetness_ = -1;
  bool ready_ = false;
};

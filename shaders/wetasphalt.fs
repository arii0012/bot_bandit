#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;
uniform float wetness; // 0..1
uniform vec2 resolution;
out vec4 finalColor;

void main() {
  vec2 uv = fragTexCoord;
  vec4 base = texture(texture0, uv) * colDiffuse * fragColor;
  // fake reflection scroll
  vec2 ru = uv + vec2(sin(uv.y * 40.0 + time) * 0.002, cos(uv.x * 30.0 + time * 0.7) * 0.002);
  vec4 refl = texture(texture0, ru);
  float road = smoothstep(0.35, 0.75, uv.y);
  vec3 wet = mix(base.rgb, refl.rgb * vec3(0.55, 0.65, 0.9), wetness * road * 0.55);
  // neon streak
  float streak = pow(max(0.0, sin(uv.x * 20.0 + time * 2.0)), 12.0) * wetness * road;
  wet += vec3(0.2, 0.7, 1.0) * streak * 0.25;
  wet += vec3(1.0, 0.2, 0.35) * streak * 0.12;
  finalColor = vec4(wet, base.a);
}

#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform float time;
uniform float rain;
uniform float vignette;
uniform float bloomProxy;
out vec4 finalColor;

void main() {
  vec2 uv = fragTexCoord;
  vec3 col = texture(texture0, uv).rgb;
  // cheap bloom proxy - brighten highlights
  float lum = dot(col, vec3(0.299, 0.587, 0.114));
  col += col * max(0.0, lum - 0.6) * bloomProxy;
  // vignette
  vec2 vc = uv - 0.5;
  float vig = 1.0 - dot(vc, vc) * vignette;
  col *= vig;
  // rain streaks
  if (rain > 0.01) {
    float streak = fract(sin(uv.x * 120.0) * 4000.0);
    float drop = step(0.97, fract(uv.y * 30.0 - time * 6.0 + streak));
    col += vec3(0.7, 0.8, 1.0) * drop * rain * 0.35;
  }
  finalColor = vec4(col, 1.0);
}

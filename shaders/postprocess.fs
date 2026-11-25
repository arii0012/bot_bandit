#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform float time;
uniform float rain;
uniform float vignette;
uniform float bloomProxy;
uniform float nitroAberration;
uniform float heatHaze;
out vec4 finalColor;

void main() {
  vec2 uv = fragTexCoord;
  // heat haze from hood
  if (heatHaze > 0.01) {
    uv.x += sin(uv.y * 40.0 + time * 8.0) * 0.002 * heatHaze;
    uv.y += cos(uv.x * 30.0 + time * 6.0) * 0.0015 * heatHaze;
  }
  // chromatic aberration on nitro
  vec2 ca = vec2(nitroAberration * 0.004, 0.0);
  float r = texture(texture0, uv + ca).r;
  float g = texture(texture0, uv).g;
  float b = texture(texture0, uv - ca).b;
  vec3 col = vec3(r, g, b);
  float lum = dot(col, vec3(0.299, 0.587, 0.114));
  col += col * max(0.0, lum - 0.55) * bloomProxy;
  // god ray proxy - radial brighten from top
  vec2 fromSun = uv - vec2(0.5, 0.0);
  float ray = pow(max(0.0, 1.0 - length(fromSun) * 1.2), 3.0) * 0.12 * (1.0 - rain * 0.5);
  col += vec3(1.0, 0.9, 0.7) * ray;
  vec2 vc = uv - 0.5;
  float vig = 1.0 - dot(vc, vc) * vignette;
  col *= vig;
  if (rain > 0.01) {
    float streak = fract(sin(uv.x * 120.0) * 4000.0);
    float drop = step(0.97, fract(uv.y * 30.0 - time * 6.0 + streak));
    col += vec3(0.7, 0.8, 1.0) * drop * rain * 0.35;
  }
  finalColor = vec4(col, 1.0);
}

#version 330
in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform float bloomStrength;
uniform float vignetteStrength;
uniform float motionBlur;
uniform float adrenaline;
uniform vec2 blurDir;
out vec4 finalColor;
void main() {
  vec2 uv = fragTexCoord;
  vec4 color = texture(texture0, uv);
  vec3 bloom = vec3(0.0);
  float wts[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
  for (int i = -4; i <= 4; ++i) {
    float w = wts[min(abs(i), 4)];
    vec3 s = texture(texture0, uv + blurDir * float(i) * 0.002).rgb;
    float lum = dot(s, vec3(0.299, 0.587, 0.114));
    bloom += s * w * step(0.55, lum);
  }
  color.rgb += bloom * bloomStrength;
  if (motionBlur > 0.001) {
    vec3 acc = color.rgb;
    for (int i = 1; i <= 4; ++i)
      acc += texture(texture0, uv - blurDir * float(i) * 0.004 * motionBlur).rgb;
    color.rgb = acc / 5.0;
  }
  float dist = distance(uv, vec2(0.5));
  color.rgb *= mix(1.0, smoothstep(0.9, 0.3, dist), vignetteStrength);
  if (adrenaline > 0.01) {
    color.r *= 1.0 + adrenaline * 0.15;
    color.b *= 1.0 - adrenaline * 0.08;
  }
  finalColor = color;
}

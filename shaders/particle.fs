#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform float softEdge;
out vec4 finalColor;
void main() {
  vec4 texel = texture(texture0, fragTexCoord);
  // soft circular particle
  vec2 c = fragTexCoord - vec2(0.5);
  float d = length(c) * 2.0;
  float alpha = smoothstep(1.0, softEdge, d);
  vec4 col = texel * fragColor;
  col.a *= (1.0 - alpha) * fragColor.a;
  if (col.a < 0.01) discard;
  finalColor = col;
}

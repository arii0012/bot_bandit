#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragPos;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 rimColor;
uniform float rimPower;
uniform vec3 lightDir;
uniform float specularStrength;
uniform int materialType; // 0 skin 1 fabric 2 leather 3 metal
out vec4 finalColor;
void main() {
  vec4 texel = texture(texture0, fragTexCoord);
  vec3 base = texel.rgb * colDiffuse.rgb * fragColor.rgb;
  vec3 N = normalize(fragNormal);
  vec3 V = normalize(viewPos - fragPos);
  vec3 L = normalize(-lightDir);
  float diff = max(dot(N, L), 0.15);
  float rim = 1.0 - max(dot(V, N), 0.0);
  rim = pow(rim, rimPower);
  float spec = 0.0;
  vec3 H = normalize(L + V);
  if (materialType == 0) spec = pow(max(dot(N, H), 0.0), 24.0) * specularStrength * 0.3;
  else if (materialType == 1) spec = pow(max(dot(N, H), 0.0), 8.0) * specularStrength * 0.15;
  else if (materialType == 2) spec = pow(max(dot(N, H), 0.0), 32.0) * specularStrength * 0.6;
  else spec = pow(max(dot(N, H), 0.0), 64.0) * specularStrength * 1.2;
  vec3 color = base * diff + vec3(spec) + rimColor * rim * 0.85;
  finalColor = vec4(color, colDiffuse.a * texel.a * fragColor.a);
}

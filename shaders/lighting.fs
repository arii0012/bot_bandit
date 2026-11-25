#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec4 ambient;
uniform float timeOfDay;
uniform int lampCount;
uniform vec3 lampPos[16];
uniform vec3 lampColor[16];
uniform float lampRange[16];
out vec4 finalColor;

void main() {
  vec4 texel = texture(texture0, fragTexCoord);
  vec3 N = normalize(fragNormal);
  vec3 L = normalize(-lightDir);
  float ndl = max(dot(N, L), 0.0);
  float night = 1.0 - smoothstep(6.0, 8.0, timeOfDay) * (1.0 - smoothstep(18.0, 20.5, timeOfDay));
  float isNight = night;
  vec3 dayAmb = ambient.rgb * 0.9;
  vec3 nightAmb = vec3(0.05, 0.05, 0.12);
  vec3 amb = mix(dayAmb, nightAmb, isNight);
  vec3 col = texel.rgb * colDiffuse.rgb * fragColor.rgb;
  vec3 lighting = amb + col * ndl * (0.35 + 0.65 * (1.0 - isNight));
  for (int i = 0; i < 16; ++i) {
    if (i >= lampCount) break;
    vec3 toL = lampPos[i] - fragPosition;
    float dist = length(toL);
    float range = max(lampRange[i], 1.0);
    float atten = clamp(1.0 - dist / range, 0.0, 1.0);
    atten *= atten;
    vec3 ldir = normalize(toL);
    float nd = max(dot(N, ldir), 0.0);
    lighting += lampColor[i] * atten * (0.4 + 0.6 * nd) * isNight;
  }
  // specular from wet road proxy
  vec3 V = normalize(viewPos - fragPosition);
  vec3 H = normalize(L + V);
  float spec = pow(max(dot(N, H), 0.0), 48.0) * 0.25;
  lighting += vec3(spec) * (0.5 + isNight * 0.5);
  lighting += col * isNight * 0.08;
  finalColor = vec4(lighting, colDiffuse.a * texel.a * fragColor.a);
}

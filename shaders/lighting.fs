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
uniform float timeOfDay; // 0..24
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
  // day/night ambient tint
  float night = smoothstep(6.0, 8.0, timeOfDay) * (1.0 - smoothstep(18.0, 20.5, timeOfDay));
  float isNight = 1.0 - night;
  vec3 dayAmb = ambient.rgb * 0.9;
  vec3 nightAmb = vec3(0.05, 0.05, 0.12);
  vec3 amb = mix(nightAmb, dayAmb, night);
  vec3 col = texel.rgb * colDiffuse.rgb * fragColor.rgb;
  vec3 lighting = amb + col * ndl * (0.35 + 0.65 * night);
  // point lamps
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
  // neon boost at night
  lighting += col * isNight * 0.08;
  finalColor = vec4(lighting, colDiffuse.a * texel.a * fragColor.a);
}

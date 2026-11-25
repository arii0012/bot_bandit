#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragPos;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform float time;
out vec4 finalColor;

void main() {
  vec2 uv = fragTexCoord + vec2(sin(time + fragTexCoord.y * 8.0) * 0.01, cos(time * 0.8 + fragTexCoord.x * 6.0) * 0.01);
  vec4 texel = texture(texture0, uv);
  vec3 N = normalize(fragNormal);
  vec3 V = normalize(viewPos - fragPos);
  float fresnel = pow(1.0 - max(dot(V, N), 0.0), 3.0);
  vec3 deep = vec3(0.02, 0.12, 0.22);
  vec3 shallow = vec3(0.15, 0.45, 0.55);
  vec3 water = mix(deep, shallow, fresnel);
  float foam = smoothstep(0.4, 0.7, sin(fragPos.x * 5.0 + time * 2.0) * sin(fragPos.z * 4.0 - time));
  water += vec3(0.8, 0.9, 1.0) * foam * 0.15;
  water = mix(water, texel.rgb * colDiffuse.rgb, 0.25);
  finalColor = vec4(water, 0.78 + fresnel * 0.15);
}

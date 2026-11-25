#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
uniform float time;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragPos;
void main() {
  vec3 pos = vertexPosition;
  pos.y += sin(pos.x * 3.0 + time * 1.5) * 0.03 + cos(pos.z * 2.5 + time * 1.1) * 0.02;
  fragTexCoord = vertexTexCoord;
  fragColor = vertexColor;
  mat3 nmat = mat3(transpose(inverse(matModel)));
  fragNormal = normalize(nmat * vertexNormal);
  fragPos = vec3(matModel * vec4(pos, 1.0));
  gl_Position = mvp * vec4(pos, 1.0);
}

#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform float shininess;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float pointLightIntensity;

out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    vec3 albedo = texel.rgb * colDiffuse.rgb * fragColor.rgb;
    float alpha = texel.a * colDiffuse.a * fragColor.a;

    vec3 N = normalize(fragNormal);
    vec3 L = normalize(-lightDir);
    vec3 V = normalize(viewPos - fragPosition);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), max(shininess, 16.0));
    float rim = pow(1.0 - max(dot(N, V), 0.0), 2.5) * 0.22;

    vec3 toP = pointLightPos - fragPosition;
    float dist = length(toP);
    vec3 Lp = normalize(toP);
    float atten = pointLightIntensity / (1.0 + dist * dist * 0.05);
    float pdiff = max(dot(N, Lp), 0.0) * atten;

    vec3 ambient = ambientColor * albedo;
    vec3 diffuse = lightColor * diff * albedo;
    vec3 specular = lightColor * spec * 0.4 * albedo;
    vec3 point = pointLightColor * pdiff * albedo;
    vec3 rimCol = lightColor * rim;

    finalColor = vec4(ambient + diffuse + specular + point + rimCol, alpha);
}

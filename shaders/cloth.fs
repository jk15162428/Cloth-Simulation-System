#version 330 core

out vec4 color;

in vec3 position;
in vec2 texCoord;
in vec3 normal;

// Texture Sampler
uniform sampler2D Texture1;
uniform sampler2D Texture2;

uniform vec3 lightPosition;
uniform vec3 lightColor;

void main()
{
    // Ambient
    float ambientStrength = 0.5f;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 lightDir = normalize(lightPosition - position);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // texture() will output the color obtained by sampling the texture with configured conditions
    color = mix(texture(Texture1, texCoord), texture(Texture2, texCoord), 0.0);
    vec3 objectColor = vec3(color.x, color.y, color.z);
    vec3 result = (ambient + diffuse) * objectColor;
    color = vec4(result, 1.0f);
}

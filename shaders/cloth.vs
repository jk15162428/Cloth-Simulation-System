#version 330 core
layout (location = 0) in vec3 vsPosition;
layout (location = 1) in vec2 vsTexCoord;
layout (location = 2) in vec3 vsNormal;

out vec3 position;
out vec2 texCoord;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    position = vsPosition;
    normal = vsNormal;
    gl_Position = projection * view * model * vec4(vsPosition, 1.0f);
    texCoord = vec2(vsTexCoord.x, vsTexCoord.y);
}
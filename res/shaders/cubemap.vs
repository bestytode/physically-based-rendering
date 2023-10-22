#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 worldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    worldPos = aPos;  // Assume mat4(1.0f) for model matrix
    gl_Position =  projection * view * vec4(worldPos, 1.0);
}

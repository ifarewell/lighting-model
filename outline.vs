#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
    vec3 outlinePos = aPos + 0.01 * Normal;
    gl_Position = projection * view * model * vec4(outlinePos, 1.0);
}
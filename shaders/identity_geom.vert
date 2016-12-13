#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT {
    vec3 Normal;
} vs_out;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

void main()
{
    vec4 pos = proj * view * model * vec4(position, 1.0);
    vec4 nrm = proj * view * model * vec4(normal, 0.0);

    gl_Position = pos;

    vs_out.Normal = normalize(vec3(nrm));
}

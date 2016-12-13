#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 ViewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view *  model * vec4(position, 1.0);
    vec4 vp     = (view * model * vec4(position, 1.0));
    // ViewPos     = (view * model * vec4(position, 1.0)).xyz;
    ViewPos     = vp.xyz / vp.w;
}

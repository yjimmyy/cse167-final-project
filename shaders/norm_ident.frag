#version 330 core

in vec3 Normal;
in vec3 Position;
out vec4 color;

void main()
{
    vec3 n = normalize(Normal);
    color = vec4(n.x, n.y, n.z, 1.0);
}

#version 330 core

in vec3 ViewPos;

out vec4 PosOut;

void main()
{
    PosOut = vec4(ViewPos, 1.0);
}

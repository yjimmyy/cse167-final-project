#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(position, 1.0);
    TexCoord = (position.xy + vec2(1.0)) / 2.0;
    // TexCoord = tex_coord;
}

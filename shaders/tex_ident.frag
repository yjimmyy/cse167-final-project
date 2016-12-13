#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D tex;

void main() {
    vec4 tex_color = texture(tex, TexCoords);
    color = tex_color;
}

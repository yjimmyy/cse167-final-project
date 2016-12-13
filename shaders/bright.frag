#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D tex;

void main() {
    vec4 tex_color = texture(tex, TexCoords);

    float brightness =
        (tex_color.x * 0.2126f) +
        (tex_color.y * 0.7152f) +
        (tex_color.z * 0.0722f);

    color = tex_color * pow(brightness, 3.0);
}

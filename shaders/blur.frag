#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D tex;
uniform int dimention;

const float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(tex, 0); // gets size of single texel

    vec3 result = texture(tex, TexCoords).rgb * weight[0];

    // blur in x direction
    if(dimention == 0) {
        for (int i = 1; i < 5; ++i) {
            result += texture(tex, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(tex, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    // blur in y direction
    else if(dimention == 1) {
        for (int i = 1; i < 5; ++i) {
            result += texture(tex, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(tex, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    else {
        result = vec3(1.0, 0.25, 0.5);
    }

    color = vec4(result, 1.0);
}

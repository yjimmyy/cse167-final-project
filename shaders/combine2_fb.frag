#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D fba;
uniform sampler2D fbb;
uniform sampler2D fbc;
uniform sampler2D fbd;

uniform int debug_channels = 0;
uniform int b_is_single_channel;

void main() {
    vec2 fb_dims = textureSize(fba, 0);
    float x_norm = gl_FragCoord.x / fb_dims.x;
    float y_norm = gl_FragCoord.y / fb_dims.y;

    vec4 colora = texture(fba, TexCoords);
    vec4 colorc = texture(fbc, TexCoords);
    vec4 colord = texture(fbd, TexCoords);

    if(b_is_single_channel != 0) {
        float colorb = texture(fbb, TexCoords).r;

        if(debug_channels == 1) {
            if(x_norm < (0.5)) {
                if(y_norm < 0.5f) {
                    // bottom left: SSAO channel
                    // | | |
                    // |*| |
                    color = vec4(vec3(colorb), 1.0);
                }
                else {
                    // top left: only SSAO
                    // |*| |
                    // | | |
                    color = colora * colorb;
                }
            }
            else {
                if(y_norm < 0.5f) {
                    // bottom right: only brightness channel
                    // | | |
                    // | |*|

                    color = colorc;
                }
                else {
                    // top right: all composited
                    // | |*|
                    // | | |
                    color = (colora * colorb) + colorc;
                }
            }
        }
        else {
            // don't debug channels
            color = (colora * colorb) + colorc * 0.7f;
        }

        color = color - vec4(vec3(0.15f), 1.0f);
        color += colord * 0.5;
        // color = vec4(vec3(colorb), 1.0);
    }
    else {
        color = colora + texture(fbb, TexCoords) + colorc;
    }
}

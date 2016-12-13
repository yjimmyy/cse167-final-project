#version 330

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gPositionMap;
uniform float gSampleRad;
uniform mat4 gProj;

const int MAX_KERNEL_SIZE = 48;
uniform vec3 gKernel[MAX_KERNEL_SIZE];

void main()
{
    vec3 Pos = texture(gPositionMap, TexCoord).xyz;

    if(Pos.z == 0.0f) {
        // ignore background, multiply by 1
        FragColor = vec4(1.0f);
    }
    else {
        float AO = 0.0;

        for (int i = 0 ; i < MAX_KERNEL_SIZE ; i++) {
            vec3 samplePos = Pos + gKernel[i]; // generate a random point
            vec4 offset = vec4(samplePos, 1.0); // make it a 4-vector
            offset = gProj * offset; // project on the near clipping plane
            offset.xy /= offset.w; // perform perspective divide
            offset.xy = (offset.xy + vec2(1.0)) / 2.0; // transform to (0,1) range

            float sampleDepth = texture(gPositionMap, offset.xy).b;

            if (abs(Pos.z - sampleDepth) < gSampleRad) {
                AO += step(sampleDepth, samplePos.z);
            }
        }

        float kern_size = float(MAX_KERNEL_SIZE);
        // AO = 1.0 - AO/kern_size;
        AO /= kern_size;

        FragColor = vec4(pow(AO, 0.5));
    }

}

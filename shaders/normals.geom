#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;
// layout (points, max_vertices = 1) out;

in VS_OUT {
    vec3 Normal;
    // vec3 Position;
} gs_in[];

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

void main()
{
    vec4 viewport_pos = gl_in[0].gl_Position;

    gl_Position = viewport_pos;
    EmitVertex();

    vec3 norm = normalize(gs_in[0].Normal);
    gl_Position = viewport_pos + vec4(norm * 0.1f, 0.0f);
    EmitVertex();

    EndPrimitive();
}

#version 330 core

in vec3 Normal;
in vec3 FragPos;
out vec4 color;

uniform bool is_light;

void main()
{
    vec3 n = Normal;
    vec3 fp = FragPos;

  if (is_light) {
    color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  } else {
    color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  }
}

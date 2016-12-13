#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D screen_texture;
uniform vec2 light_pos;

const float exposure = 0.0034f;
const float decay = 1.0f;
const float density = 0.84f;
const float weight = 5.65f;
const int NUM_SAMPLES = 50;

void main()
{
  //vec2 light_pos = vec2(0.0f, 0.0f);
  float step_scale = 1.0f / float(NUM_SAMPLES) * density;
  vec2 step = vec2(TexCoords - light_pos) * step_scale;
  vec2 tex_pos = TexCoords;

  float illumination_decay = 1.0f;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    tex_pos -= step;
    vec4 sample = texture(screen_texture, tex_pos) * illumination_decay * weight;
    color += sample;
    illumination_decay *= decay;
  }

  if (distance(light_pos, TexCoords) < 0.01f) {
    color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    color *= exposure;
  }
}

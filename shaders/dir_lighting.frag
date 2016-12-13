#version 330 core

in vec3 Normal;
in vec3 FragPos;
out vec4 color;

struct DirLight {
    vec3 pos;
    vec3 color;
};

uniform vec3      obj_color;
uniform DirLight  dir_light;
uniform vec3      cam_pos;

// uniform vec2      screen_size;
// uniform sampler2D gAOMap;

const float ambient_strength  = 0.1f;
const float diffuse_strength  = 1.0f;
const float specular_strength = 0.5f;

// vec2 get_screen_texcoord() {
//     return gl_FragCoord.xy / screen_size;
// }

void main()
{
    // Ambient
    vec3 ambient = ambient_strength * dir_light.color;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(dir_light.pos - FragPos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * dir_light.color * diffuse_strength;

    // Specular
    vec3 view_dir = normalize(cam_pos - FragPos);
    vec3 reflectDir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflectDir), 0.0), 32);
    vec3 specular = specular_strength * spec * dir_light.color;

    vec3 result = (ambient + diffuse + specular) * obj_color;

    // float ao_color = texture(gAOMap, get_screen_texcoord()).r;
    // result = result * ao_color;
    color = vec4(result, 1.0f);

}

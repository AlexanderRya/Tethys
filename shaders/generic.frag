#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec2 uvs;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec3 frag_pos;
layout (location = 3) in vec3 view_pos;

layout (location = 0) out vec4 frag_color;

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

layout (std430, set = 0, binding = 2) buffer readonly PointLights {
    PointLight[] point_lights;
};

layout (set = 0, binding = 3) uniform sampler2D[] textures;

layout (push_constant) uniform Indices {
    int transform_index;
    int material_index;
};

vec3 apply_point_light(vec3 color, PointLight light) {
    // Ambient
    vec3 ambient = color * light.ambient;

    // Diffuse
    vec3 normalized = normalize(normals);
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(normalized, light_dir), 0.0);
    vec3 diffuse = light.diffuse * (diff * color);

    // Specular
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normalized);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 64);
    vec3 specular = light.specular * (spec * color);

    return ambient + diffuse + specular;
}

void main() {
    vec3 color = texture(textures[material_index], uvs).rgb;

    for (int i = 0; i < 1; ++i) {
        color = apply_point_light(color, point_lights[i]);
    }

    frag_color = vec4(color, 1.0);
}

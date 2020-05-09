#version 460
#extension GL_EXT_nonuniform_qualifier : enable

const float ambient = 0.1;

layout (location = 0) in vertex_out {
    vec3 vertex_pos;
    vec3 frag_pos;
    vec2 uvs;
    vec3 view_pos;
    vec3 normals;
    mat3 TBN;
};

layout (location = 0) out vec4 frag_color;

struct Falloff {
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct PointLight {
    vec3 position;
    vec3 color;
    Falloff falloff;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

layout (set = 0, binding = 2) uniform sampler2D[] textures;

layout (std140, set = 1, binding = 0) buffer readonly PointLights {
    PointLight[] point_lights;
};

layout (std140, set = 1, binding = 1) buffer readonly DirectionalLights {
    DirectionalLight[] directional_lights;
};

layout (push_constant) uniform Constants {
    uint transform_index;
    uint albedo_index;
    uint specular_index;
    uint normal_index;
    uint point_lights_count;
    uint directional_lights_count;
};

vec3 apply_point_light(PointLight light, vec3 color, vec3 specular, vec3 normal, vec3 view_dir);
vec3 apply_directional_light(DirectionalLight light, vec3 color, vec3 specular, vec3 normal, vec3 view_dir);

void main() {
    vec3 result = vec3(1.0);

    vec3 albedo = texture(textures[albedo_index], uvs).rgb;
    vec3 specular = texture(textures[specular_index], uvs).rgb;
    vec3 normal = normal_index == 2 ? normals : normalize(TBN * (2.0 * texture(textures[normal_index], uvs).rgb - 1.0));

    result = albedo * ambient;

    vec3 view_dir = normalize(view_pos - frag_pos);

    for (uint i = 0; i < point_lights_count; ++i) {
        result += apply_point_light(point_lights[i], albedo, specular, normal, view_dir);
    }

    for (uint i = 0; i < directional_lights_count; ++i) {
        result += apply_directional_light(directional_lights[i], albedo, specular, normal, view_dir);
    }

    frag_color = vec4(result, 1.0);
}

vec3 apply_point_light(PointLight light, vec3 color, vec3 specular, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(light.position - frag_pos);

    // Diffuse
    float diff = max(dot(normal, light_dir), 0.0);

    // Specular
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 64);

    // Attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.falloff.constant + (light.falloff.linear * distance) + (light.falloff.quadratic * (distance * distance)));

    // Combine
    vec3 result_diffuse = light.color * diff * color;
    vec3 result_specular = light.color * spec * specular;

    return (result_diffuse + result_specular) * attenuation * light.falloff.intensity;
}

vec3 apply_directional_light(DirectionalLight light, vec3 color, vec3 specular, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);

    // Diffuse
    float diff = max(dot(normal, light_dir), 0.0);

    // Specular
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 64);

    // Combine
    vec3 result_diffuse = light.color * diff * color;
    vec3 result_specular = light.color * spec * specular;

    return result_diffuse + result_specular;
}
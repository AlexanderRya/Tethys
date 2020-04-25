#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec2 uvs;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec3 frag_pos;
layout (location = 3) in vec3 view_pos;

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

layout (push_constant) uniform Indices {
    uint transform_index;
    uint diffuse_index;
    uint specular_index;
    uint normal_index;
    uint point_lights_count;
    uint directional_lights_count;
};

vec3 apply_point_light(PointLight light, vec3 color, vec3 specular, vec3 normal, vec3 view_dir);
vec3 apply_directional_light(DirectionalLight light, vec3 color, vec3 specular, vec3 normal, vec3 view_dir);

void main() {
    vec3 result = vec3(1.0);
    vec4 color = texture(textures[diffuse_index], uvs).rgba;
    vec3 diffuse = color.rgb;
    float alpha = color.a;
    vec3 specular = texture(textures[specular_index], uvs).rgb;

    result = diffuse * vec3(0.1); // Ambient

    vec3 norms = normalize(normals);
    vec3 view_dir = normalize(view_pos - frag_pos);

    for (uint i = 0; i < point_lights_count; ++i) {
        result += apply_point_light(point_lights[i], diffuse, specular, norms, view_dir);
    }

    for (uint i = 0; i < directional_lights_count; ++i) {
        result += apply_directional_light(directional_lights[i], diffuse, specular, norms, view_dir);
    }

    frag_color = vec4(result, 1.0);
}

vec3 apply_point_light(PointLight light, vec3 color, vec3 specular, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(light.position - frag_pos);

    // Diffuse
    float diff = max(dot(normal, light_dir), 0.0);

    // Specular
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 32);

    // Attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.falloff.constant + light.falloff.linear * distance + light.falloff.quadratic * (distance * distance));

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
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 32);

    // Combine
    vec3 result_diffuse = light.color * diff * color;
    vec3 result_specular = light.color * spec * specular;

    return result_diffuse + result_specular;
}
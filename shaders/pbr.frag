#version 460
#extension GL_EXT_nonuniform_qualifier : enable

const float ambient = 0.1;
const float pi = 3.1415926535897932384626433;

layout (location = 0) in vertex_out {
    vec3 vertex_pos;
    vec3 frag_pos;
    vec2 uvs;
    vec3 view_pos;
    vec3 normals;
};

layout (location = 0) out vec4 frag_color;

struct PointLight {
    vec4 position;
    vec4 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec4 direction;
    vec4 color;
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
    uint metallic_index;
    uint normal_index;
    uint roughness_index;
    uint occlusion_index;
    uint point_lights_count;
    uint directional_lights_count;
};

vec3 process_normal_map(vec3 normal_map);
float process_ggx_distribution(vec3 N, vec3 H, float roughness);
float process_geometry_schlick_ggx(float NV, float roughness);
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnel_schlick(float cos_theta, vec3 F0);

void main() {
    vec3 result = vec3(1.0);

    vec3 albedo = texture(textures[albedo_index], uvs).rgb;
    float metallic = texture(textures[metallic_index], uvs).r;
    vec3 norms = process_normal_map(texture(textures[normal_index], uvs).xyz);
    float roughness = texture(textures[roughness_index], uvs).r;
    float occlusion = texture(textures[occlusion_index], uvs).r;

    vec3 V = normalize(view_pos - frag_pos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < point_lights_count; ++i) {
        PointLight light = point_lights[i];

        // Per-light radiance
        vec3 L = normalize(vec3(light.position) - frag_pos);
        vec3 H = normalize(V + L);
        float distance = length(vec3(light.position) - frag_pos);
        float attenuation = 1.0 / (light.constant + (light.linear * distance) + (light.quadratic * (distance * distance)));
        vec3 radiance = vec3(light.color) * attenuation * light.intensity;

        // Cook-Torrance BRDF
        float NDF = process_ggx_distribution(norms, H, roughness);
        float G = geometry_smith(norms, V, L, roughness);
        vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);
        vec3 specular = (NDF * G * F) / (4 * max(dot(norms, V), 0.0) * max(dot(norms, L), 0.0) + 0.001);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
        float NL = max(dot(norms, L), 0.0);
        Lo += (kD * albedo / pi + specular) * radiance * NL;
    }

    vec3 ambient = vec3(0.03) * albedo * occlusion;
    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));

    frag_color = vec4(color, 1.0);
}

vec3 process_normal_map(vec3 normal_map) {
    vec3 tangent_normal = normal_map * 2.0 - 1.0;

    vec3 Q1 = dFdx(frag_pos);
    vec3 Q2 = dFdy(frag_pos);
    vec2 st1 = dFdx(uvs);
    vec2 st2 = dFdy(uvs);

    vec3 N = normalize(normals);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangent_normal);
}

float process_ggx_distribution(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NH = max(dot(N, H), 0.0);
    float NH2 = NH * NH;

    float num = a2;
    float den = (NH2 * (a2 - 1.0) + 1.0);
    den = pi * den * den;

    return num / den;
}

float process_geometry_schlick_ggx(float NV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NV;
    float den = NV * (1.0 - k) + k;

    return num / den;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NV = max(dot(N, V), 0.0);
    float NL = max(dot(N, L), 0.0);

    float ggx1 = process_geometry_schlick_ggx(NL, roughness);
    float ggx2 = process_geometry_schlick_ggx(NV, roughness);

    return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}
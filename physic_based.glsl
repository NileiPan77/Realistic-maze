#version 330 core

struct pointLight{
    vec3 position;
    vec3 direction;
    vec3 color;
};

struct spotLight{
    vec3 position;
    vec3 direction;
    float inner_falloff;
    float outter_falloff;

    vec3 color;
};

out vec4 outColor;


in vec3 vertNormal;
in vec3 pos;
in vec2 texcoord;
in vec3 tangentViewPos;
in vec3 tangentFragPos;
in mat3 TBN;
in vec4 shadowCoord;



#define NUM_OF_POINT_LIGHTS 2

uniform int flashOn;
uniform float phong;
uniform vec3 viewPos;
uniform pointLight points[NUM_OF_POINT_LIGHTS];
uniform spotLight spots;

#define MAX_TEXTURES 5

uniform sampler2D texture_diffuse[MAX_TEXTURES];
uniform sampler2D texture_normal[MAX_TEXTURES];
uniform sampler2D texture_specular[MAX_TEXTURES];
uniform sampler2D texture_albedo[MAX_TEXTURES];
uniform sampler2D texture_ao[MAX_TEXTURES];
uniform sampler2D texture_roughness[MAX_TEXTURES];
uniform sampler2D texture_metallic[MAX_TEXTURES];
uniform sampler2D shadowMap;


uniform int texID;
 
vec3 pointComponents(pointLight l, vec3 normal, vec3 Pos, vec3 viewDir, 
float roughness, float metallic, vec3 albedo);
vec3 spotComponents(spotLight l, vec3 normal, vec3 Pos, vec3 viewDir,
float roughness, float metallic, vec3 albedo);

const float PI = 3.14159265359;

float roughnessDistribution(vec3 N, vec3 H, float roughness){
    float a2 = pow(roughness * roughness,2.0);
    float NdotH = max(dot(N,H), 0.0) * max(dot(N,H), 0.0);

    return a2/(PI * pow((NdotH * (a2 - 1.0) + 1.0),2.0));
}
float SchlickApproximation(float NdotV, float roughness){
    return NdotV / (NdotV * (1.0 - ((pow((roughness + 1.0),2.0)) / 8.0)) + ((pow((roughness + 1.0),2.0)) / 8.0));
}
float specApprox(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return SchlickApproximation(NdotV, roughness) * SchlickApproximation(NdotL, roughness);
}
vec3 fresnel(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}


void main()
{
    vec3 tangentNormal = texture(texture_normal[texID], texcoord).xyz * 2.0 - 1.0;

    vec3 N   = normalize(vertNormal);
    vec3 T  = normalize(dFdx(pos)*dFdx(texcoord).t - dFdy(pos)*dFdy(texcoord).t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);


    //tangent space normal
    vec3 norm =  normalize(TBN * tangentNormal);

    vec3 albedo = pow(texture(texture_albedo[texID], texcoord).rgb, vec3(2.2));
    float metallic = texture(texture_metallic[texID], texcoord).r;
    float roughness = texture(texture_roughness[texID], texcoord).r;
    float ao = texture(texture_ao[texID], texcoord).r;    
    
    vec3 viewDir = normalize(viewPos - pos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0,albedo,metallic);

    vec3 pointLightComponent = vec3(0.0);

    float bias = 0.005;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float depthSample = texture(shadowMap, shadowCoord.xy + vec2(x, y) * texelSize).r; 
            shadow += shadowCoord.z - bias > depthSample  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    if(shadowCoord.z > 1.0) shadow = 0.0;


    for(int i = 0; i < NUM_OF_POINT_LIGHTS; i++){
        pointLightComponent += pointComponents(points[i], norm, pos, viewDir,roughness, metallic, albedo);
    }
    vec3 spotLightComponent = spotComponents(spots, norm, pos, viewDir,roughness, metallic, albedo);

    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 colorF = ambient + pointLightComponent * (1-shadow) + spotLightComponent * flashOn;

    colorF = colorF / (colorF + vec3(1.0));
    colorF = pow(colorF, vec3(1.0/2.2));

    outColor = vec4(colorF ,1.0);


}


vec3 pointComponents(pointLight l, vec3 normal, vec3 pos, vec3 viewDir,
    float roughness, float metallic, vec3 albedo){
    vec3 F0 = vec3(0.04);
    F0 = mix(F0,albedo,metallic);

    vec3 lightDir = normalize(l.position - pos);
    vec3 Half = normalize(viewDir + lightDir);
    float distance = length(l.position - pos);
    float atten = 1.0 / (distance * distance);
    vec3 radiance = l.color * atten;

    float NormalDistribution = roughnessDistribution(normal, Half, roughness);
    float S = specApprox(normal, viewDir, lightDir, roughness);
    vec3 F = fresnel(max(dot(Half,viewDir),0.0), F0);

    vec3 specular = (NormalDistribution * S * F) / (4 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001);

    vec3 kD = (vec3(1.0) - F) * (1-metallic);

    float NdotL = max(dot(normal,lightDir), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;

}
vec3 spotComponents(spotLight l, vec3 normal, vec3 pos, vec3 viewDir,
        float roughness, float metallic, vec3 albedo){

    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0,albedo,metallic);

    vec3 lightDir = normalize(l.position - pos);
    vec3 Half = normalize(viewDir + lightDir);
    float distance = length(l.position - pos);
    float atten = 1.0 / (distance * distance);
    vec3 radiance = l.color * atten;

    float NormalDistribution = roughnessDistribution(normal, Half, roughness);
    float s = specApprox(normal, viewDir, lightDir, roughness);
    vec3 F = fresnel(max(dot(Half,viewDir),0.0), F0);

    vec3 specular = (NormalDistribution * s * F) / (4 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

    float NdotL = max(dot(normal,lightDir), 0.0);

    float inner = dot(lightDir,normalize(-l.direction));
    float outter = l.inner_falloff - l.outter_falloff;
    float intensity = clamp((inner - l.outter_falloff)/ outter, 0.0, 1.0);
    
    return (kD * albedo / PI + specular) * radiance * NdotL * intensity;

}
#version 150 core


struct directionLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct pointLight{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct spotLight{
    vec3 position;
    vec3 direction;
    float inner_falloff;
    float outter_falloff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 outColor;


in vec3 vertNormal;
in vec3 pos;
in vec2 texcoord;
in vec3 tangentViewPos;
in vec3 tangentFragPos;
in mat3 TBN;
in vec4 ShadowCoord;
uniform sampler2DShadow shadowMap;


#define NUM_OF_POINT_LIGHTS 2

uniform float phong;
uniform vec3 viewPos;
uniform directionLight dir;
uniform pointLight points[NUM_OF_POINT_LIGHTS];
uniform spotLight spots;

#define MAX_TEXTURES 5

uniform sampler2D texture_diffuse[MAX_TEXTURES];
uniform sampler2D texture_normal[MAX_TEXTURES];
uniform sampler2D texture_specular[MAX_TEXTURES];

uniform int texID;

vec3 directionalComponents(directionLight l, vec3 normal, vec3 viewDir, vec3 color);
vec3 pointComponents(pointLight l, vec3 normal, vec3 Pos, vec3 viewDir, vec3 color);
vec3 spotComponents(spotLight l, vec3 normal, vec3 Pos, vec3 viewDir, vec3 color);

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

void main()
{

    vec3 color = texture(texture_diffuse[texID], texcoord).rgb;
    vec3 norm = texture(texture_normal[texID], texcoord).rgb;
    norm = normalize(norm * 2.0 -1.0);
    //tangent space normal
    
    

    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);

    vec3 oColor = directionalComponents(dir, norm, viewDir, color);

    for(int i = 0; i < NUM_OF_POINT_LIGHTS; i++){
        oColor += pointComponents(points[i], norm, pos, viewDir, color);
    }
    oColor += spotComponents(spots, norm, pos, viewDir, color);


   // oColor = color;
    outColor = vec4(oColor * visibility,1.0);
}
vec3 directionalComponents(directionLight l, vec3 normal, vec3 viewDir, vec3 color){
    l.direction = normalize(TBN * l.direction);

    vec3 diffuseC = color*max(dot(-l.direction,normal),0.0) * l.diffuse;
    vec3 ambC = color*l.ambient;
    vec3 reflectDir = normalize(reflect(l.direction,normal));
    float spec = max(dot(reflectDir,viewDir),0.0);
    vec3 specC = color *l.specular*pow(spec,phong);
    vec3 oColor = ambC+diffuseC+specC;
    return oColor;
}
vec3 pointComponents(pointLight l, vec3 normal, vec3 pos, vec3 viewDir, vec3 color){
    vec3 pos_tangent = TBN * l.position;
    vec3 lightDir_tangent = normalize(pos_tangent - tangentFragPos);


    float lambertian = max(dot(normal,lightDir_tangent),0.0);

    vec3 reflectDir = normalize(reflect(-lightDir_tangent,normal));

    float phongCoeff = pow(max(dot(viewDir, reflectDir),0.0), phong);

    float distance = length(l.position - pos);
    float atten = 1.0 / (1 + 0.09 * distance + 0.032 * distance * distance);

    vec3 ambient = l.ambient * color;
    vec3 diffuse = color *   lambertian;
    vec3 specular = color * l.specular * phongCoeff;

    ambient *= atten;
    diffuse *= atten;
    specular *= atten;

    return (ambient + diffuse + specular);
}
vec3 spotComponents(spotLight l, vec3 normal, vec3 pos, vec3 viewDir, vec3 color){
    vec3 pos_tangent = TBN * l.position;
    vec3 lightDir_tangent = normalize(pos_tangent - tangentFragPos);

    float lambertian = max(dot(normal,lightDir_tangent),0.0);
    vec3 reflectDir = normalize(reflect(-lightDir_tangent,normal));
    float phongCoeff = pow(max(dot(viewDir, reflectDir),0.0), phong);


    float distance = length(l.position - pos);
    float atten = 1.0 / (1 + 0.09 * distance + 0.032 * distance * distance);

    float inner = dot(normalize(l.position - pos),normalize(-l.direction));
    float outter = l.inner_falloff - l.outter_falloff;
    float intensity = clamp((inner - l.outter_falloff)/ outter, 0.0, 1.0);
    
    vec3 ambient = l.ambient * color;
    vec3 diffuse = color *  lambertian;
    vec3 specular = color * l.specular * phongCoeff;

    ambient *= atten;
    diffuse *= atten * intensity;
    specular *= atten* intensity;

    return (ambient + diffuse + specular);
}
#version 150 core

struct pointLight{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 outColor;


in vec3 Color;
in vec3 vertNormal;
in vec3 pos;
in vec2 texcoord;


uniform float phong;
uniform vec3 viewPos;
uniform sampler2D tex0;
uniform sampler2D tex1;

uniform int texID;
uniform pointLight point;

void main()
{
    vec3 color;
    if (texID == -1)
        color = Color;
    else if (texID == 0)
        color = texture(tex0, texcoord).rgb;
    else if (texID == 1)
        color = texture(tex1, texcoord).rgb;  
    else{
        outColor = vec4(1,1,1,1);
        return; //This was an error, stop lighting!
    }
    vec3 ambient = point.ambient * color;

    vec3 normal = normalize(vertNormal);
    vec3 viewDir = normalize(viewPos - pos); 

    vec3 lightDir = normalize(point.position - pos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = point.diffuse * diff * color;

    
    vec3 reflectDir = normalize(reflect(-lightDir, normal));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), phong);
    vec3 specular = point.specular * spec * color;

    float distance = length(point.position - pos);
    float atten = 1.0 / (1 + 0.09 * distance + 0.032 * distance * distance);
    
    ambient *= atten;
    diffuse *= atten;
    specular *= atten;

    vec3 oColor = ambient+diffuse+specular;
    outColor = vec4(oColor,1);
}
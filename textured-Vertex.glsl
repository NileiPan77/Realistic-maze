#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out vec3 Color;
out vec3 vertNormal;
out vec3 pos;
out vec2 texcoord;
out vec4 shadowCoord;

out vec3 tangentViewPos;
out vec3 tangentFragPos;
out mat3 TBN;


uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 inColor;

uniform vec3 viewPos;
uniform mat4 DepthBiasVP;



void main() {
    Color = inColor;

    pos = vec3(model * vec4(position,1.0));
    vertNormal = (mat3(transpose(inverse(model))) * inNormal);
    texcoord = inTexcoord;

    gl_Position = proj * view * model * vec4(position,1.0);
    shadowCoord = DepthBiasVP * model * vec4(position,1.f);


    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * vertNormal);
    T = normalize(T - dot(T,N) * N);
    vec3 B = cross(N,T);

    TBN = transpose(mat3(T,B,N));
    tangentViewPos = TBN * viewPos;
    tangentFragPos = TBN * pos;
}
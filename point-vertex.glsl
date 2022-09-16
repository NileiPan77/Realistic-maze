#version 150 core

in vec3 position;
in vec3 inColor;
in vec3 inNormal;
in vec2 inTexcoord;

out vec3 Color;
out vec3 vertNormal;
out vec3 pos;
out vec3 lightDir;
out vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 inColor;
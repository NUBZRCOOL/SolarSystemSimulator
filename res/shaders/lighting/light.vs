#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

out float vLogDepth;
const float C = 1.0f;

out vec3 LocalPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    vec4 clipPos = proj * view * model * vec4(vPos, 1.0);
    vLogDepth = clipPos.w;
    gl_Position = clipPos;
    TexCoord = aTexCoord;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPos = vec3(model * vec4(vPos, 1.0));

    LocalPos = vPos;
}
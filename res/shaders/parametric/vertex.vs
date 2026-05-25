#version 330 core
layout (location = 0) in vec3 vPos;

uniform mat4 view;
uniform mat4 proj;

out float vLogDepth_VS;

void main() {
    vec4 clipPos = proj * view * vec4(vPos, 1.0f);
    vLogDepth_VS = clipPos.w;
    gl_Position = clipPos;
}
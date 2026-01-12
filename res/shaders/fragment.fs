#version 330 core

out vec4 FragColor;
in vec4 vertexCol;
in vec2 TexCoord;

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float opa;

void main() {
    FragColor = mix(texture(tex1, TexCoord), texture(tex2, TexCoord), opa);
}
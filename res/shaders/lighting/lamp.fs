#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

in float vLogDepth;

uniform sampler2D texture_diffuse1;
uniform vec3 lightCol;


void main() {
    FragColor = texture(texture_diffuse1, TexCoord);
    gl_FragDepth = 1.0 - (log2(vLogDepth + 1.0) / log2(1e12f + 1.0));
}
#version 330 core

out vec4 FragColor;

in float vLogDepth;

void main() {
    FragColor = vec4(1.0f);
    gl_FragDepth = 1.0 - (log2(vLogDepth + 1.0) / log2(1e12f + 1.0));
}
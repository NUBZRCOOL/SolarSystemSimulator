#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices=4) out;

uniform float lineWidth;
uniform vec2 viewportSize;

void main() {

    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;

    vec2 ndc1 = p1.xy / p1.w;
    vec2 ndc2 = p2.xy / p2.w;

    vec2 dir = normalize(ndc2 - ndc1);
    vec2 normal = vec2(-dir.y, dir.x);

    vec2 offset = normal * lineWidth * 2.0 / viewportSize;

    gl_Position = vec4((ndc1 + offset) * p1.w, p1.z, p1.w);
    EmitVertex();

    gl_Position = vec4((ndc1 - offset) * p1.w, p1.z, p1.w);
    EmitVertex();

    gl_Position = vec4((ndc2 + offset) * p2.w, p2.z, p2.w);
    EmitVertex();

    gl_Position = vec4((ndc2 - offset) * p2.w, p2.z, p2.w);
    EmitVertex();

    EndPrimitive();
}
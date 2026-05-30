#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    vec3 newCoords = vec3(-TexCoords.x, TexCoords.y, TexCoords.z);
    FragColor = texture(skybox, newCoords);
}
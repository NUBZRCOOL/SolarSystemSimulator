#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 objCol;
uniform vec3 lightCol;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    float dist = length(lightPos - FragPos);
    float atten = 1.0 / (1.0 + 0.056 * dist + 0.012 * (dist * dist));

    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * texture(texture_diffuse1, TexCoord).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = diff * lightCol * texture(texture_diffuse1, TexCoord).rgb;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightCol * texture(texture_specular1, TexCoord).rgb;

    // diffuse *= atten;
    // specular *= atten;

    vec3 result = (ambient + diffuse + specular);

    FragColor = vec4(result, 1.0);
}
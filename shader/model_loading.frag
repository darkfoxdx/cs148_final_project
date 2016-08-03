#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D oceanHeight;
uniform sampler2D oceanNormal;

void main()
{
    vec3 result = vec3(texture(oceanHeight, TexCoords)) * vec3(0.9, 0.9, 0.9);
    color = vec4(result, 1.0);

    //color = vec4(1.0, 1.0, 1.0, 1.0);
    //color = vec4(Normal, 1.0);
}

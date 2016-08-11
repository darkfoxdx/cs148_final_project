#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec3 PlaneNormal; //position and normal of the caustics plane
out vec3 PlanePosition;
out vec4 ShadowCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 DepthBiasMVP;

void main()
{
    vec3 plane_pos = vec3(0, 5.0, 0.0f);
    vec3 plane_normal = vec3(0, 1.0, 0.0f);

    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoords = texCoords;

    mat3 normalMatrix = mat3(transpose(inverse(model)));

    Normal = normalMatrix * normal;
    PlaneNormal = normalMatrix*plane_normal;

    FragPos = vec3(model * vec4(position, 1.0));
    PlanePosition = vec3(model * vec4(plane_pos, 1.0));

    ShadowCoord = DepthBiasMVP * vec4(FragPos,1);
}

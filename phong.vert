// CS148 Summer 2016 Homework 3 - Shaders

#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // TODO: Your code here
    // Remember to set gl_Position (correctly) or you will get a black screen...

    gl_Position = projection * view * model * vec4(position, 1.0);

    // Getting position in world space
    FragPos = vec3(model * vec4(position, 1.0));

    // Getting normal matrix in world space
    // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalMatrix * normal;
}

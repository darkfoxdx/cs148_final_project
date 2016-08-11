#version 330 core
in float intensity;
layout (location = 0) out vec4 color;

void main()
{
    // The intensity of the photon is controlled via a uniform
    //color = vec4(vec3(1.0,1.0,1.0),intensity);
    color = vec4(1, 0, 0, 1);
}

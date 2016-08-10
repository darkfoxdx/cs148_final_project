#version 330 core
// Input Attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texCoords;

// Input Uniforms
uniform vec3 lightPos;

// To Fragment Shader
out float intensity;

// Indices of refraction
const float kRefractionAir = 1.0; // Real world: 1.000293
const float kRefractionWater = 1.333;
const float kAir2Water = kRefractionAir/kRefractionWater;

const float depth = 5;
const float tileSize = 1;
const float photonIntensity = 1;
const float photonScale = 0.1;

void main(){

    // Render mesh grid as full screen quad
    gl_Position = vec4(position.x/(tileSize/2.0),
                       position.z/(tileSize/2.0), -1.0, 1.0);

    vec3 position =  position;
    // Input positions are in the range (-0.5 to 0.5), offset these to 0 to 1.0
    position.x += 0.5;
    position.z += 0.5;

    // gl_Position is the ocean surface position vector
    // vNormal is the normal of the ocean surface at gl_Position
    // lightPos is the position of the light source in world space

    // Refract the light ray direction vector from the light source to the
    // ocean surface position vector and determine the intersection position
    // of the refracted ray on a flat ocean floor at depth D.

    // Get the light direction vector
    vec3 vLightDirection = lightPos - position;
    vLightDirection = normalize(vLightDirection);
    // Obtain the refracted light ray vector
    vec3 n = normalize(normal);
    vec3 vRefract = refract(vLightDirection, n, kAir2Water);
    // Calculate the distance along the Refraction ray from the ocean surface
    // to the interception point on the ocean floor.
    float distance = (depth - position.y) / vRefract.y;
    // Calculate the interception point of the ray on the ocean floor.
    vec3 vIntercept = ((position + vRefract * distance)/tileSize);

    // Make the caustic texture tileable by using wrap-around co-ordinates
    // Rays that are wrapped around will appear to have originated from a
    // neighbouring ocean tile.
    vIntercept.x = mod(vIntercept.x, 1.0);
    vIntercept.z = mod(vIntercept.z, 1.0);
    if (vIntercept.x < 0.0) {
        vIntercept.x = 1.0 - (abs(vIntercept.x));
    }
    if (vIntercept.z < 0.0) {
        vIntercept.z = 1.0 - (abs(vIntercept.z));
    }

    // The intensity of the photon is controlled via a uniform
    intensity = photonIntensity/256.0; // Intensity contribution
    // Set the position of the GL_POINT according to the interception point
//    gl_Position.x = (vIntercept.x*2.0)-1.0;
//    gl_Position.y = (vIntercept.z*2.0)-1.0;
    // The ray always strikes the ocean floor to create a planar field of points
//    gl_Position.z = 0.0;
    // The point size is controlled via a uniform
    //gl_PointSize = photonScale;

    gl_Position = vec4(texCoords.x, texCoords.y, -1, 0);
}

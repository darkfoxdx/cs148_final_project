#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 PlaneNormal; //position and normal of the caustics plane
in vec3 PlanePosition;

out vec4 color;

//uniform sampler2D causticTexture;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D oceanHeight;
uniform sampler2D oceanNormal;
uniform sampler2D lightMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

vec3 raytrace(vec3 plane_center, vec3 plane_normal, vec3 ray_origin, vec3 ray_direction){
    float t=dot(plane_normal,plane_center-ray_origin)/dot(plane_normal, ray_direction);
	vec3 position = ray_origin + t * ray_direction;
    return position;
}

void main()
{
    vec3 hit_pos = raytrace(PlanePosition, PlaneNormal, FragPos, Normal);

    vec4 light, height_map, normal_map;
    float oceanBoundary = 5;
        height_map = texture(oceanHeight, vec2(hit_pos.x/oceanBoundary+0.5, hit_pos.z/oceanBoundary+0.5));
        normal_map = texture(oceanNormal, vec2(hit_pos.x/oceanBoundary+0.5, hit_pos.z/oceanBoundary+0.5));
        //move the light
        hit_pos.y+=height_map.y;

        vec3 lightDir = normalize(hit_pos - lightPos);
        float c1 = dot(vec3(normal_map), lightDir);
        float refractionRatio = 1.3;
        float c2 = sqrt(1 - (refractionRatio * refractionRatio) * (1 - (c1*c1)));
        vec3 refractionDir = (refractionRatio * lightDir) + (refractionRatio * c1 - c2) * vec3(-normal_map);

        vec3 hit_pos2 = raytrace(lightPos, lightDir, hit_pos, refractionDir);

        light = texture(lightMap, vec2(hit_pos2.x/50+0.5, hit_pos2.z/50+0.5));

    color = light;//*lighting;//*vec4(hit_pos, 1);
    //vec4 colorResult = vec4((ambient + diffuse + specular), 1.0);
    //color=colorResult;
    //color = texture(lightMap, TexCoords);
}

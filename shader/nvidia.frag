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
    vec4 final_color;
    float total = 0;
    float range = 0.8;
    float steps = range/3;
    for(float i=-range; i<range; i+=steps){
    for(float j=-range; j<range; j+=steps){
    for(float k=-range; k<range; k+=steps){
        vec3 hit_pos = raytrace(PlanePosition, PlaneNormal, FragPos, vec3(Normal.x+i, Normal.y+j, Normal.z+k));
        vec4 light, height_map, normal_map;
        float oceanBoundary = 2;
        height_map = texture(oceanHeight, vec2(hit_pos.x/oceanBoundary/2+0.5, hit_pos.z/oceanBoundary/2+0.5));
        normal_map = texture(oceanNormal, vec2(hit_pos.x/oceanBoundary/2+0.5, hit_pos.z/oceanBoundary/2+0.5));
        //if(length(hit_pos - PlanePosition)>oceanBoundary){
        //light = vec4(0.0, 0.0, 0.0, 1.0);
        //}else{
        //    light = height_map;
        //}

        //move the light
        hit_pos.y+=height_map.y;

        //vec3 lightDir = normalize(hit_pos - lightPos);
        vec3 lightDir = normalize(lightPos - FragPos);

        float c1 = dot(vec3(normal_map), lightDir);
        float refractionRatio = 0.7;
        float c2 = sqrt(1 - (refractionRatio * refractionRatio) * (1-c1*c1));
        vec3 refractionDir =  ((refractionRatio * c1 + c2) * vec3(-normal_map))-(lightDir*refractionRatio);


        vec3 hit_pos2 = raytrace(lightPos, lightDir, hit_pos, refractionDir);

        float light_x_range = 10;
        float light_z_range = 10;

        if(hit_pos2.x>light_x_range){
            hit_pos2.x=light_x_range;
        }
        if(hit_pos2.z>light_z_range){
            hit_pos2.z=light_z_range;
        }
        if(hit_pos2.x<-light_x_range){
            hit_pos2.x=-light_x_range;
        }
        if(hit_pos2.z<-light_z_range){
            hit_pos2.z=-light_z_range;
        }

        light = texture(lightMap, vec2(hit_pos2.x/light_x_range/2+0.5, hit_pos2.z/light_z_range/2+0.5));
        float intensity = light.r;
        //float light_strength = length(hit_pos2)/10;
        final_color += light;
        total+=1.0;

    }}}
    //*lighting;//*vec4(hit_pos, 1);
    //vec4 colorResult = vec4((ambient + diffuse + specular), 1.0);
    //color=colorResult;
    //color = texture(lightMap, TexCoords);
    //color = texture(oceanNormal, TexCoords);

    // Sample the ocean floor texture
    vec4 fragColour = vec4(1, 0, 0, 1);
    vec4 waterColour = vec4(0, 1, 1, 1);
    // Apply distance fog
    float fogFactor = 0.8;
	fragColour = fragColour * (1.0-fogFactor) + waterColour * (fogFactor);
    // Sample the caustic texture
    vec4 caustic = final_color/total;
    float colorCaustic = min(0.7, caustic.r);
    // Apply the caustic texture
    fragColour += vec4(colorCaustic, colorCaustic, colorCaustic, 1);
    fragColour.a = 1.0;
    // Output the colour
    color = fragColour;
}

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
// Ambient light
	// Ambient strength multiply with lightColor
	vec3 ambient = 0.05 * lightColor;

	// Diffuse
    // Light direction towards the light position
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 normal = normalize(Normal);
	// Max is used because if the dot product is negative,
	// set to 0 to prevent negative diffuse.
	// Calculate the cos theta of the lightDir and normal
	float diffCosTheta = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diffCosTheta * lightColor;

	// Specular
	// Direction of fragment towards the eye
	vec3 viewDir = normalize(viewPos - FragPos);
	// Getting the vector of the reflected ray
	// Using negative lightDir so it points towards the fragment
	vec3 reflectDir = reflect(-lightDir, normal);

	// Calculate the cos theta of the viewDir and reflectDir
	// and make sure it isn't negative
	float specCosTheta = max(dot(viewDir, reflectDir), 0.0);
	// The specCosTheta is raised to the power of the shininess.
    float spec = pow(specCosTheta, 64);

    // Spec multiply by the spec strength and light color
	vec3 specular = 0.5 * spec * lightColor;

    vec3 hit_pos = raytrace(PlanePosition, PlaneNormal, FragPos, Normal);

	//vec4 colorResult = vec4((ambient + diffuse + specular), 1.0);//*vec4(texture(causticTexture, TexCoords));
    //vec3 result = vec3(texture(oceanNormal, TexCoords)) * vec3(0.9, 0.9, 0.9);
    //color = vec4(result, 1.0);

    /*
    vec4 light;
    if(length(hit_pos - PlanePosition)>1.0){
        light = vec4(1, 1, 1, 1.0);
    }else{
        light = vec4(texture(causticTexture, vec2(hit_pos.x, hit_pos.y)));
    }
    color = light;//*vec4(hit_pos, 1);
    */
    vec4 lighting = vec4((ambient + diffuse + specular), 1.0);
    vec4 light;
    float oceanBoundary = 1;
    if(length(hit_pos - PlanePosition)>oceanBoundary){
        light = vec4(0.0, 0.0, 0.0, 1.0);
    }else{
        vec4 height_map = texture(oceanHeight, vec2(hit_pos.x/oceanBoundary+0.5, hit_pos.z/oceanBoundary+0.5));
        vec4 normal_map = texture(oceanNormal, vec2(hit_pos.x/oceanBoundary+0.5, hit_pos.z/oceanBoundary+0.5));
        //move the light
        hit_pos.y+=height_map.y;
        //https://habibs.wordpress.com/water-mathematics/
        vec3 normal_map3 = normalize(vec3(normal_map.x, normal_map.y, normal_map.z));
        float ns = dot(Normal, normal_map3);
        float n1n2 = 1.0 * 0.7; //for water?
        vec3 normal_air = -normal_map3*((sqrt((1-n1n2*n1n2)*(1-ns)*(1-ns)))+n1n2*(ns))-Normal*n1n2;
        vec3 hit_pos2 = raytrace(lightPos, lightDir, hit_pos, normal_air);
        float lightStrength = length(lightPos - hit_pos2)/100;
        light = lightStrength*vec4(1.0, 1.0, 1.0, 1.0);
        //light = normal_map;
    }
    color = light;//*lighting;//*vec4(hit_pos, 1);
    //vec4 colorResult = vec4((ambient + diffuse + specular), 1.0);
    //color=colorResult;
}

#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 PlaneNormal; //position and normal of the caustics plane
in vec3 PlanePosition;

out vec4 color;

uniform sampler2D causticTexture;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

vec3 ray_trace(vec3 plane_center, vec3 plane_normal, vec3 ray_origin, vec3 ray_direction){
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

    vec3 hit_pos = ray_trace(PlanePosition, PlaneNormal, FragPos, Normal);

	vec4 colorResult = vec4((ambient + diffuse + specular), 1.0)*vec4(texture(causticTexture, TexCoords));
    if(length(hit_pos - PlanePosition)>1.0){
        color = vec4(0, 0, 1, 1.0);
    }else{
        color = vec4(0, 1, 0, 1.0);
    }
    //color = colorResult;//*vec4(hit_pos, 1);
}

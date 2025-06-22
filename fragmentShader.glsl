#version 330 core
in vec3 fragPos;
in vec3 normal;
in vec3 ourCol;
in vec2 texCoord;

out vec4 FragColor;
	
struct Material {
	float ambient;
	float diffuse;
	float specular;
	float shininess;
};
	
struct Light {
	vec3 pos;
	vec3 color;
	float constant;
	float linear;
	float quadratic;
};
	
uniform Material material;
uniform Light light;
uniform vec3 skyColor;
uniform sampler2D texture1;
uniform vec3 objectColor;
uniform vec3 viewPos;
	
void main() {
	vec3 texCol = vec3(texture(texture1, texCoord).x);
	// vec3 col = texture(texture1, texCoord);
	vec3 col;
	if(ourCol != vec3(0.0f)) col = ourCol;
	else col = objectColor;
	
	
	vec3 norm = normalize(normal);
	
	float avgObjectColor = (texCol.x + texCol.y + texCol.z) / 3.0f;
	
	vec3 lightDir = normalize(light.pos - fragPos);
	// vec3 lightDir = vec3(0.5f, 3.0f, 0.4f);
	float lightDist = length(light.pos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear*lightDist + light.quadratic*(lightDist*lightDist));
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	
	vec3 ambient = (light.color * attenuation) * material.ambient * col;
	
	float diff = max((dot(norm, lightDir)+0.4f), 0.0f);
	vec3 diffuse = diff * material.diffuse * light.color * attenuation * col;
	
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = material.specular * spec * light.color * attenuation * (vec3(avgObjectColor*3.0f) + texCol/2.0f);
	
	vec3 sky = skyColor * max((dot(norm/2.0f, vec3(0.0f, 1.0f, 0.0f))+0.4f), 0.0f);
	
	FragColor = vec4(col * (ambient + diffuse + specular) + sky, 1.0f);
	// FragColor = vec4(ourCol, 1.0f);
}

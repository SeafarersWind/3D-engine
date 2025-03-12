#version 330 core
in float size;
	
out vec4 FragColor;
	
uniform vec2 screenspaceCenter;
uniform float screenspaceRadius;
uniform vec3 lightColor;

void main() {
	float centerDistance = 1.0f - (distance(gl_FragCoord.xy, screenspaceCenter) / (screenspaceRadius*size));
	if(centerDistance <= 0.0f) discard;
	centerDistance = smoothstep(0.0f, 1.0f, centerDistance);
	FragColor = vec4(lightColor, centerDistance);
}

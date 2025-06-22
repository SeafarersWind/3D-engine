#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aCol;
layout (location = 4) in ivec4 boneIds;
layout (location = 5) in vec4 weights;

out vec3 fragPos;
out vec3 normal;
out vec3 ourCol;
out vec2 texCoord;
out vec4 debug_totalPosition;
out mat4 debug_bone;

const int MAX_BONES = 16;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bones[MAX_BONES];

void main() {
	vec4 totalPosition = vec4(0.0f);
	vec3 totalNormal = vec3(0.0f);
	
	bool hasBones = false;
	for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
		if(boneIds[i] == -1) continue;
		hasBones = true;
		if(boneIds[i] >= MAX_BONES) {
			totalPosition = vec4(aPos, 1.0f);
			totalNormal = aNormal;
			break;
		}
		vec4 localPosition = bones[boneIds[i]] * vec4(aPos, 1.0f);
		totalPosition += localPosition * weights[i];
		vec3 localNormal = mat3(bones[boneIds[i]]) * aNormal;
		totalNormal += localNormal * weights[i];
	}
	
	if(!hasBones) {
		totalPosition = vec4(aPos, 1.0f);
		totalNormal = aNormal;
	}
	
	// ourCol = vec3(bones[0][1][0], bones[0][1][1], bones[0][1][2]);
	
	debug_totalPosition = totalPosition;
	gl_Position = projection * view * model * totalPosition;
	normal = mat3(transpose(inverse(model))) * totalNormal;
	texCoord = aTexCoord;
}

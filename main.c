#if !defined(FLYAROUND) && !defined(WALKAROUND) && !defined(ICOSA)
#define ICOSA
#endif

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"




int windowWidth  = 720;
int windowHeight =  720;
const char* windowTitle  = "Quiet Field";

const double limitFPS = 1.0f / 20.0f;
const double tickSpeed = 1.0f / 20.0f;

bool inW, inS, inA, inD, inSpace, inLeftShift;
float lastX,lastY;
bool firstMouse = true;

const float cameraSpeed = 0.04f;	
const float cameraSensitivity = 0.002f;
vec3 cameraPos   = {0.0f, 1.0f, 0.0f};
vec3 cameraFront = {-1.0f, 0.0f, 0.0f};
vec3 cameraUp    = {0.0f, 1.0f, 0.0f};
vec3 cameraRight = {0.0f, 0.0f, -1.0f};
vec3 cameraDown  = {0.0f, -1.0f, 0.0f};
vec3 cameraTarget;
float cameraDistance = 18.0f;
float cameraYaw  = -0.5f*M_PI;
float cameraPitch = -0.0625*M_PI;
float cameraRoll;
versor cameraOrientation;

vec3 up   = {0.0f, 	1.0f, 0.0f};

float clearG = 0.05;
float clearB = 0.1;
float clearR = 0.0;
float clearMaxR = 0.05f;
float clearMaxG = 0.3f;
float clearMaxB = 0.4f;
float clearMinR = 0.0f;
float clearMinG = 0.0f;
float clearMinB = -0.05f;
int col = 0;
int fade = 0;

#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS 4
#define MAX_BONES 16



struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	unsigned int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Bone {
	char* name;
	mat4 offset;
};

struct Mesh {
	struct Vertex* vertices;
	unsigned int verticesCount;
	
	unsigned int* indices;
	unsigned int indexCount;
	
	unsigned int boneCount;
	char** boneNames;
	mat4* boneOffsets;
	
	unsigned int material;
	
	unsigned int VAO, VBO, EBO;
};

struct Model {
	struct Mesh* meshes;
	unsigned int meshCount;
	unsigned int* textures;
	unsigned int textureCount;
};

struct Object {
	struct Model model;
	vec3 pos;
	vec3 rot;
	vec3 scale;
};

struct Terrain {
	vec3* vertices;
	unsigned int vertexCount;
	unsigned int* indices;
	unsigned int faceCount;
};

// struct Mesh meshes[256];
//unsigned int* textures;
//unsigned int textureCount;



static void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	switch(action) {
	case GLFW_PRESS:
		switch(key) {
		case GLFW_KEY_W:
			inW = true;
			break;
		case GLFW_KEY_S:
			inS = true;
			break;
		case GLFW_KEY_A:
			inA = true;
			break;
		case GLFW_KEY_D:
			inD = true;
			break;
		case GLFW_KEY_SPACE:
			inSpace = true;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			inLeftShift = true;
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		}
		break;
	case GLFW_RELEASE:
		switch(key) {
		case GLFW_KEY_W:
			inW = false;
			break;
		case GLFW_KEY_S:
			inS = false;
			break;
		case GLFW_KEY_A:
			inA = false;
			break;
		case GLFW_KEY_D:
			inD = false;
			break;
		case GLFW_KEY_SPACE:
			inSpace = false;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			inLeftShift = false;
			break;
		}
	}
	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
	if(firstMouse) {
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	float xOffset = lastX - xPos;
	float yOffset = lastY - yPos;
	lastX = xPos;
	lastY = yPos;
	
	
	#ifdef WALKAROUND
	cameraYaw   -= xOffset * cameraSensitivity;	// walkaround camera
	cameraPitch += yOffset * cameraSensitivity;
	
	if     (cameraYaw >=  M_PI) cameraYaw -= 2.0f*M_PI;
	else if(cameraYaw <= -M_PI) cameraYaw += 2.0f*M_PI;
	if     (cameraPitch >=  M_PI) cameraPitch -= 2.0f*M_PI;
	else if(cameraPitch <= -M_PI) cameraPitch += 2.0f*M_PI;
	if     (cameraRoll >=  M_PI) cameraPitch -= 2.0f*M_PI;
	else if(cameraRoll <= -M_PI) cameraPitch += 2.0f*M_PI;
	
	cameraDirection[0] = cos(cameraYaw) * cos(cameraPitch);
	cameraDirection[1] = sin(cameraPitch);
	cameraDirection[2] = sin(cameraYaw) * cos(cameraPitch);
	glm_normalize_to(cameraDirection, cameraFront);
	if(cameraPitch < 0.5*M_PI && cameraPitch > -0.5*M_PI) {
		glm_vec3_crossn(cameraFront, cameraUp, cameraRight);
	} else {
		glm_vec3_crossn(cameraFront, cameraDown, cameraRight);
	}
	
	
	#elif defined(FLYAROUND)
	float controlYaw   = xOffset * cameraSensitivity;		// flyaround camera
	float controlPitch = yOffset * cameraSensitivity;
	glm_vec3_rotate(cameraFront, controlYaw, cameraUp);
	glm_vec3_rotate(cameraRight, controlYaw, cameraUp);
	glm_vec3_rotate(cameraFront, controlPitch, cameraRight);
	glm_vec3_rotate(cameraUp, controlPitch, cameraRight);
	
	
	#elif defined(ICOSA)
	cameraYaw   -= xOffset * cameraSensitivity;				// orbit icosa
	cameraPitch += yOffset * cameraSensitivity;
	
	if(cameraPitch >  0.5*M_PI) { cameraPitch =  0.5*M_PI; }
	if(cameraPitch < -0.5*M_PI) { cameraPitch = -0.5*M_PI; }
	if     (cameraYaw >=  M_PI) cameraYaw -= 2.0f*M_PI;
	else if(cameraYaw <= -M_PI) cameraYaw += 2.0f*M_PI;
	if     (cameraRoll >=  M_PI) cameraPitch -= 2.0f*M_PI;
	else if(cameraRoll <= -M_PI) cameraPitch += 2.0f*M_PI;
	
	vec3 cameraDirection = {
		cos(cameraYaw) * cos(cameraPitch),
		sin(cameraPitch),
		sin(cameraYaw) * cos(cameraPitch)
	};
	glm_normalize_to(cameraDirection, cameraFront);
	
	cameraRight[0] = -sin(cameraYaw);
	cameraRight[1] = 0.0f;
	cameraRight[2] = cos(cameraYaw);
	glm_normalize(cameraRight);
	
	cameraUp[0] = -cos(cameraYaw) * sin(cameraPitch);
	cameraUp[1] =  cos(cameraPitch);
	cameraUp[2] = -sin(cameraYaw) * sin(cameraPitch);
	glm_normalize(cameraUp);
	
	glm_quat_for(cameraDirection, cameraUp, cameraOrientation);
	#endif
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

GLFWwindow* initWindow() {
	glfwSetErrorCallback(error_callback);
	if(!glfwInit()) {
		printf("Failed to initialize GLFW.");
		return NULL;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	windowWidth  = mode->width;
	windowHeight = mode->height;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, monitor, NULL);
	if (!window) {
		printf("Failed to create GLFW window.\n");
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	
	if(!gladLoadGL(glfwGetProcAddress)) {
		printf("Failed to initialize GLAD.\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return NULL;
	}
	
	glViewport(0, 0, windowWidth, windowHeight);
	
	return window;
}



GLuint createShader(char* shaderPath, GLenum shaderType) {
	FILE* shaderFile = fopen(shaderPath, "rb");
	fseek(shaderFile, 0, SEEK_END);
	long shaderFileLength = ftell(shaderFile);
	GLchar* shaderContent = malloc(shaderFileLength+1);
	rewind(shaderFile);
	fread(shaderContent, 1, shaderFileLength, shaderFile);
	fclose(shaderFile);
	
	int success;
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, (const GLchar**)&shaderContent, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("%s:\n%s", shaderPath, infoLog);
	}
	
	return shader;
}

GLuint createShaderProgram(char* programName, GLuint vertexShader, GLuint fragmentShader) {
	int success;
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s:\n%s", programName, infoLog);
	}
	
	return shaderProgram;
}



struct Mesh processMesh(struct aiMesh* aiMesh, const struct aiScene* scene) {
printf("\n");
	unsigned int verticesCount = aiMesh->mNumVertices;
	struct Vertex* vertices = malloc(aiMesh->mNumVertices*sizeof(struct Vertex));
	
	unsigned int indexCount = 0;
	for(unsigned int i = 0; i < aiMesh->mNumFaces; i++) { indexCount += aiMesh->mFaces[i].mNumIndices; }
	unsigned int* indices = malloc(indexCount*sizeof(unsigned int));
	
	// vertices
	for(unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
		struct Vertex vertex;
		vertex.position[0] = aiMesh->mVertices[i].x;
		vertex.position[1] = aiMesh->mVertices[i].y;
		vertex.position[2] = aiMesh->mVertices[i].z;
		vertex.normal[0] = aiMesh->mNormals[i].x;
		vertex.normal[1] = aiMesh->mNormals[i].y;
		vertex.normal[2] = aiMesh->mNormals[i].z;
		vertex.texCoords[0] = aiMesh->mTextureCoords[0][i].x;
		vertex.texCoords[1] = aiMesh->mTextureCoords[0][i].y;
		for (int ii = 0; ii < MAX_BONE_WEIGHTS; ii++) {
			vertex.m_BoneIDs[ii] = -1;
			vertex.m_Weights[ii] = 0.0f;
		}
		vertices[i] = vertex;
	}
	
	// indices
	indexCount = 0;
	for(unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
		for(unsigned int j = 0; j < aiMesh->mFaces[i].mNumIndices; j++) {
			indices[indexCount] = aiMesh->mFaces[i].mIndices[j];
			indexCount++;
		}
	}
	
	// bones
	char* boneIndicesNames[aiMesh->mNumBones];
	unsigned int boneIndices[aiMesh->mNumBones];
	unsigned int boneCount = 0;
	for(unsigned int i = 0; i < aiMesh->mNumBones; i++) {
		char* boneName = aiMesh->mBones[i]->mName.data;
		unsigned int ii;
		for(ii = 0; ii < boneCount; ii++) {
			if(strcmp(boneName, boneIndicesNames[ii]) == 0) {
				break;
			}
		} if(ii >= boneCount) {
			boneIndicesNames[boneCount] = boneName;
			boneIndices[boneCount] = boneCount;
printf("%d %s\n", boneCount, boneName);
			boneCount++;
		}
	}
	
	char** boneNames = malloc(boneCount);
	mat4* boneOffsets = malloc(sizeof(mat4)*boneCount);
	
	for(unsigned int i = 0; i < boneCount; i++) {
		boneNames[i] = malloc(aiMesh->mBones[boneIndices[i]]->mName.length+1);
		strcpy(boneNames[i], aiMesh->mBones[boneIndices[i]]->mName.data);
		
		boneOffsets[i][0][0] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.a1;
		boneOffsets[i][0][1] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.a2;
		boneOffsets[i][0][2] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.a3;
		boneOffsets[i][0][3] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.a4;
		boneOffsets[i][1][0] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.b1;
		boneOffsets[i][1][1] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.b2;
		boneOffsets[i][1][2] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.b3;
		boneOffsets[i][1][3] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.b4;
		boneOffsets[i][2][0] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.c1;
		boneOffsets[i][2][1] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.c2;
		boneOffsets[i][2][2] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.c3;
		boneOffsets[i][2][3] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.c4;
		boneOffsets[i][3][0] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.d1;
		boneOffsets[i][3][1] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.d2;
		boneOffsets[i][3][2] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.d3;
		boneOffsets[i][3][3] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.d4;
printf("%d %s \n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", i, boneNames[i],
 boneOffsets[i][0][0], boneOffsets[i][0][1], boneOffsets[i][0][2], boneOffsets[i][0][3],
 boneOffsets[i][1][0], boneOffsets[i][1][1], boneOffsets[i][1][2], boneOffsets[i][1][3],
 boneOffsets[i][2][0], boneOffsets[i][2][1], boneOffsets[i][2][2], boneOffsets[i][2][3],
 boneOffsets[i][3][0], boneOffsets[i][3][1], boneOffsets[i][3][2], boneOffsets[i][3][3]);
		
		for(unsigned int ii = 0; ii < aiMesh->mBones[boneIndices[i]]->mNumWeights; ii++) {
			struct aiVertexWeight weight = aiMesh->mBones[boneIndices[i]]->mWeights[ii];
			for(unsigned int iii = 0; iii < MAX_BONE_INFLUENCE; iii++) {
				if(vertices[weight.mVertexId].m_BoneIDs[iii] == -1 && weight.mWeight != 0.0f) {
// printf("%f ", weight.mWeight);
					vertices[weight.mVertexId].m_BoneIDs[iii] = i;
					vertices[weight.mVertexId].m_Weights[iii] = weight.mWeight;
					break;
				}
			}
		}
// printf("\n");
	}
	
	
	
	struct Mesh finalMesh = {
		.vertices = vertices,
		.verticesCount = verticesCount,
		.indices = indices,
		.indexCount = indexCount,
		.material = aiMesh->mMaterialIndex,
		.boneCount = boneCount,
		.boneOffsets = boneOffsets,
		.boneNames = boneNames
	};
printf("\n");
for(unsigned int i = 0; i < verticesCount; i++) {
	for(unsigned int ii = 0; ii < MAX_BONE_INFLUENCE; ii++) {
		printf("%d %f ", vertices[i].m_BoneIDs[ii], vertices[i].m_Weights[ii]);
	}
	printf("\n");
}
printf("\n");
	
	glGenVertexArrays(1, &finalMesh.VAO);
	glGenBuffers(1, &finalMesh.VBO);
	glGenBuffers(1, &finalMesh.EBO);
	
	glBindVertexArray(finalMesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, finalMesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, finalMesh.verticesCount*sizeof(*finalMesh.vertices), &finalMesh.vertices[0].position[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalMesh.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, finalMesh.indexCount*sizeof(*finalMesh.indices), &finalMesh.indices[0], GL_STATIC_DRAW);
	
	// position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)0);
	// normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)offsetof(struct Vertex, normal));
	// texture attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)offsetof(struct Vertex, texCoords));
	// bone id attribute
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 4, GL_INT, sizeof(struct Vertex), (void*)offsetof(struct Vertex, m_BoneIDs));
	// bone weight attribute
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, m_Weights));
	
	glBindVertexArray(0);
	
	return finalMesh;
}

unsigned int meshCount;

void processNode(struct aiNode* node, struct Mesh* meshes, const struct aiScene* scene) {
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		struct aiMesh *aiMesh = scene->mMeshes[node->mMeshes[i]]; 
		meshes[meshCount] = processMesh(aiMesh, scene);
		meshCount++;
	}
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], meshes, scene);
	}
}

unsigned int countMeshes(struct aiNode* node) {
	unsigned int meshCount = node->mNumMeshes;
	for(unsigned int i = 0; i < node->mNumChildren; i++) meshCount += countMeshes(node->mChildren[i]);
	return meshCount;
}

struct Terrain createTerrain(char* modelPath) {
	struct Terrain terrain;
	
	const struct aiScene* scene = aiImportFile( modelPath,
	aiProcess_CalcTangentSpace       |
	aiProcess_Triangulate            |
	aiProcess_JoinIdenticalVertices  |
	aiProcess_SortByPType);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf(aiGetErrorString());
		return terrain;
	}
	
	terrain.vertexCount = 0;
	terrain.faceCount = 0;
	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		terrain.vertexCount += scene->mMeshes[i]->mNumVertices;
		terrain.faceCount += scene->mMeshes[i]->mNumFaces;
	}
	
	terrain.vertices = malloc(terrain.vertexCount*sizeof(vec3));
	terrain.indices  = malloc(terrain.faceCount*3*sizeof(unsigned int));
	
	unsigned int vertexIndex = 0;
	unsigned int indexIndex  = 0;
	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		for(unsigned int ii = 0; ii < scene->mMeshes[i]->mNumVertices; ii++) {
			terrain.vertices[vertexIndex][0] = scene->mMeshes[i]->mVertices[ii].x;
			terrain.vertices[vertexIndex][1] = scene->mMeshes[i]->mVertices[ii].y;
			terrain.vertices[vertexIndex][2] = scene->mMeshes[i]->mVertices[ii].z;
			vertexIndex++;
		}
		for(unsigned int ii = 0; ii < scene->mMeshes[i]->mNumFaces; ii++) {
			terrain.indices[indexIndex*3]   = scene->mMeshes[i]->mFaces[ii].mIndices[0];
			terrain.indices[indexIndex*3+1] = scene->mMeshes[i]->mFaces[ii].mIndices[1];
			terrain.indices[indexIndex*3+2] = scene->mMeshes[i]->mFaces[ii].mIndices[2];
			indexIndex++;
		}
	}
	
	aiReleaseImport(scene);
	
	return terrain;
}

struct Model loadModel(char* modelPath) {
	struct Model model;
	
	const struct aiScene* scene = aiImportFile( modelPath,
	 aiProcess_CalcTangentSpace       |
	 aiProcess_Triangulate            |
	 aiProcess_JoinIdenticalVertices  |
	 aiProcess_SortByPType);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf(aiGetErrorString());
		return model;
	}
	
	// meshes
	model.meshCount = countMeshes(scene->mRootNode);
	model.meshes = malloc(model.meshCount*sizeof(struct Mesh));
	meshCount = 0;
	processNode(scene->mRootNode, model.meshes, scene);
	
	// textures
	model.textures = malloc(scene->mNumMaterials*sizeof(unsigned int));
	glGenTextures(scene->mNumMaterials, model.textures);
	char* texPath = (char*)malloc(256*sizeof(char));
	unsigned int texPathLength = strrchr(modelPath, '/')+1 - modelPath;
	strncpy(texPath, modelPath, texPathLength);
	for(unsigned int i = 1; i < scene->mNumMaterials; i++) {	
		struct aiString texStr;
		aiGetMaterialTexture(scene->mMaterials[i], aiTextureType_DIFFUSE, 0, &texStr, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0);
		strcpy(&texPath[texPathLength], texStr.data);
		int texWidth, texHeight, nrChannels;
		glBindTexture(GL_TEXTURE_2D, model.textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		unsigned char *texData = stbi_load(texPath, &texWidth, &texHeight, &nrChannels, 0);
		if(texData) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else {
			printf("Failed to load texture \"%s\".\n", texPath);
		}
		stbi_image_free(texData);
	}
	free(texPath);
	
	aiReleaseImport(scene);
	
	return model;
}

struct Object createObject(struct Model model) {
	struct Object object;
	
	object.model = model;
	object.pos[0] = 0.0f;
	object.pos[1] = 0.0f;
	object.pos[2] = 0.0f;
	object.rot[0] = 0.0f;
	object.rot[1] = 0.0f;
	object.rot[2] = 0.0f;
	object.scale[0] = 1.0f;
	object.scale[1] = 1.0f;
	object.scale[2] = 1.0f;
	
	return object;
}



int main(void) {
	GLFWwindow* window = initWindow();
	if(!window) return -1;
	
	
	
	// shaders
	int vertexShader = createShader("vertexShader.glsl", GL_VERTEX_SHADER);
	int skelVertexShader = createShader("vertexShaderSkel.glsl", GL_VERTEX_SHADER);
	int billboardVertexShader = createShader("vertexShaderBillboard.glsl", GL_VERTEX_SHADER);
	int fragmentShader = createShader("fragmentShader.glsl", GL_FRAGMENT_SHADER);
	int lightFragmentShader = createShader("fragmentShaderLight.glsl", GL_FRAGMENT_SHADER);
	
	int shaderProgram = createShaderProgram("shaderProgram", vertexShader, fragmentShader);
	int skelShaderProgram = createShaderProgram("skelShaderProgram", skelVertexShader, fragmentShader);
	int lightShaderProgram = createShaderProgram("lightShaderProgram", billboardVertexShader, lightFragmentShader);
	
	glDeleteShader(vertexShader);
	glDeleteShader(skelVertexShader);
	glDeleteShader(billboardVertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(lightFragmentShader);
	
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.0f, 0.7f, 1.0f);
	
	
	
	// terrian
	struct Model terrain = loadModel("assets/Terrain/Field.obj");
	struct Terrain geometry = createTerrain("assets/Terrain/Field.obj");
	
	
	
	// objects
	struct Object icosa = createObject(loadModel("assets/Character/icosa2.dae"));
	icosa.pos[0] = 200.0f;
	icosa.pos[1] = 100.0f;
	icosa.pos[2] = 200.0f;
	
	const unsigned int objectCount = 1;
	struct Object objects[objectCount];
	objects[0] = icosa;
	
	
	
	// light
	glUseProgram(lightShaderProgram);
	float circleVertices[] = {
		 0.0f,     2.0f,
		-1.7321f, -1.0f,
		 1.7321f, -1.0f
	};
	
	int circleVBO, lightVAO;
	glGenBuffers(1, &circleVBO);
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	vec3 lightPos = {0.0f, 20.0f, 0.0f};
	vec3 lightColor = {1.0f, 1.0f, 1.0f};
	float lightConstant  = 1.0f;
	float lightLinear    = 0.035f;
	float lightQuadratic = 0.0000f;
	float lightIntensity = 1.0f;
	
	
	
	// camera
	vec3 cameraDirection = {
		cos(cameraYaw) * cos(cameraPitch),
		sin(cameraPitch),
		sin(cameraYaw) * cos(cameraPitch)
	};
	glm_normalize_to(cameraDirection, cameraFront);
	
	cameraRight[0] = -sin(cameraYaw);
	cameraRight[1] = 0.0f;
	cameraRight[2] = cos(cameraYaw);
	glm_normalize(cameraRight);
	
	cameraUp[0] = -cos(cameraYaw) * sin(cameraPitch);
	cameraUp[1] =  cos(cameraPitch);
	cameraUp[2] = -sin(cameraYaw) * sin(cameraPitch);
	glm_normalize(cameraUp);
	
	glm_quat_for(cameraDirection, cameraUp, cameraOrientation);
	
	
	
	// time
	double nowTime;
	double lastTime = glfwGetTime();
	double deltaTime = 0.0;
	double tickTock = 0.0;
	double secondTimer = 0.0;
	unsigned int frames = 0;
	unsigned int ticks = 0;
	unsigned int timer = 0;
	
	//forever
	while(!glfwWindowShouldClose(window)) {
		
		//tick
		while(tickTock >= 1.0) {
			switch(col) {
			case 0:
				clearB += 0.0002f;
				if(clearB >= clearMaxB/2.0) col++;
				break;
			case 1:
				clearG += 0.0002f;
				if(clearG >= clearMaxG) col++;
				break;
			case 2:
				clearR -= 0.0002f;
				clearB -= 0.0002f;
				if(clearR <= clearMinR) col++;
				break;
			case 3:
				clearB += 0.0003f;
				if(clearB >= clearMaxB) col++;
				break;
			case 4:
				clearG -= 0.0005f;
				if(clearG <= clearMinG+0.2f) col++;
				break;
			case 5:
				clearB -= 0.0002f;
				clearG -= 0.0001f;
				if(clearB <= clearMinB) col++;
				break;
			case 6:
				clearR += 0.0002f;
				clearB += 0.0001f;
				if(clearR >= clearMaxR) col = 0;
				break;
			}
			
			vec2 icosaFront = { cos(cameraYaw), sin(cameraYaw) };
			vec2 icosaRight = { cos(cameraYaw+M_PI/2.0f), sin(cameraYaw+M_PI/2.0f) };
			vec2 icosaForwards = { 0.0f, 0.0f };
			if(inW && !inS) {											// follow icosa
				icosaForwards[0] += icosaFront[0];
				icosaForwards[1] += icosaFront[1];
			} else if(inS && !inW) {
				icosaForwards[0] -= icosaFront[0];
				icosaForwards[1] -= icosaFront[1];
			} if(inA && !inD) {
				icosaForwards[0] -= icosaRight[0];
				icosaForwards[1] -= icosaRight[1];
			} else if(inD && !inA) {
				icosaForwards[0] += icosaRight[0];
				icosaForwards[1] += icosaRight[1];
			} if(inSpace && !inLeftShift) {
				objects[0].pos[1] += 0.15f;
			} else if(inLeftShift && !inSpace) {
				objects[0].pos[1] -= 0.15f;
			}
			glm_normalize(icosaForwards);
			objects[0].pos[0] += icosaForwards[0] *0.5f;
			objects[0].pos[2] += icosaForwards[1] *0.5f;
			
			if(icosaForwards[0] != 0.0f || icosaForwards[1] != 0.0f) {
				if(objects[0].rot[1] >  M_PI) objects[0].rot[1] -= M_PI*2.0;
				if(objects[0].rot[1] < -M_PI) objects[0].rot[1] += M_PI*2.0;
				float angleIncrement = (icosaForwards[0] > 0.0f ?
					-atan(icosaForwards[1] / icosaForwards[0]) :
					-atan(icosaForwards[1] / icosaForwards[0])+M_PI)
					- objects[0].rot[1];
				if(angleIncrement >  M_PI) angleIncrement -= M_PI*2.0;
				if(angleIncrement < -M_PI) angleIncrement += M_PI*2.0;
				objects[0].rot[1] += angleIncrement*0.6;
			}
			
			for(unsigned int i = 0; i < geometry.faceCount; i++) {
				vec3 vertex1 = {
					geometry.vertices[geometry.indices[i*3+0]][0],
					geometry.vertices[geometry.indices[i*3+0]][1],
					geometry.vertices[geometry.indices[i*3+0]][2]
				};
				vec3 vertex2 = {
					geometry.vertices[geometry.indices[i*3+1]][0],
					geometry.vertices[geometry.indices[i*3+1]][1],
					geometry.vertices[geometry.indices[i*3+1]][2]
				};
				vec3 vertex3 = {
					geometry.vertices[geometry.indices[i*3+2]][0],
					geometry.vertices[geometry.indices[i*3+2]][1],
					geometry.vertices[geometry.indices[i*3+2]][2]
				};
				float x1 = vertex1[0];
				float y1 = vertex1[2];
				float x2 = vertex2[0];
				float y2 = vertex2[2];
				float x3 = vertex3[0];
				float y3 = vertex3[2];
				float px = objects[0].pos[0];
				float py = objects[0].pos[2];
				float faceArea = fabs( (x2-x1)*(y3-y1) - (x3-x1)*(y2-y1) );
				float area1 =    fabs( (x2-px)*(y3-py) - (x3-px)*(y2-py) );
				float area2 =    fabs( (x3-px)*(y1-py) - (x1-px)*(y3-py) );
				float area3 =    fabs( (x1-px)*(y2-py) - (x2-px)*(y1-py) );
				if (area1 + area2 + area3 >= faceArea - 0.0001 && area1 + area2 + area3 <= faceArea + 0.0001) {
					objects[0].pos[1] = (vertex1[1]*area1 + vertex2[1]*area2 + vertex3[1]*area3) / (faceArea) /*+ 3.0f*/;
				}
			}
			
			
			// light
			lightPos[0] = sin(timer/12.0);
			lightPos[2] = cos(timer/12.0);
			lightPos[1] = sin(timer/26.0)*0.25+12.0;
			
			lightIntensity = cos(timer/30.0)*0.125+0.8;
			lightColor[0] = lightIntensity;
			lightColor[1] = lightIntensity;
			lightColor[2] = lightIntensity;
			
			
			
			timer++;
			
			ticks++;
			tickTock--;
		}
		
		//render
		if(deltaTime >= 1.0) {
			glClearColor(clearR, clearG, clearB, 1.0f);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_CULL_FACE);
			
			
			
			
			//view matrix
			mat4 viewMat;
			
			#if defined(WALKAROUND)
			vec3 cameraTarget = {cameraPos[0]+cameraFront[0], cameraPos[1]+cameraFront[1], cameraPos[2]+cameraFront[2]};
			if(cameraPitch < 0.5*M_PI && cameraPitch > -0.5*M_PI) {
				glm_lookat(cameraPos, cameraTarget, cameraUp, viewMat);
			} else {
				glm_lookat(cameraPos, cameraTarget, cameraDown, viewMat);
			}
			
			#elif defined(ICOSA) || defined(FLYAROUND)
			cameraTarget[0] = objects[0].pos[0];
			cameraTarget[1] = objects[0].pos[1]+2.0f;
			cameraTarget[2] = objects[0].pos[2];
			cameraPos[0] = -cos(cameraYaw) * cos(cameraPitch) * cameraDistance + cameraTarget[0];
			cameraPos[1] = -sin(cameraPitch) * cameraDistance + cameraTarget[1];
			cameraPos[2] = -sin(cameraYaw) * cos(cameraPitch) * cameraDistance + cameraTarget[2];
			glm_quat_look(cameraPos, cameraOrientation, viewMat);
			#endif
			
			//projection matrix
			mat4 projMat;
			glm_perspective((0.25f*M_PI), ((float)windowWidth / (float)windowHeight), 0.1f, 600.0f, projMat);
			
			//model matrix
			mat4 modelMat;
			
			
			
			// terrain
			glUseProgram(shaderProgram);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, (GLfloat*)viewMat);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projMat);
			glActiveTexture(GL_TEXTURE0);
			glUniform3fv(glGetUniformLocation(shaderProgram, "light.pos"), 1, lightPos);
			glUniform3fv(glGetUniformLocation(shaderProgram, "light.color"), 1, lightColor);
			glUniform1f(glGetUniformLocation(shaderProgram, "light.constant"), lightConstant);
			glUniform1f(glGetUniformLocation(shaderProgram, "light.linear"), lightLinear);
			glUniform1f(glGetUniformLocation(shaderProgram, "light.quadratic"), lightQuadratic);
			glUniform3f(glGetUniformLocation(shaderProgram, "skyColor"), clearR, clearG, clearB);
			glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, cameraPos);
			glUniform1f(glGetUniformLocation(shaderProgram, "material.ambient"), 0.2f);
			glUniform1f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.8f);
			glUniform1f(glGetUniformLocation(shaderProgram, "material.specular"), 0.0f);
			glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 1.0f);
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.2f, 0.7f, 0.4f);
			glm_mat4_identity(modelMat);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (GLfloat*)modelMat);
			
			for(int i = 0; i < terrain.meshCount; i++) {
				glBindTexture(GL_TEXTURE_2D, terrain.textures[terrain.meshes[i].material]);
				glBindVertexArray(terrain.meshes[i].VAO);
				glDrawElements(GL_TRIANGLES, terrain.meshes[i].indexCount, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
			
			
			
			// objects
			glUseProgram(skelShaderProgram);
			glUniformMatrix4fv(glGetUniformLocation(skelShaderProgram, "view"), 1, GL_FALSE, (GLfloat*)viewMat);
			glUniformMatrix4fv(glGetUniformLocation(skelShaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projMat);
			glActiveTexture(GL_TEXTURE0);
			glUniform3fv(glGetUniformLocation(skelShaderProgram, "light.pos"), 1, lightPos);
			glUniform3fv(glGetUniformLocation(skelShaderProgram, "light.color"), 1, lightColor);
			glUniform1f(glGetUniformLocation(skelShaderProgram, "light.constant"), lightConstant);
			glUniform1f(glGetUniformLocation(skelShaderProgram, "light.linear"), lightLinear);
			glUniform1f(glGetUniformLocation(skelShaderProgram, "light.quadratic"), lightQuadratic);
			glUniform3f(glGetUniformLocation(skelShaderProgram, "skyColor"), clearR, clearG, clearB);
			glUniform3fv(glGetUniformLocation(skelShaderProgram, "viewPos"), 1, cameraPos);
			glUniform1f(glGetUniformLocation(skelShaderProgram, "material.ambient"), 0.2f);
			glUniform1f(glGetUniformLocation(skelShaderProgram, "material.diffuse"), 0.8f);
			glUniform1f(glGetUniformLocation(skelShaderProgram, "material.specular"), 0.0f);
			glUniform1f(glGetUniformLocation(skelShaderProgram, "material.shininess"), 4.0f);
			glUniform3f(glGetUniformLocation(skelShaderProgram, "objectColor"), /*0.8f, 0.5f, 0.4f*/ 0.0f, 0.0f, 0.0f);
			vec3 modelRotX = { 1.0f, 0.0f, 0.0f };
			vec3 modelRotY = { 0.0f, 1.0f, 0.0f };
			vec3 modelRotZ = { 0.0f, 0.0f, 1.0f };
			for(int i = 0; i < objectCount; i++) {
				glm_translate_make(modelMat, objects[i].pos);
				glm_rotate(modelMat, objects[i].rot[0], modelRotX);
				glm_rotate(modelMat, objects[i].rot[1], modelRotY);
				glm_rotate(modelMat, objects[i].rot[2], modelRotZ);
				glm_scale(modelMat, objects[i].scale);
				glUniformMatrix4fv(glGetUniformLocation(skelShaderProgram, "model"), 1, GL_FALSE, (GLfloat*)modelMat);
				
				for(int ii = 0; ii < objects[i].model.meshCount; ii++) {
					glBindTexture(GL_TEXTURE_2D, objects[i].model.textures[objects[i].model.meshes[ii].material]);
					glBindVertexArray(objects[i].model.meshes[ii].VAO);
					glUniformMatrix4fv(glGetUniformLocation(skelShaderProgram, "bones"),
					 objects[i].model.meshes[ii].boneCount, GL_FALSE,
					 (float*)objects[i].model.meshes[ii].boneOffsets );
					glDrawElements(GL_TRIANGLES, objects[i].model.meshes[ii].indexCount, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
				}
			}
			
			
			
			// light
			glUseProgram(lightShaderProgram);
			glUniform3f(glGetUniformLocation(lightShaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
			glUniform1f(glGetUniformLocation(lightShaderProgram, "aSize"), lightIntensity*1.25);
		
			mat4 lightModelMat;
			mat4 lightViewModelMat;
			vec4 lightViewModel = {0.0f, 0.0f, 0.0f, 1.0f};
			glm_translate_make(lightModelMat, lightPos);
			glm_mat4_mul(viewMat, lightModelMat, lightViewModelMat);
			glm_mat4_mulv(lightViewModelMat, lightViewModel, lightViewModel);
			glUniform4fv(glGetUniformLocation(lightShaderProgram, "viewModel"), 1, lightViewModel);
			glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projMat);
			vec4 lightProjViewModel;
			glm_mat4_mulv(projMat, lightViewModel, lightProjViewModel);
			lightProjViewModel[0] /= lightProjViewModel[3];
			lightProjViewModel[1] /= lightProjViewModel[3];
			lightProjViewModel[0] += 1.0f;
			lightProjViewModel[1] += 1.0f;
			lightProjViewModel[0] *= windowWidth/2;
			lightProjViewModel[1] *= windowHeight/2;
			glUniform2f(glGetUniformLocation(lightShaderProgram, "screenspaceCenter"), lightProjViewModel[0], lightProjViewModel[1]);
			vec4 lightProjViewModel1 = {1.0f, 0.0f, 0.0f, 0.0f};
			glm_vec4_add(lightViewModel, lightProjViewModel1, lightProjViewModel1);
			glm_mat4_mulv(projMat, lightProjViewModel1, lightProjViewModel1);
			lightProjViewModel1[0] /= lightProjViewModel1[3];
			lightProjViewModel1[0] += 1.0f;
			lightProjViewModel1[0] *= windowWidth/2;
			glUniform1f(glGetUniformLocation(lightShaderProgram, "screenspaceRadius"), lightProjViewModel1[0] - lightProjViewModel[0]);
			
			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			
			
			
			glBindVertexArray(0);
			glUseProgram(0);
			glfwSwapBuffers(window);
			frames++;
			deltaTime--;
		}
		
		glfwPollEvents();
		
		nowTime = glfwGetTime();
		deltaTime += (nowTime - lastTime) / limitFPS;
		tickTock  += (nowTime - lastTime) / tickSpeed;
		lastTime = nowTime;
		
		if(nowTime - secondTimer > 0.125f) {
			secondTimer+= 0.125f;
			//printf("%d ticks, %d frames\n", ticks, frames);
			ticks = 0;
			frames = 0;
		}
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
	
}

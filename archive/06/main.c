#define WALKAROUND

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
	

const char* vertexShaderSource =
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aNormal;\n"
	"layout (location = 2) in vec2 aTexCoord;\n"
	"layout (location = 3) in vec3 aCol;\n"

	"out vec3 fragPos;\n"
	"out vec3 normal;\n"
	"out vec3 ourCol;\n"
	"out vec2 texCoord;\n"

	"uniform mat4 model;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"

	"void main() {\n"
	"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
	"	fragPos = vec3(model * vec4(aPos, 1.0f));\n"
	"	normal = mat3(transpose(inverse(model))) * aNormal;\n"
	"	ourCol = aCol;\n"
	"	texCoord = aTexCoord;\n"
	"}\0";

const char* fragmentShaderSource =
	"#version 330 core\n"
	"in vec3 fragPos;\n"
	"in vec3 normal;\n"
	"in vec3 ourCol;\n"
	"in vec2 texCoord;\n"

	"out vec4 FragColor;\n"
	
	"struct Material {\n"
	"	float ambient;\n"
	"	float diffuse;\n"
	"	float specular;\n"
	"	float shininess;\n"
	"};\n"
	
	"struct Light {\n"
	"	vec3 pos;\n"
	"	vec3 color;\n"
	"	float constant;\n"
	"	float linear;\n"
	"	float quadratic;\n"
	"};\n"
	
	"uniform Material material;\n"
	"uniform Light light;\n"
	"uniform sampler2D texture1;\n"
	"uniform vec3 objectColor;\n"
	"uniform vec3 viewPos;\n"
	"uniform vec3 lightAmbient;\n"
	
	"void main() {\n"
	"	vec3 texCol = vec3(texture(texture1, texCoord).x);\n"
	"	vec3 norm = normalize(normal);\n"
	"	float avgObjectColor = (texCol.x + texCol.y + texCol.z) / 3.0f;\n"
	"	vec3 lightDir = normalize(light.pos - fragPos);\n"
	"	float lightDist = length(light.pos - fragPos);\n"
	"	float attenuation = 1.0 / (light.constant + light.linear*lightDist + light.quadratic*(lightDist*lightDist));"
	"	vec3 viewDir = normalize(viewPos - fragPos);\n"
	"	vec3 reflectDir = reflect(-lightDir, norm);\n"
	
	"	vec3 ambient = (light.color * attenuation + lightAmbient) * material.ambient * texCol;\n"
	
	"	float diff = max((dot(norm, lightDir)+0.4f), 0.0f);\n"
	"	vec3 diffuse = diff * material.diffuse * light.color * attenuation * texCol;\n"
	
	"	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);\n"
	"	vec3 specular = material.specular * spec * light.color * attenuation * (vec3(avgObjectColor*3.0f) + texCol/2.0f);\n"
	
	"	FragColor = vec4(vec3(texture(texture1, texCoord) * vec4((ambient + diffuse + specular), 1.0f)), 1.0f);\n"
	"}\0";

	  int   windowWidth  = 720;
	  int   windowHeight =  720;
const char* windowTitle  = "Small Chest";
const double limitFPS = 1.0f / 20.0f;
const double tickSpeed = 1.0f / 60.0f;

bool inW, inS, inA, inD, inSpace, inLeftShift;
float lastX, lastY;
bool firstMouse = true;

const float cameraSpeed = 0.04f;	
const float cameraSensitivity = 0.001f;
vec3 cameraPos   = {0.0f, -1.1f, 11.0f};
vec3 cameraFront = {0.0f, 0.0f, -1.0f};
vec3 cameraUp    = {0.0f, 1.0f, 0.0f};
vec3 cameraRight = {1.0f, 0.0f, 0.0f};
vec3 cameraDown  = {0.0f, -1.0f, 0.0f};
vec3 cameraDirection = {0.0f, 0.0f, 0.0f};
float cameraYaw  = -0.5f*M_PI;
float cameraPitch;
float cameraRoll;

float clearG = 0.6;
float clearB = 0.4;
float clearR = 1.0;
int col = 2;
int fade = 0;



struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 texCoords;
};

struct Mesh {
	struct Vertex* vertices;
	unsigned int verticesCount;
	
	unsigned int* indices;
	unsigned int indicesCount;
	
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

// struct Mesh meshes[256];
unsigned char meshCount = 0;
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
	#else
	float controlYaw   = xOffset * cameraSensitivity;		// flyaround camera
	float controlPitch = yOffset * cameraSensitivity;
	glm_vec3_rotate(cameraFront, controlYaw, cameraUp);
	glm_vec3_rotate(cameraRight, controlYaw, cameraUp);
	glm_vec3_rotate(cameraFront, controlPitch, cameraRight);
	glm_vec3_rotate(cameraUp, controlPitch, cameraRight);
	#endif
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}



struct Mesh processMesh(struct aiMesh* aiMesh, const struct aiScene* scene) {
	unsigned int verticesCount = aiMesh->mNumVertices;
	struct Vertex* vertices = malloc(aiMesh->mNumVertices*sizeof(struct Vertex));
	
	unsigned int indicesCount = 0;
	for(unsigned int i = 0; i < aiMesh->mNumFaces; i++) { indicesCount += aiMesh->mFaces[i].mNumIndices; }
	unsigned int* indices = malloc(indicesCount*sizeof(unsigned int));

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
		vertices[i] = vertex;
		//printf("%f, %f, %f      %f, %f, %f      %f, %f\n", vertex.position[0], vertex.position[1], vertex.position[2], vertex.normal[0], vertex.normal[1], vertex.normal[2], vertex.texCoords[0], vertex.texCoords[1]);
	}
	//printf("\n\n\n\n\n\n\n\n");
	
	indicesCount = 0;
	for(unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
		for(unsigned int j = 0; j < aiMesh->mFaces[i].mNumIndices; j++) {
			indices[indicesCount] = aiMesh->mFaces[i].mIndices[j];
			indicesCount++;
		}
	}
	
	struct Mesh finalMesh = {
		.vertices = vertices,
		.verticesCount = verticesCount,
		.indices = indices,
		.indicesCount = indicesCount,
		.material = aiMesh->mMaterialIndex
	};
	
	glGenVertexArrays(1, &finalMesh.VAO);
	glGenBuffers(1, &finalMesh.VBO);
	glGenBuffers(1, &finalMesh.EBO);
	
	glBindVertexArray(finalMesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, finalMesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, finalMesh.verticesCount*sizeof(*finalMesh.vertices), &finalMesh.vertices[0].position[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalMesh.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, finalMesh.indicesCount*sizeof(*finalMesh.indices), &finalMesh.indices[0], GL_STATIC_DRAW);
	
	// position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)0);
	// normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)offsetof(struct Vertex, normal));
	// texture attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)offsetof(struct Vertex, texCoords));
	
	glBindVertexArray(0);
	
	return finalMesh;
}

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
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		meshCount += countMeshes(node->mChildren[i]);
	}
	return meshCount;
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
	
	model.meshCount = countMeshes(scene->mRootNode);
	model.meshes = malloc(model.meshCount*sizeof(struct Mesh));
	meshCount = 0;
	processNode(scene->mRootNode, model.meshes, scene);
	
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
	glfwSetErrorCallback(error_callback);
	if(!glfwInit()) {
		printf("Failed to initialize GLFW.");
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, 0, 0);
	if (!window) {
		printf("Failed to create GLFW window.\n");
		glfwTerminate();
		return -1;
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
		return -1;
	}
	
	glViewport(0, 0, windowWidth, windowHeight);
	
	
	int success;
	char infoLog[512];
	
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf(infoLog);
	}
	
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf(infoLog);
	}
	
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf(infoLog);
	}
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.0f, 0.7f, 1.0f);
	
	
	
	struct Model terrain = loadModel("assets/Lakebed Temple/Dungeon/R03/model.obj");
	
	struct Object smallChest = createObject(loadModel("assets/Small/Small_Chest.obj"));
	smallChest.pos[0] = -0.89f;
	smallChest.pos[1] = -3.76f;
	smallChest.pos[2] = 4.45f;
	
	struct Object mediumChest = createObject(loadModel("assets/Medium/Medium_Chest.obj"));
	mediumChest.pos[0] = 0.89f;
	mediumChest.pos[1] = -3.76f;
	mediumChest.pos[2] = 4.45f;
	
	struct Object smallKey = createObject(loadModel("assets/Key/Small Key 3.obj"));
	smallKey.pos[1] = -2.6f;
	
	const unsigned int objectCount = 3;
	struct Object objects[objectCount];
	objects[0] = smallChest;
	objects[1] = mediumChest;
	objects[2] = smallKey;
	
	
	
	double nowTime;
	double lastTime = glfwGetTime();
	double deltaTime = 0.0;
	double tickTock = 0.0;
	double secondTimer = 0.0;
	int frames = 0;
	int ticks = 0;
	float timer = 0.0f;
	
	float objRot = 1.0f;
	//glm_vec3_rotate(cameraFront, -0.35f, cameraRight);
	//glm_vec3_rotate(cameraUp, -0.35f, cameraRight);
	cameraPitch += -0.1f;
	if     (cameraPitch >=  M_PI) cameraPitch -= 2.0f*M_PI;
	else if(cameraPitch <= -M_PI) cameraPitch += 2.0f*M_PI;
	cameraDirection[0] = cos(cameraYaw) * cos(cameraPitch);
	cameraDirection[1] = sin(cameraPitch);
	cameraDirection[2] = sin(cameraYaw) * cos(cameraPitch);
	glm_normalize_to(cameraDirection, cameraFront);
	if(cameraPitch < 0.5*M_PI && cameraPitch > -0.5*M_PI) {
		glm_vec3_crossn(cameraFront, cameraUp, cameraRight);
	} else {
		glm_vec3_crossn(cameraFront, cameraDown, cameraRight);
	}
	
	vec3 lightPos = {0.0f, 0.0f, 3.0f};
	vec3 lightColor = {1.0f, 1.0f, 1.0f};
	float lightConstant  = 1.0f;
	float lightLinear    = 0.045f;
	float lightQuadratic = 0.0075f;
	
	//forever
	while(!glfwWindowShouldClose(window)) {
		
		//tick
		while(tickTock >= 1.0) {
			switch(col) {
			case 0:
				clearG += 0.0003f;
				if(clearG >= 1.0f) col++;
				break;
			case 1:
				clearR -= 0.0003f;
				if(clearR <= 0.4f) col++;
				break;
			case 2:
				clearB += 0.0003f;
				if(clearB >= 1.0f) col++;
				break;
			case 3:
				clearG -= 0.0003f;
				if(clearG <= 0.4f) col++;
				break;
			case 4:
				clearR += 0.0003f;
				if(clearR >= 1.0f) col++;
				break;
			case 5:
				clearB -= 0.0003f;
				if(clearB <= 0.4f) col = 0;
				break;
			}
			
			vec3 cameraForwards;
			#ifdef WALKAROUND									//walkaround camera
			glm_vec3_crossn(cameraUp, cameraRight, cameraForwards);
			if(inW && !inS) {
				cameraPos[0] += cameraSpeed*cameraForwards[0];
				cameraPos[1] += cameraSpeed*cameraForwards[1];
				cameraPos[2] += cameraSpeed*cameraForwards[2];
			} else if(inS && !inW) {
				cameraPos[0] -= cameraSpeed*cameraForwards[0];
				cameraPos[1] -= cameraSpeed*cameraForwards[1];
				cameraPos[2] -= cameraSpeed*cameraForwards[2];
			} if(inA && !inD) {
				vec3 cameraStrafe;
				cameraPos[0] -= cameraSpeed*cameraRight[0];
				cameraPos[1] -= cameraSpeed*cameraRight[1];
				cameraPos[2] -= cameraSpeed*cameraRight[2];
			} else if(inD && !inA) {
				cameraPos[0] += cameraSpeed*cameraRight[0];
				cameraPos[1] += cameraSpeed*cameraRight[1];
				cameraPos[2] += cameraSpeed*cameraRight[2];
			} if(inSpace && !inLeftShift) {
				cameraPos[0] += cameraSpeed*cameraUp[0];
				cameraPos[1] += cameraSpeed*cameraUp[1];
				cameraPos[2] += cameraSpeed*cameraUp[2];
			} else if(inLeftShift && !inSpace) {
				cameraPos[0] -= cameraSpeed*cameraUp[0];
				cameraPos[1] -= cameraSpeed*cameraUp[1];
				cameraPos[2] -= cameraSpeed*cameraUp[2];
			}
			#else
			if(inW && !inS) {											// flyaround camera
				cameraPos[0] += cameraSpeed*cameraFront[0];
				cameraPos[1] += cameraSpeed*cameraFront[1];
				cameraPos[2] += cameraSpeed*cameraFront[2];
			} else if(inS && !inW) {
				cameraPos[0] -= cameraSpeed*cameraFront[0];
				cameraPos[1] -= cameraSpeed*cameraFront[1];
				cameraPos[2] -= cameraSpeed*cameraFront[2];
			} if(inA && !inD) {
				vec3 cameraStrafe;
				cameraPos[0] -= cameraSpeed*0.6f*cameraRight[0];
				cameraPos[1] -= cameraSpeed*0.6f*cameraRight[1];
				cameraPos[2] -= cameraSpeed*0.6f*cameraRight[2];
			} else if(inD && !inA) {
				cameraPos[0] += cameraSpeed*0.6f*cameraRight[0];
				cameraPos[1] += cameraSpeed*0.6f*cameraRight[1];
				cameraPos[2] += cameraSpeed*0.6f*cameraRight[2];
			}
			#endif							
			glm_vec3_crossn(cameraUp, cameraRight, cameraForwards);		//walkaround camera
			if(inW && !inS) {
				cameraPos[0] += cameraSpeed*cameraForwards[0];
				cameraPos[1] += cameraSpeed*cameraForwards[1];
				cameraPos[2] += cameraSpeed*cameraForwards[2];
			} else if(inS && !inW) {
				cameraPos[0] -= cameraSpeed*cameraForwards[0];
				cameraPos[1] -= cameraSpeed*cameraForwards[1];
				cameraPos[2] -= cameraSpeed*cameraForwards[2];
			} if(inA && !inD) {
				vec3 cameraStrafe;
				cameraPos[0] -= cameraSpeed*cameraRight[0];
				cameraPos[1] -= cameraSpeed*cameraRight[1];
				cameraPos[2] -= cameraSpeed*cameraRight[2];
			} else if(inD && !inA) {
				cameraPos[0] += cameraSpeed*cameraRight[0];
				cameraPos[1] += cameraSpeed*cameraRight[1];
				cameraPos[2] += cameraSpeed*cameraRight[2];
			} if(inSpace && !inLeftShift) {
				cameraPos[0] += cameraSpeed*cameraUp[0];
				cameraPos[1] += cameraSpeed*cameraUp[1];
				cameraPos[2] += cameraSpeed*cameraUp[2];
			} else if(inLeftShift && !inSpace) {
				cameraPos[0] -= cameraSpeed*cameraUp[0];
				cameraPos[1] -= cameraSpeed*cameraUp[1];
				cameraPos[2] -= cameraSpeed*cameraUp[2];
			}
			
			// object rotate
			//objRot += 0.009375f;
			objects[2].rot[1] += 0.015f;
			
			timer += 0.002f;
			
			ticks++;
			tickTock--;
		}
		
		//render
		if(deltaTime >= 1.0) {
			glClearColor(clearR/3.0f, clearG/3.0f, clearB/3.0f, 1.0f);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_CULL_FACE);
			
			
			
			glUseProgram(shaderProgram);
			glActiveTexture(GL_TEXTURE0);
			glUniform3fv(glGetUniformLocation(shaderProgram, "light.pos"), 1, lightPos);
			glUniform3fv(glGetUniformLocation(shaderProgram, "light.color"), 1, lightColor);
			glUniform1f(glGetUniformLocation(shaderProgram, "light.constant"), lightConstant);
			glUniform1f(glGetUniformLocation(shaderProgram, "light.linear"), lightLinear);
			glUniform1f(glGetUniformLocation(shaderProgram, "light.quadratic"), lightQuadratic);
			glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, cameraPos);
			glUniform1f(glGetUniformLocation(shaderProgram, "material.ambient"), 0.2f);
			glUniform1f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.8f);
			glUniform1f(glGetUniformLocation(shaderProgram, "material.specular"), 0.0f);
			glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 2.0f);
			glUniform3f(glGetUniformLocation(shaderProgram, "lightAmbient"), 0.0f, 0.0f, 0.0f);
			
			//view matrix
			mat4 viewMat;
			vec3 cameraTarget = {cameraPos[0]+cameraFront[0], cameraPos[1]+cameraFront[1], cameraPos[2]+cameraFront[2]};
			if(cameraPitch < 0.5*M_PI && cameraPitch > -0.5*M_PI) {
				glm_lookat(cameraPos, cameraTarget, cameraUp, viewMat);
			} else {
				glm_lookat(cameraPos, cameraTarget, cameraDown, viewMat);
			}
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, (GLfloat*)viewMat);
			
			//projection matrix
			mat4 projMat;
			glm_perspective((0.25f*M_PI), ((float)windowWidth / (float)windowHeight), 0.1f, 100.0f, projMat);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projMat);
			
			//model matrix
			mat4 modelMat;
			
			// terrain
			vec3 modelScale = { 0.5f, 0.5f, 0.5f };
			glm_scale_make(modelMat, modelScale);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (GLfloat*)modelMat);
			
			for(int i = 0; i < terrain.meshCount; i++) {
				glBindTexture(GL_TEXTURE_2D, terrain.textures[terrain.meshes[i].material]);
				glBindVertexArray(terrain.meshes[i].VAO);
				glDrawElements(GL_TRIANGLES, terrain.meshes[i].indicesCount, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
			
			glUniform3f(glGetUniformLocation(shaderProgram, "lightAmbient"), 1.0f, 1.0f, 1.0f);
			
			// objects
			vec3 modelRotX = { 1.0f, 0.0f, 0.0f };
			vec3 modelRotY = { 0.0f, 1.0f, 0.0f };
			vec3 modelRotZ = { 0.0f, 0.0f, 1.0f };
			for(int i = 0; i < objectCount; i++) {
				glm_translate_make(modelMat, objects[i].pos);
				glm_rotate(modelMat, objects[i].rot[0], modelRotX);
				glm_rotate(modelMat, objects[i].rot[1], modelRotY);
				glm_rotate(modelMat, objects[i].rot[2], modelRotZ);
				glm_scale(modelMat, objects[i].scale);
				glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (GLfloat*)modelMat);
				
				for(int ii = 0; ii < objects[i].model.meshCount; ii++) {
					glBindTexture(GL_TEXTURE_2D, objects[i].model.textures[objects[i].model.meshes[ii].material]);
					glBindVertexArray(objects[i].model.meshes[ii].VAO);
					glDrawElements(GL_TRIANGLES, objects[i].model.meshes[ii].indicesCount, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
				}
			}
			
			
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
		
		if(nowTime - secondTimer > 1.0f) {
			secondTimer++;
			printf("%d ticks, %d frames\n", ticks, frames);
			ticks = 0;
			frames = 0;
		}
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
	
}

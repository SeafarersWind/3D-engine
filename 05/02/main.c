
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
	"uniform sampler2D texture2;\n"
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

const char* billboardVertexShaderSource =
	"#version 330 core\n"
	"layout (location = 0) in vec2 aPos;\n"
	
	"out float size;\n"
	
	"uniform float aSize;\n"
	"uniform vec4 viewModel;\n"
	"uniform mat4 projection;\n"

	"void main() {\n"
	"	gl_Position = projection * (viewModel + vec4(aPos, 0.0f, 0.0f) * vec4(aSize));\n"
	"	size = aSize;\n"
	"}\0";

const char* lightFragmentShaderSource =
	"#version 330 core\n"
	"in float size;\n"
	
	"out vec4 FragColor;\n"
	
	"uniform vec2 screenspaceCenter;\n"
	"uniform float screenspaceRadius;\n"
	"uniform vec3 lightColor;\n"

	"void main() {\n"
	"	float centerDistance = 1.0f - (distance(gl_FragCoord.xy, screenspaceCenter) / (screenspaceRadius*size));\n"
	"	if(centerDistance <= 0.0f) discard;\n"
	"	centerDistance = smoothstep(0.0f, 1.0f, centerDistance);\n"
	"	FragColor = vec4(lightColor, centerDistance);\n"
	"}\0";

	  int   windowWidth  = 720;
	  int   windowHeight =  720;
const char* windowTitle  = "Waddle Dee";
const double limitFPS = 1.0f / 20.0f;
const double tickSpeed = 1.0f / 60.0f;

bool inW, inS, inA, inD, inSpace, inLeftShift;
float lastX, lastY;
bool firstMouse = true;

const float cameraSpeed = 0.04f;	
const float cameraSensitivity = 0.001f;
vec3 cameraPos   = {0.0f, 0.75f, 1.5f};
vec3 cameraFront = {0.0f, 0.0f, -1.0f};
vec3 cameraUp    = {0.0f, 1.0f, 0.0f};
vec3 cameraRight = {1.0f, 0.0f, 0.0f};
vec3 cameraDown  = {0.0f, -1.0f, 0.0f};
vec3 cameraDirection;
float cameraYaw  = -0.5f*M_PI;
float cameraPitch;
float cameraRoll;

float clearG = 1.0;
float clearB = 0.4;
float clearR = 0.4;
int col = 2;
int fade = 0;



struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 texCoords;
};

struct Texture {
	unsigned int id;
	unsigned char type;
};

struct Mesh {
	struct Vertex* vertices;
	unsigned int verticesCount;
	
	unsigned int* indices;
	unsigned int indicesCount;
	
	struct Texture* textures;
	unsigned int texturesCount;
	
	unsigned int VAO, VBO, EBO;
};

struct Mesh meshes[256];
unsigned char meshCount;



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
	float controlYaw   = xOffset * cameraSensitivity;		// flyaround camera
	float controlPitch = yOffset * cameraSensitivity;
	glm_vec3_rotate(cameraFront, controlYaw, cameraUp);
	glm_vec3_rotate(cameraRight, controlYaw, cameraUp);
	glm_vec3_rotate(cameraFront, controlPitch, cameraRight);
	glm_vec3_rotate(cameraUp, controlPitch, cameraRight);
	// float cameraYaw   -= xOffset * cameraSensitivity;	// walkaround camera
	// float cameraPitch += yOffset * cameraSensitivity;
	
	// if     (cameraYaw >=  M_PI) cameraYaw -= 2.0f*M_PI;
	// else if(cameraYaw <= -M_PI) cameraYaw += 2.0f*M_PI;
	// if     (cameraPitch >=  M_PI) cameraPitch -= 2.0f*M_PI;
	// else if(cameraPitch <= -M_PI) cameraPitch += 2.0f*M_PI;
	// if     (cameraRoll >=  M_PI) cameraPitch -= 2.0f*M_PI;
	// else if(cameraRoll <= -M_PI) cameraPitch += 2.0f*M_PI;
	
	// cameraDirection[0] = cos(cameraYaw) * cos(cameraPitch);
	// cameraDirection[1] = sin(cameraPitch);
	// cameraDirection[2] = sin(cameraYaw) * cos(cameraPitch);
	// glm_normalize_to(cameraDirection, cameraFront);
	
}



static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}



struct Mesh processMesh(struct aiMesh* mesh, const struct aiScene* scene) {
	struct Vertex* vertices = malloc(mesh->mNumVertices*sizeof(struct Vertex));
	
	unsigned int indicesCount = 0;
	for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
		indicesCount += mesh->mFaces[i].mNumIndices;
	}
	unsigned int* indices = malloc(indicesCount*sizeof(unsigned int));
	
	struct Texture* textures;

	for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
		struct Vertex vertex;
		vertex.position[0] = mesh->mVertices[i].x;
		vertex.position[1] = mesh->mVertices[i].y;
		vertex.position[2] = mesh->mVertices[i].z;
		vertex.normal[0] = mesh->mNormals[i].x;
		vertex.normal[1] = mesh->mNormals[i].y;
		vertex.normal[2] = mesh->mNormals[i].z;
		vertex.texCoords[0] = mesh->mTextureCoords[0][i].x;
		vertex.texCoords[1] = mesh->mTextureCoords[0][i].y;
		vertices[i] = vertex;
		//printf("%f, %f, %f      %f, %f, %f      %f, %f\n", vertex.position[0], vertex.position[1], vertex.position[2], vertex.normal[0], vertex.normal[1], vertex.normal[2], vertex.texCoords[0], vertex.texCoords[1]);
	}
	//printf("\n\n\n\n\n\n\n\n");
	
	indicesCount = 0;
	for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
		for(unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
			indices[indicesCount] = mesh->mFaces[i].mIndices[j];
			indicesCount++;
		}
	}
	
	if(mesh->mMaterialIndex >= 0) {
		// process material
	}
	
	struct Mesh finalMesh = {
		.vertices = vertices,
		.verticesCount = mesh->mNumVertices,
		.indices = indices,
		.indicesCount = indicesCount
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

void processNode(struct aiNode* node, const struct aiScene* scene) {
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
		meshes[meshCount] = processMesh(mesh, scene);
		meshCount++;
	}
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
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
	
	
	float vertices[] = {
		-0.5f,-0.5f,-0.5f,   1.0f, 1.0f,   0.0f, 0.0f,-1.0f,
		 0.5f,-0.5f,-0.5f,   0.0f, 1.0f,   0.0f, 0.0f,-1.0f,
		 0.5f, 0.5f,-0.5f,   0.0f, 0.0f,   0.0f, 0.0f,-1.0f,
		 0.5f, 0.5f,-0.5f,   0.0f, 0.0f,   0.0f, 0.0f,-1.0f,
		-0.5f, 0.5f,-0.5f,   1.0f, 0.0f,   0.0f, 0.0f,-1.0f,
		-0.5f,-0.5f,-0.5f,   1.0f, 1.0f,   0.0f, 0.0f,-1.0f,

		-0.5f,-0.5f, 0.5f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
		 0.5f,-0.5f, 0.5f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
		 0.5f, 0.5f, 0.5f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		 0.5f, 0.5f, 0.5f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 0.5f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		-0.5f,-0.5f, 0.5f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,

		-0.5f, 0.5f, 0.5f,   1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f,-0.5f,   0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f,-0.5f,-0.5f,   0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f,-0.5f,-0.5f,   0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f,-0.5f, 0.5f,   1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,   1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,

		 0.5f, 0.5f, 0.5f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		 0.5f, 0.5f,-0.5f,   1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		 0.5f,-0.5f,-0.5f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
		 0.5f,-0.5f,-0.5f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
		 0.5f,-0.5f, 0.5f,   0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
		 0.5f, 0.5f, 0.5f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		 
		-0.5f,-0.5f,-0.5f,   1.0f, 1.0f,   0.0f,-1.0f, 0.0f,
		 0.5f,-0.5f,-0.5f,   0.0f, 1.0f,   0.0f,-1.0f, 0.0f,
		 0.5f,-0.5f, 0.5f,   0.0f, 0.0f,   0.0f,-1.0f, 0.0f,
		 0.5f,-0.5f, 0.5f,   0.0f, 0.0f,   0.0f,-1.0f, 0.0f,
		-0.5f,-0.5f, 0.5f,   1.0f, 0.0f,   0.0f,-1.0f, 0.0f,
		-0.5f,-0.5f,-0.5f,   1.0f, 1.0f,   0.0f,-1.0f, 0.0f,
		
		-0.5f, 0.5f,-0.5f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		 0.5f, 0.5f,-0.5f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		 0.5f, 0.5f, 0.5f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
		 0.5f, 0.5f, 0.5f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f,-0.5f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f
	};
	unsigned int indices[] = {
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};
	
	float circleVertices[] = {
		 0.0f,     2.0f,
		 1.7321f, -1.0f,
		-1.7321f, -1.0f
	};
	
	vec3 cubePositions[] = { 
		{ 0.0f, 0.0f, 0.0f}
	};
	
	// vec3 cubeRot = {1.0f, 0.3f, 0.5f};
	
	vec3 lightPos = {1.4f, 2.5f, 0.4f};
	
	vec3 lightColor = {1.0f, 1.0f, 1.0f};
	float lightConstant  = 1.0f;
	float lightLinear    = 0.045f;
	float lightQuadratic = 0.0075f;
	
	
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
	
	int billboardVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(billboardVertexShader, 1, &billboardVertexShaderSource, NULL);
	glCompileShader(billboardVertexShader);
	glGetShaderiv(billboardVertexShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(billboardVertexShader, 512, NULL, infoLog);
		printf(infoLog);
	}
	
	int lightFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(lightFragmentShader, 1, &lightFragmentShaderSource, NULL);
	glCompileShader(lightFragmentShader);
	glGetShaderiv(lightFragmentShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(lightFragmentShader, 512, NULL, infoLog);
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
	
	int lightShaderProgram = glCreateProgram();
	glAttachShader(lightShaderProgram, billboardVertexShader);
	glAttachShader(lightShaderProgram, lightFragmentShader);
	glLinkProgram(lightShaderProgram);
	glGetProgramiv(lightShaderProgram, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(lightShaderProgram, 512, NULL, infoLog);
		printf(infoLog);
	}
	glDeleteShader(fragmentShader);
	glDeleteShader(lightFragmentShader);
	
	
	const struct aiScene* scene = aiImportFile( "assets/Waddle Dee/waddledee.obj",
	aiProcess_CalcTangentSpace       |
	aiProcess_Triangulate            |
	aiProcess_JoinIdenticalVertices  |
	aiProcess_SortByPType);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf(aiGetErrorString());
	}
	processNode(scene->mRootNode, scene);
	aiReleaseImport(scene);
	
	
	int circleVBO, lightVAO;
	// billboard
	glGenBuffers(1, &circleVBO);
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
	// position attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	unsigned int texture1, texture2;
	int texWidth, texHeight, nrChannels;
	
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char *texData = stbi_load("assets/Waddle Dee/5198EA4E_c.png", &texWidth, &texHeight, &nrChannels, 0);
	if(texData) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		printf("Failed to load texture.\n");
	}
	stbi_image_free(texData);
	
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.0f, 0.7f, 1.0f);
	
	glUseProgram(lightShaderProgram);
	glUniform3fv(glGetUniformLocation(lightShaderProgram, "lightColor"), 1, lightColor);
	glUniform1f(glGetUniformLocation(lightShaderProgram, "aSize"), 0.15f);
	
	
	double nowTime;
	double lastTime = glfwGetTime();
	double deltaTime = 0.0;
	double tickTock = 0.0;
	double secondTimer = 0.0;
	int frames = 0;
	int ticks = 0;
	float timer = 0.0f;
	
	float objRot = 0.0f;
	glm_vec3_rotate(cameraFront, -0.35f, cameraRight);
	glm_vec3_rotate(cameraUp, -0.35f, cameraRight);
	glm_vec3_rotate(cameraUp, 0.16f, cameraFront);
	glm_vec3_rotate(cameraRight, 0.16f, cameraFront);
	
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
			
			if(inW && !inS) {
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
			} if(inSpace && !inLeftShift) {
				cameraPos[0] += cameraSpeed*0.6f*cameraUp[0];
				cameraPos[1] += cameraSpeed*0.6f*cameraUp[1];
				cameraPos[2] += cameraSpeed*0.6f*cameraUp[2];
			} else if(inLeftShift && !inSpace) {
				cameraPos[0] -= cameraSpeed*0.6f*cameraUp[0];
				cameraPos[1] -= cameraSpeed*0.6f*cameraUp[1];
				cameraPos[2] -= cameraSpeed*0.6f*cameraUp[2];
			}
			
			// object rotate
			objRot += 0.009375f;
			
			timer += 0.002f;
			
			ticks++;
			tickTock--;
		}
		
		//render
		if(deltaTime >= 1.0) {
			glClearColor(clearR/2.0f, clearG/2.0f, clearB/2.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texture2);
			glUseProgram(shaderProgram);
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
			glUniform3f(glGetUniformLocation(shaderProgram, "lightAmbient"), clearR*1.4f+0.6f, clearG*1.4f+0.6f, clearB*1.4f+0.6f);
			
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
			vec3 modelRot = { 0.0f, 1.0f, 0.0f };
			vec3 modelScale = { 2.0f, 2.0f, 2.0f };
			glm_rotate_make(modelMat, objRot, modelRot);
			glm_scale(modelMat, modelScale);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (GLfloat*)modelMat);
			
			for(int i = 0; i < meshCount; i++) {
				glBindVertexArray(meshes[i].VAO);
				glDrawElements(GL_TRIANGLES, meshes[i].indicesCount, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
			
			
			
			// glUseProgram(lightShaderProgram);
			// glUniform3fv(glGetUniformLocation(lightShaderProgram, "lightColor"), 1, lightColor);
			
			// mat4 lightModelMat;
			// mat4 lightViewModelMat;
			// vec4 lightViewModel = {0.0f, 0.0f, 0.0f, 1.0f};
			// glm_translate_make(lightModelMat, lightPos);
			// glm_mat4_mul(viewMat, lightModelMat, lightViewModelMat);
			// glm_mat4_mulv(lightViewModelMat, lightViewModel, lightViewModel);
			// glUniform4fv(glGetUniformLocation(lightShaderProgram, "viewModel"), 1, lightViewModel);
			// glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projMat);
			// vec4 lightProjViewModel;
			// glm_mat4_mulv(projMat, lightViewModel, lightProjViewModel);
			// lightProjViewModel[0] /= lightProjViewModel[3];
			// lightProjViewModel[1] /= lightProjViewModel[3];
			// lightProjViewModel[0] += 1.0f;
			// lightProjViewModel[1] += 1.0f;
			// lightProjViewModel[0] *= windowWidth/2;
			// lightProjViewModel[1] *= windowHeight/2;
			// glUniform2f(glGetUniformLocation(lightShaderProgram, "screenspaceCenter"), lightProjViewModel[0], lightProjViewModel[1]);
			// vec4 lightProjViewModel1 = {1.0f, 0.0f, 0.0f, 0.0f};
			// glm_vec4_add(lightViewModel, lightProjViewModel1, lightProjViewModel1);
			// glm_mat4_mulv(projMat, lightProjViewModel1, lightProjViewModel1);
			// lightProjViewModel1[0] /= lightProjViewModel1[3];
			// lightProjViewModel1[0] += 1.0f;
			// lightProjViewModel1[0] *= windowWidth/2;
			// glUniform1f(glGetUniformLocation(lightShaderProgram, "screenspaceRadius"), lightProjViewModel1[0] - lightProjViewModel[0]);
			
			// glBindVertexArray(lightVAO);
			// glDrawArrays(GL_TRIANGLES, 0, 36);
			
			
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

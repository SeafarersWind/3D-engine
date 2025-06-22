
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const double limitFPS = 1.0f / 20.0f;
const double tickSpeed = 1.0f / 60.0f;

bool inW, inS, inA, inD, inSpace, inLeftShift;
float lastX = 400, lastY = 300;
bool firstMouse = true;

const float cameraSpeed = 0.04f;	
const float cameraSensitivity = 0.001f;
vec3 cameraPos   = {0.0f, 0.0f, 10.0f};
vec3 cameraFront = {0.0f, 0.0f, -1.0f};
vec3 cameraUp    = {0.0f, 1.0f, 0.0f};
vec3 cameraRight = {1.0f, 0.0f, 0.0f};
vec3 cameraDown  = {0.0f, -1.0f, 0.0f};
vec3 cameraDirection;
float cameraYaw  = -0.5f*M_PI;
float cameraPitch;
float cameraRoll;

float clearR = 1.0;
float clearG = 0.4;
float clearB = 0.4;
int col = 0;
int fade = 0;



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



int main(void) {
	glfwSetErrorCallback(error_callback);
	if(!glfwInit()) {
		printf("Failed to initialize GLFW.");
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	
	const int   windowWidth  = 1280;
	const int   windowHeight =  720;
	const char* windowTitle  = "Cube oh cube oh have you a clue";
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
	
	
	const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aCol;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    
    "out vec3 ourCol;\n"
    "out vec2 texCoord;\n"
    
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "	ourCol = aCol;\n"
    "	texCoord = aTexCoord;\n"
    "}\0";
    
    const char* fragmentShaderSource = "#version 330 core\n"
	"in vec3 ourCol;\n"
	"in vec2 texCoord;\n"
	
	"out vec4 FragColor;\n"
	
	"uniform sampler2D texture1;\n"
	"uniform sampler2D texture2;\n"
	
	"void main() {\n"
	"	FragColor = texture(texture1, texCoord);\n"
	"}\0"; 
	
	

	float vertices[] = {
	    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	unsigned int indices[] = {
	    0, 1, 3,   // first triangle
	    1, 2, 3    // second triangle
	};
	
	vec3 cubePositions[] = { 
	    { 2.8f, -0.8f, -1.5f},
	    { 0.0f,  0.1f,  3.3f},
	    {-1.5f, -2.2f,  0.5f},
	    {-3.8f, -2.0f, -4.3f},
	    { 1.3f, -2.0f,  0.4f},
	    {-1.7f,  3.0f, -2.5f},
	    { 1.5f,  0.2f,  2.2f},
	    { 2.0f,  5.0f, -6.0f},
	    { 1.5f,  2.0f,  2.4f},
	    {-1.3f,  1.0f, -3.2f}
	};
	
	int VBO, VAO, EBO;
	int vertexShader;
	int fragmentShader;
	int shaderProgram;
	
	int success;
	char infoLog[512];
	
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf(infoLog);
	}
	
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf(infoLog);
	}
	
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success) {
    	glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    	printf(infoLog);
	}
	glUseProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// // color attribute
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	// glEnableVertexAttribArray(1);
	// texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(2);
	
	glEnable(GL_DEPTH_TEST);
	
	
	unsigned int texture1, texture2;
	int texWidth, texHeight, nrChannels;
	
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	unsigned char *texData = stbi_load("mushroom.png", &texWidth, &texHeight, &nrChannels, 0);
	if(texData) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		printf("Failed to load texture.\n");
	}
	stbi_image_free(texData);
	
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	texData = stbi_load("box_top.png", &texWidth, &texHeight, &nrChannels, 0);
	if(texData) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		printf("Failed to load texture.\n");
	}
	stbi_image_free(texData);
	
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);
	
	
	double nowTime;
	double lastTime = glfwGetTime();
	double deltaTime = 0.0;
	double tickTock = 0.0;
	double secondTimer = 0.0;
	int frames = 0;
	int ticks = 0;
	float timer = 0.0f;
	
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
				glm_vec3_normalize(cameraFront);
			} if(inS && !inW) {
				cameraPos[0] -= cameraSpeed*cameraFront[0];
				cameraPos[1] -= cameraSpeed*cameraFront[1];
				cameraPos[2] -= cameraSpeed*cameraFront[2];
				glm_vec3_normalize(cameraFront);
			} if(inA && !inD) {
				vec3 cameraStrafe;
				glm_vec3_crossn(cameraFront, cameraUp, cameraStrafe);
				cameraPos[0] -= cameraSpeed*0.6f*cameraStrafe[0];
				cameraPos[1] -= cameraSpeed*0.6f*cameraStrafe[1];
				cameraPos[2] -= cameraSpeed*0.6f*cameraStrafe[2];
				glm_vec3_normalize(cameraFront);
			} if(inD && !inA) {
				vec3 cameraStrafe;
				glm_vec3_crossn(cameraFront, cameraUp, cameraStrafe);
				cameraPos[0] += cameraSpeed*0.6f*cameraStrafe[0];
				cameraPos[1] += cameraSpeed*0.6f*cameraStrafe[1];
				cameraPos[2] += cameraSpeed*0.6f*cameraStrafe[2];
				glm_vec3_normalize(cameraFront);
			} if(inSpace && !inLeftShift) {
				cameraPos[0] += cameraSpeed*0.6f*cameraUp[0];
				cameraPos[1] += cameraSpeed*0.6f*cameraUp[1];
				cameraPos[2] += cameraSpeed*0.6f*cameraUp[2];
			} else if(inLeftShift && !inSpace) {
				cameraPos[0] -= cameraSpeed*0.6f*cameraUp[0];
				cameraPos[1] -= cameraSpeed*0.6f*cameraUp[1];
				cameraPos[2] -= cameraSpeed*0.6f*cameraUp[2];
			}
			
			timer += 0.002f;
			
			ticks++;
			tickTock--;
		}
		
		//render
		if(deltaTime >= 1.0) {
			glClearColor(clearR, clearG, clearB, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	        glActiveTexture(GL_TEXTURE0);
	        glBindTexture(GL_TEXTURE_2D, texture1);
	        glActiveTexture(GL_TEXTURE1);
	        glBindTexture(GL_TEXTURE_2D, texture2);
			glUseProgram(shaderProgram);
			glBindVertexArray(VAO);
			glBindVertexArray(VAO);
			
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
			glm_perspective((0.25f*M_PI), (800.0f/600.0f), 0.1f, 100.0f, projMat);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projMat);
			
			//model matrix
			for(unsigned int i = 0; i < 10; i++) {
			    mat4 modelMat;
			    vec3 modelRot = {1.0f, 0.3f, 0.5f};
			    glm_translate_make(modelMat, cubePositions[i]);
			    glm_rotate(modelMat, (timer*(i+0.7f)), modelRot);
				glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (GLfloat*)modelMat);

			    glDrawArrays(GL_TRIANGLES, 0, 36);
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

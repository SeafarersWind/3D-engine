
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const double limitFPS = 1.0f / 60.0f;

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
		case GLFW_KEY_SPACE:
			fade = 1;
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		}
		break;
	case GLFW_RELEASE:
		switch(key) {
		case GLFW_KEY_SPACE:
			fade = 0;
			break;
		}
	}
	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
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
	const char* windowTitle  = "Rectangle oh I wonder";
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, 0, 0);
	if (!window) {
		printf("Failed to create GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	
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
	"	FragColor = texture(texture1, texCoord) * vec4(ourCol, 1.0);\n"
	"}\0"; 
	
	float vertices[] = {
	     0.5f, 0.5f, 0.0f,   1.0f, 0.5f, 0.0f,   1.0f, 0.0f, // top right
	     0.5f,-0.5f, 0.0f,   0.0f, 1.0f, 0.5f,   1.0f, 1.0f, // bottom right
	    -0.5f,-0.5f, 0.0f,   0.5f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom left
	    -0.5f, 0.5f, 0.0f,   0.4f, 0.1f, 0.1f,   0.0f, 0.0f  // top left 
	};
	unsigned int indices[] = {
	    0, 1, 3,   // first triangle
	    1, 2, 3    // second triangle
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);
	
	
	unsigned int texture1, texture2;
	int texWidth, texHeight, nrChannels;
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	unsigned char *texData = stbi_load("water.png", &texWidth, &texHeight, &nrChannels, 0);
	if(texData) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		printf("Failed to load texture.\n");
	}
	stbi_image_free(texData);
	
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	texData = stbi_load("hello.png", &texWidth, &texHeight, &nrChannels, 0);
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
	double secondTimer = 0.0;
	int frames = 0;
	int ticks = 0;
	float rotation = 0.0f;
	
	//forever
	while(!glfwWindowShouldClose(window)) {
		
		
		//render
		glClearColor(clearR, clearG, clearB, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		
		
		//tick
		while(deltaTime >= 1.0) {
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
			
			mat4 modelMat;
			vec3 modelAxis = {1.0f, 0.0f, 0.0f};
			glm_rotate_make(modelMat, (1.694444f*M_PI), modelAxis);
			vec3 modelRotAxis = {0.0f, 0.0f, 1.0f};
			glm_rotate(modelMat, rotation, modelRotAxis);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (GLfloat*)modelMat);
			mat4 viewMat;
			vec3 viewPos = {0.0f, 0.0f, -3.0f};
			glm_translate_make(viewMat, viewPos);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, (GLfloat*)viewMat);
			mat4 projMat;
			glm_perspective((0.25f*M_PI), (800.0f/600.0f), 0.1f, 100.0f, projMat);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projMat);
			rotation += 0.005f;
			
			ticks++;
			deltaTime--;
		}
		
		
		//render
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);
		frames++;
		
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		nowTime = glfwGetTime();
		deltaTime += (nowTime - lastTime) / limitFPS;
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

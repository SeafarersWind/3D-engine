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



#define WINDOW_WIDTH  720
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "Quiet Field"
#define WINDOW_FULLSCREEN true

const double FRAMES_PER_SECOND = 1.0f / 20.0f;
const double TICK_SPEED = 1.0f / 20.0f;

const float CAMERA_SPEED = 0.04f;	
const float CAMERA_SENSITIVITY = 0.002f;

const vec3 UP = {0.0f, 1.0f, 0.0f};

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



#include "Window.c"

#include "Shader.c"

#include "Model.c"

int main(void) {
	GLFWwindow* window = initWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, WINDOW_FULLSCREEN);
	if(!window) return -1;
	
	
	
	// shaders
	int vertexShader = createShader("vertexShader.glsl", GL_VERTEX_SHADER);
	if(!vertexShader) return -1;
	int skelVertexShader = createShader("vertexShaderSkel.glsl", GL_VERTEX_SHADER);
	if(!skelVertexShader) return -1;
	int billboardVertexShader = createShader("vertexShaderBillboard.glsl", GL_VERTEX_SHADER);
	if(!billboardVertexShader) return -1;
	int fragmentShader = createShader("fragmentShader.glsl", GL_FRAGMENT_SHADER);
	if(!fragmentShader) return -1;
	int lightFragmentShader = createShader("fragmentShaderLight.glsl", GL_FRAGMENT_SHADER);
	if(!lightFragmentShader) return -1;
	
	int shaderProgram = createShaderProgram("shaderProgram", vertexShader, fragmentShader);
	if(!shaderProgram) return -1;
	int charShaderProgram = createShaderProgram("charShaderProgram", skelVertexShader, fragmentShader);
	if(!charShaderProgram) return -1;
	int lightShaderProgram = createShaderProgram("lightShaderProgram", billboardVertexShader, lightFragmentShader);
	if(!lightShaderProgram) return -1;
	
	glDeleteShader(vertexShader);
	glDeleteShader(skelVertexShader);
	glDeleteShader(billboardVertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(lightFragmentShader);
	
	glUseProgram(charShaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.0f, 0.7f, 1.0f);
	
	
	
	// terrian
	struct Model terrain = loadModel("assets/Terrain/Field.obj");
	struct Terrain geometry = createTerrain("assets/Terrain/Field.obj");
	
	
	
	// objects
printf("icosa");
	struct Object icosa = createObject(loadModel("assets/Character/icosa2.dae"));
printf("done");
	icosa.pos[0] = 00.0f;
	icosa.pos[1] = 00.0f;
	icosa.pos[2] = 00.0f;
	
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
			glUseProgram(charShaderProgram);
			glUniformMatrix4fv(glGetUniformLocation(charShaderProgram, "view"), 1, GL_FALSE, (GLfloat*)viewMat);
			glUniformMatrix4fv(glGetUniformLocation(charShaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projMat);
			glActiveTexture(GL_TEXTURE0);
			glUniform3fv(glGetUniformLocation(charShaderProgram, "light.pos"), 1, lightPos);
			glUniform3fv(glGetUniformLocation(charShaderProgram, "light.color"), 1, lightColor);
			glUniform1f(glGetUniformLocation(charShaderProgram, "light.constant"), lightConstant);
			glUniform1f(glGetUniformLocation(charShaderProgram, "light.linear"), lightLinear);
			glUniform1f(glGetUniformLocation(charShaderProgram, "light.quadratic"), lightQuadratic);
			glUniform3f(glGetUniformLocation(charShaderProgram, "skyColor"), clearR, clearG, clearB);
			glUniform3fv(glGetUniformLocation(charShaderProgram, "viewPos"), 1, cameraPos);
			glUniform1f(glGetUniformLocation(charShaderProgram, "material.ambient"), 0.2f);
			glUniform1f(glGetUniformLocation(charShaderProgram, "material.diffuse"), 0.8f);
			glUniform1f(glGetUniformLocation(charShaderProgram, "material.specular"), 0.0f);
			glUniform1f(glGetUniformLocation(charShaderProgram, "material.shininess"), 4.0f);
			vec3 modelRotX = { 1.0f, 0.0f, 0.0f };
			vec3 modelRotY = { 0.0f, 1.0f, 0.0f };
			vec3 modelRotZ = { 0.0f, 0.0f, 1.0f };
			for(int i = 0; i < objectCount; i++) {
				glm_translate_make(modelMat, objects[i].pos);
				glm_rotate(modelMat, objects[i].rot[0], modelRotX);
				glm_rotate(modelMat, objects[i].rot[1], modelRotY);
				glm_rotate(modelMat, objects[i].rot[2], modelRotZ);
				glm_scale(modelMat, objects[i].scale);
				glUniformMatrix4fv(glGetUniformLocation(charShaderProgram, "model"), 1, GL_FALSE, (GLfloat*)modelMat);
				glUniform3f(glGetUniformLocation(charShaderProgram, "objectColor"), 0.8f, 0.5f, 0.4f );
				
				//vec3 objectColor[6] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 1.0f } };
				for(int ii = 0; ii < objects[i].model.meshCount; ii++) {
					//glUniform3f(glGetUniformLocation(charShaderProgram, "objectColor"), objectColor[ii%6][0], objectColor[ii%6][1], objectColor[ii%6][2]);
					glUniformMatrix4fv(glGetUniformLocation(charShaderProgram, "bones"), 4, GL_FALSE,
					  objects[i].model.meshes[ii].boneOffsets[0][0]);
					glBindTexture(GL_TEXTURE_2D, objects[i].model.textures[objects[i].model.meshes[ii].material]);
					glBindVertexArray(objects[i].model.meshes[ii].VAO);
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
		deltaTime += (nowTime - lastTime) / FRAMES_PER_SECOND;
		tickTock  += (nowTime - lastTime) / TICK_SPEED;
		lastTime = nowTime;
		
		if(nowTime - secondTimer > 0.125f) {
			secondTimer+= 0.125f;
			//printf("%d ticks, %d frames\n", ticks, frames);
			ticks = 0;
			frames = 0;
		}
	}
	
	
	
	// end
	glfwDestroyWindow(window);
	glfwTerminate();
	
}

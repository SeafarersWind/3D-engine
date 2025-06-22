
int windowWidth;
int windowHeight;

bool inW, inS, inA, inD, inSpace, inLeftShift;
float lastX,lastY;
bool firstMouse = true;



static void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
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
	cameraYaw   -= xOffset * CAMERA_SENSITIVITY;	// walkaround camera
	cameraPitch += yOffset * CAMERA_SENSITIVITY;
	
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
	float controlYaw   = xOffset * CAMERA_SENSITIVITY;		// flyaround camera
	float controlPitch = yOffset * CAMERA_SENSITIVITY;
	glm_vec3_rotate(cameraFront, controlYaw, cameraUp);
	glm_vec3_rotate(cameraRight, controlYaw, cameraUp);
	glm_vec3_rotate(cameraFront, controlPitch, cameraRight);
	glm_vec3_rotate(cameraUp, controlPitch, cameraRight);
	
	
	#elif defined(ICOSA)
	cameraYaw   -= xOffset * CAMERA_SENSITIVITY;				// orbit icosa
	cameraPitch += yOffset * CAMERA_SENSITIVITY;
	
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

GLFWwindow* initWindow(int width, int height, char* title, bool fullscreen) {
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
	
	if(fullscreen) {
		windowWidth  = mode->width;
		windowHeight = mode->height;
	} else {
		windowWidth  = width;
		windowHeight = height;
	}
	
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, title, monitor, NULL);
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

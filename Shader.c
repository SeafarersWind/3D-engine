
GLuint createShader(char* shaderPath, GLenum shaderType) {
	FILE* shaderFile = fopen(shaderPath, "rb");
	if(!shaderFile) {
		printf("Failed to open %s. ", shaderPath);
		return 0;
	}
	fseek(shaderFile, 0, SEEK_END);
	long shaderFileLength = ftell(shaderFile);
	GLchar* shaderContent = malloc(shaderFileLength);
	rewind(shaderFile);
	fread(shaderContent, 1, shaderFileLength, shaderFile);
	fclose(shaderFile);
	
	int success;
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, (const GLchar**)&shaderContent, (const GLint*)&shaderFileLength);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("%s:\n%s", shaderPath, infoLog);
		return 0;
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
		return 0;
	}
	
	return shaderProgram;
}

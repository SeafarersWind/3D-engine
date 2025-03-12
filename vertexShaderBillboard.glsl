#version 330 core
layout (location = 0) in vec2 aPos;
	
out float size;
	
uniform float aSize;
uniform vec4 viewModel;
uniform mat4 projection;

void main() {
	gl_Position = projection * (viewModel + vec4(aPos, 0.0f, 0.0f) * vec4(aSize));
	size = aSize;
}

#version 440

// Model view projection matrix
uniform mat4 MVP;

// Incoming value for the position
layout(location = 0) in vec3 position;

layout(location = 1) out vec4 pos;

// Main vertex shader function
void main() {
  // Calculate screen position of vertex
  pos = MVP * vec4(position, 1.0);
  gl_Position = pos;
}
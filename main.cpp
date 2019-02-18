#include <lib/glm/glm/glm.hpp>
#include <graphics_framework.h>
#include <iostream>
#include <math.h>
#include "audio_handler.h"



using namespace std;
using namespace graphics_framework;
using namespace glm;

effect eff;
float elapsed_time = 0;
float control1 = 0.0f;
float control2 = 0.0f;



geometry screen_quad;

bool load_content() {
	// Screen quad
	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),	vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.set_type(GL_TRIANGLE_STRIP);
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);

	Audio_handler ah;

	// Load in shaders
	eff.add_shader("res/shaders/Fractal.vert", GL_VERTEX_SHADER);
	eff.add_shader("res/shaders/glowing.frag", GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();
	return true;
}


bool update(float delta_time)
{
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT))
	{
		control2 -= 0.5f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT))
	{
		control2 += 0.5f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN))
	{
		control1 -= 0.5f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP))
	{
		control1 += 0.5f * delta_time;
	}

	elapsed_time += delta_time / 5.0f;
	//cout << sinf(control1) << endl;
	return true;
}

bool render() {
	// Bind effect
	renderer::bind(eff);
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(mat4(1.0f)));
	glUniform1f(eff.get_uniform_location("aspect_ratio"), renderer::get_screen_aspect());
	glUniform1f(eff.get_uniform_location("control1"), control1);
	glUniform1f(eff.get_uniform_location("control2"), control2);
	// Render geometry
	renderer::render(screen_quad);
	return true;
}

void main() {
	// Create application
	app application("Graphics Coursework");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}
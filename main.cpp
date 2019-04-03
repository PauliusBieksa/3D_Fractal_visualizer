#include <glm/glm.hpp>
#include <graphics_framework.h>
#include <iostream>
#include <math.h>
#include "audio_handler.h"



using namespace std;
using namespace graphics_framework;
using namespace glm;

effect eff;
texture colour_gradient;
float elapsed_time = 0;
bool fullscreen = false;

Audio_handler ah;
sound_attributes sa;
sound_attributes control_params;

geometry screen_quad;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

float lerp(float start, float end, float alpha)
{
	clamp(alpha, 0.0f, 1.0f);
	float d = end - start;
	return start += d * alpha;
}

bool load_content() {
	renderer::set_screen_dimensions(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Screen quad
	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),	vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.set_type(GL_TRIANGLE_STRIP);
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);

	control_params.pitch = 0.0;
	control_params.rms = 0.0;
	control_params.spectral_centroid = 0.0;
	control_params.spectral_crest = 0.0;
	control_params.spectral_rolloff = 0.0;
	control_params.zcr = 0.0;

	colour_gradient = texture("res/textures/red_blue_gradient.png");

	// Load in shaders
	eff.add_shader("res/shaders/Fractal.vert", GL_VERTEX_SHADER);
	eff.add_shader("res/shaders/glowing.frag", GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();

	// Bind only effect and set MVP uniform for a quad
	renderer::bind(eff);
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(mat4(1.0f)));
	return true;
}


bool update(float delta_time)
{
	static float cd = 0.0f;
	cd -= delta_time;
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_ENTER) && cd < 0.0f)
	{
		cd = 0.5f;
		if (fullscreen)
		{
			renderer::set_screen_mode(renderer::windowed);
			renderer::set_screen_dimensions(SCREEN_WIDTH, SCREEN_HEIGHT);
		}
		else
			renderer::set_screen_mode(renderer::fullscreen);
		fullscreen = !fullscreen;
	}

	auto start = chrono::steady_clock::now();

	// Get sound attributes for the video frame
	sa = ah.update();
	// Check for nan and use previous values for sound below treshold
	if (!(isnan(sa.pitch) || sa.rms < 0.001f))
	{
		//control_params.pitch = sa.pitch;
		float p = sa.pitch;

		// 0 to 1 in human hearing range
		//control_params.pitch = (control_params.pitch - log2f(20)) / (log2f(20000) - log2f(20));

		// 0 to 1 in approximate standard tuned guitar range
		p = (logf(p) - logf(80.0f)) / (logf(1600.0f) - logf(80.0f));

		// Log can produce nan on some inputs
		if (!isnan(p))
		{
			p *= 9.0f;
			p += 12.0f;
			control_params.pitch = lerp(control_params.pitch, p, delta_time * 10.0f);
		}
		if (isnan(control_params.pitch))
			control_params.pitch = 0.5f;

		control_params.spectral_centroid = lerp(control_params.spectral_centroid, sa.spectral_centroid * 0.8f, sa.rms * sa.rms);
		control_params.spectral_crest = lerp(control_params.spectral_crest, sa.spectral_crest, sa.rms * sa.rms);
		control_params.spectral_rolloff = lerp(control_params.spectral_rolloff, sa.spectral_rolloff * 15.0f, sa.rms * sa.rms);
		control_params.zcr = lerp(control_params.zcr, sa.zcr * 0.15f, sa.rms * sa.rms);
		control_params.rms = sa.rms;
	}

	elapsed_time += delta_time / 5.0f;
	return true;
}

bool render() {
	glUniform1f(eff.get_uniform_location("aspect_ratio"), renderer::get_screen_aspect());
	glUniform1f(eff.get_uniform_location("iterated_angle"), control_params.pitch);
	glUniform1f(eff.get_uniform_location("cam_rotate"), elapsed_time);
	float sc01 = control_params.spectral_centroid * 0.1;
	glUniform1f(eff.get_uniform_location("col_picker"),sc01);
	vec3 tmp = vec3(control_params.spectral_crest, control_params.spectral_rolloff, control_params.zcr);
	glUniform3fv(eff.get_uniform_location("rot_axis"), 1, value_ptr(tmp));
	// Render geometry
	renderer::render(screen_quad);
	return true;
}

void main()
{
	// Initialize the audio handler
	while (ah.initialize_custom() != 0)
		printf("\nIncompatible audio stream parameters. Do not choose an output on a different device than the input.\n");
	//ah.initialize_default();

	// Create application
	app application("Fractals");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}
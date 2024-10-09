#include "renderer.h"
#include <cstdlib>
#include <iostream>
#include <GL/freeglut_std.h>
#include <sstream>
#include <filesystem>

Renderer::Renderer(const std::string& title, int display_width, int display_height, int display_scale, int debug_width) :
	title(title),
	display_width(display_width),
	display_height(display_height),
	display_scale(display_scale),
	debug_width(debug_width),
	window(nullptr)
{
}

int Renderer::init(int argc, char* argv[])
{
	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	glutInit(&argc, argv);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	return EXIT_SUCCESS;
}

int Renderer::create_window()
{
	window = glfwCreateWindow((display_width + debug_width) * display_scale, display_height * display_scale, title.c_str(), NULL, NULL);
	if (!window) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	return EXIT_SUCCESS;
}

void Renderer::set_key_callback(GLFWkeyfun callback) {
	glfwSetKeyCallback(window, callback);
}

void Renderer::draw(const Emulator& emulator) const
{
	glClear(GL_COLOR_BUFFER_BIT);

	draw_display(emulator);
	draw_debug(emulator);

	glfwSwapBuffers(window);
}

void Renderer::draw_display(const Emulator& emulator) const
{
	for (int y = 0; y < display_height; ++y) {
		for (int x = 0; x < display_width; ++x) {
			if (emulator.display[x + y * display_width]) {
				draw_display_square(x, y);
			}
		}
	}
}

void Renderer::draw_debug(const Emulator& emulator) const
{
	const float scale = 0.125f;
	const float line_height = 1.5f;
	const float column_1_x = 1.0f;
	const float column_2_x = 8.0f;

	for (uint16_t i = 0; i < 16; ++i) {
		std::ostringstream str_stream;
		str_stream << std::uppercase << std::hex << "V" << i << " = #" << std::setw(2) << std::setfill('0') << static_cast<int>(emulator.v[i]);
		draw_debug_text(str_stream.str(), column_1_x, (i + 1) * line_height, scale);
	}

	{
		std::ostringstream str_stream;
		str_stream << std::uppercase << std::hex << "DT = #" << std::setw(2) << std::setfill('0') << static_cast<int>(emulator.dt);
		draw_debug_text(str_stream.str(), column_2_x, 1.0f * line_height, scale);
	}

	{
		std::ostringstream str_stream;
		str_stream << std::uppercase << std::hex << "ST = #" << std::setw(2) << std::setfill('0') << static_cast<int>(emulator.st);
		draw_debug_text(str_stream.str(), column_2_x, 2.0f * line_height, scale);
	}

	{
		std::ostringstream str_stream;
		str_stream << std::uppercase << std::hex << " I = #" << std::setw(4) << std::setfill('0') << static_cast<int>(emulator.i);
		draw_debug_text(str_stream.str(), column_2_x, 5.0f * line_height, scale);
	}

	{
		std::ostringstream str_stream;
		str_stream << std::uppercase << std::hex << "PC = #" << std::setw(4) << std::setfill('0') << static_cast<int>(emulator.pc);
		draw_debug_text(str_stream.str(), column_2_x, 7.0f * line_height, scale);
	}

	{
		std::ostringstream str_stream;
		str_stream << std::uppercase << std::hex << "SP = #" << std::setw(2) << std::setfill('0') << static_cast<int>(emulator.sp);
		draw_debug_text(str_stream.str(), column_2_x, 8.0f * line_height, scale);
	}
}

void Renderer::poll_events()
{
	glfwPollEvents();
}

bool Renderer::should_close() const
{
	return glfwWindowShouldClose(window);
}

void Renderer::close()
{
	glfwTerminate();
}

void Renderer::draw_display_square(int x, int y) const
{
	// Convert the top-left corner position to NDC
	float ndc_x = 2.0f * (static_cast<float>(x) / (display_width + debug_width)) - 1.0f;
	float ndc_y = 1.0f - 2.0f * (static_cast<float>(y) / display_height);

	// Convert size to NDC scale relative to the window dimensions
	float ndc_size_x = 2.0f * (1.0f / (display_width + debug_width));
	float ndc_size_y = 2.0f * (1.0f / display_height);

	glBegin(GL_QUADS);
	glVertex2f(ndc_x, ndc_y);							// Top-left corner
	glVertex2f(ndc_x + ndc_size_x, ndc_y);				// Top-right corner
	glVertex2f(ndc_x + ndc_size_x, ndc_y - ndc_size_y); // Bottom-right corner
	glVertex2f(ndc_x, ndc_y - ndc_size_y);				// Bottom-left corner
	glEnd();
}

void Renderer::draw_debug_text(const std::string& text, float x, float y, float scale) const
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	{
		glLoadIdentity();
		glOrtho(0, (display_width + debug_width) * display_scale, 0, display_height * display_scale, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		{
			glLoadIdentity();
			glTranslatef((x + display_width) * display_scale, (display_height - y) * display_scale, 0.0f);
			glScalef(scale, scale, 1.0f);
			for (const char c : text) {
				glutStrokeCharacter(GLUT_STROKE_ROMAN, (int)c);
			}
			glScalef(1.0f, 1.0f, 1.0f);
		}
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
	}
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

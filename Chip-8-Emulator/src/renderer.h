#pragma once
#include <string>
#include <GLFW/glfw3.h>
#include <iostream>
#include "emulator.h"
#include <ostream>

class Renderer
{
public:
	Renderer(const std::string& title, int display_width, int display_height, int display_scale, int debug_width);

	int init(int argc, char* argv[]);
	int create_window();
	void set_key_callback(GLFWkeyfun callback);
	void draw(const Emulator& emulator) const;
	void poll_events();
	bool should_close() const;
	void close();

private:
	std::string title;
	int display_width;
	int display_height;
	int display_scale;
	int debug_width;
	GLFWwindow* window;

	void draw_display(const Emulator& emulator) const;
	void draw_debug(const Emulator& emulator) const;

	void draw_display_square(int x, int y) const;
	void draw_debug_text(const std::string& text, float x, float y, float scale) const;
};
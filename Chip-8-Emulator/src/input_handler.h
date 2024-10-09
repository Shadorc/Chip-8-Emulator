#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>
#include <cstdint>

static uint16_t inputs_mask;

#if _DEBUG
static bool debug_mode;
static bool step;
#endif

// Keys are mapped using Qwerty layout
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static const std::unordered_map<int, uint8_t> keyboard_map = {
		{'1', 0x1},
		{'2', 0x2},
		{'3', 0x3},
		{'4', 0xC},
		{'q', 0x4}, {'Q', 0x4},
		{'w', 0x5}, {'W', 0x5},
		{'e', 0x6}, {'E', 0x6},
		{'r', 0xD}, {'R', 0xD},
		{'a', 0x7}, {'A', 0x7},
		{'s', 0x8}, {'S', 0x8},
		{'d', 0x9}, {'D', 0x9},
		{'f', 0xE}, {'F', 0xE},
		{'z', 0xA}, {'Z', 0xA},
		{'x', 0x0}, {'X', 0x0},
		{'c', 0xB}, {'C', 0xB},
		{'v', 0xF}, {'V', 0xF}
	};

	auto it = keyboard_map.find(key);
	if (it != keyboard_map.end()) {
		uint8_t mapped_key = it->second;

		switch (action) {
		case GLFW_PRESS:
			inputs_mask |= (1 << mapped_key);
			break;
		case GLFW_RELEASE:
			inputs_mask &= ~(1 << mapped_key);
			break;
		}

		return;
	}

#if _DEBUG
	if (action == GLFW_PRESS) {
		switch (key) {
		case 'b':
		case 'B':
			debug_mode = !debug_mode;
			break;
		case 'n':
		case 'N':
			step = true;
			break;
		}
	}
#endif
}


#include "emulator.h"
#include "renderer.h"
#include "frequency_lock.h"
#include "input_handler.h"
#include <iostream>

int main(int argc, char* argv[])
{
	std::string program_path = "resources/programs/Chip8 emulator Logo [Garstyciuks].ch8";

	Emulator emulator;
	if (emulator.init(program_path) == EXIT_FAILURE) {
		std::cerr << "Failed to initialize emulator" << std::endl;
		return EXIT_FAILURE;
	}

	Renderer renderer("Chip-8 Emulator", Emulator::DISPLAY_WIDTH, Emulator::DISPLAY_HEIGHT, 16, 16);
	if (renderer.init(argc, argv) == EXIT_FAILURE) {
		std::cerr << "Failed to initialize renderer" << std::endl;
		return EXIT_FAILURE;
	}

	if (renderer.create_window() == EXIT_FAILURE) {
		std::cerr << "Failed to show window" << std::endl;
		return EXIT_FAILURE;
	}

	renderer.set_key_callback(key_callback);

	bool running = true;
	while (running && !renderer.should_close()) {
		FrequencyLock loop_frequency_setter(Emulator::CPU_FREQUENCY);

#if _DEBUG
		if (!debug_mode || debug_mode && step)
#endif
		{
			running = emulator.cycle();
			if (emulator.draw_flag) {
				renderer.draw(emulator);
			}

#if _DEBUG
			step = false;
#endif
			}

		renderer.poll_events();
		emulator.inputs_mask = inputs_mask;
		}

	renderer.close();

	return EXIT_SUCCESS;
	}


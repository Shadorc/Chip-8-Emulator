#include "frequency_lock.h"
#include <iostream>
#include <thread>

FrequencyLock::FrequencyLock(int frequency) :
	frame_duration(1000 / frequency),
	start_time(std::chrono::high_resolution_clock::now())
{
}

FrequencyLock::~FrequencyLock()
{
	auto end_time = std::chrono::high_resolution_clock::now();
	auto elapsed_time = end_time - start_time;

	if (elapsed_time < frame_duration) {
		std::this_thread::sleep_for(frame_duration - elapsed_time);
	}
}

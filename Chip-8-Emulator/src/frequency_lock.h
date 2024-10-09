#pragma once
#include <chrono>

class FrequencyLock
{
public:
	FrequencyLock(int frequency);
	~FrequencyLock();

private:
	std::chrono::milliseconds frame_duration;
	std::chrono::steady_clock::time_point start_time;
};


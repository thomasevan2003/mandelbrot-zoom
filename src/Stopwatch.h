#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>

class Stopwatch {
	public:
		std::chrono::high_resolution_clock::time_point start_time;
		void start();
		double time();
};

#endif
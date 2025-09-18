#include "Stopwatch.h"

void Stopwatch::start() {
	start_time = std::chrono::high_resolution_clock::now();
}

double Stopwatch::time() {
	std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(time-start_time);
	return elapsed.count();
}
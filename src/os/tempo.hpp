#ifndef __TEMPO_H__
#define __TEMPO_H__

#include "platform.hpp"

#ifdef ENABLE_TEMPO

#define MEASURE_ELAPSED_TIME(txt, p, a) { \
static uint64_t avg_frame_time = 0; \
static uint64_t frame_count = 0; \
static auto now = std::chrono::high_resolution_clock::now(); \
{ a } \
avg_frame_time += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count(); \
frame_count++; \
now = std::chrono::high_resolution_clock::now(); \
if (frame_count % p == 0) { \
	printf(txt##"Avg frame time: %f ms\n", avg_frame_time / (double)frame_count); } \
}
#else
#define MEASURE_ELAPSED_TIME(txt, p, a) { a }
#endif

#endif
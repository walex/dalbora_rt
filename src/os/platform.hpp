#ifndef __os_platform_hpp__
#define __os_platform_hpp__

#ifdef _WIN32

#ifdef _WIN64

#define WINDOWS_x64_PLATFORM 2
#define WINDOWS_PLATFORM WINDOWS_x64_PLATFORM

#else

#define WINDOWS_x86_PLATFORM 1
#define WINDOWS_PLATFORM WINDOWS_x86_PLATFORM

#endif

#elif __ANDROID__

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/bitmap.h>
#include <android/log.h>

#define ANDROID_PLATFORM

#elif __APPLE__

#include "TargetConditionals.hpp"

#define IOS_SIM_PLATFORM 1
#define IOS_DEVICE_PLATFORM 2

#if TARGET_IPHONE_SIMULATOR

#define IOS_PLATFORM IOS_SIM_PLATFORM

#elif TARGET_OS_IPHONE

#define IOS_PLATFORM IOS_DEVICE_PLATFORM

#elif TARGET_OS_MAC

#error "mac osx is not supported"

#else

#error "not supported Apple platform"

#endif

#elif __EMSCRIPTEN__

#define EMSCRIPTEN_PLATFORM 1

#include <emscripten.h>

#elif __linux__

#define LINUX_PLATFORM

#elif __unix__

#error "not supported unix platform"

#elif defined(_POSIX_VERSION)

#error "not supported posix version"

#else

#error "not supported platform"

#endif

#if defined(__clang__)

#define INLINE_METHOD(m) inline m

#if defined(ANDROID_PLATFORM)

/* *** ndk r16b *** */
/*
Opencv 3.3 and 3.4 (static or shared) are compiled with gnustl libraries.
So we were forced to compile this library using also gnustl libraries.
We had to implement by "hand" some std c++ 11 functions needed by this system because gnustl still does not implements it.
gnustl and gcc aparently will be deprecated on future ndk versions.
Ideally we should compile this library with libc++ but opencv throws link errors because gnustl is not entirely compatible with libc++.
We hope that on future releases of opencv or ndk one of this scenarios will occur.
1) Have an android version of opencv libraries entirely compiled with libc++,
at the moment is not possible as libopencv_java3.so only can be built with gnustl ( see opencv toolchain )
2) Wait for next version of ndk (17 or superior) with the hope that libc++ turns more compatible with gnustl.
*/
// #define ANDROID_GNU_STL

#endif

#elif defined(__GNUC__) || defined(__GNUG__)

#define INLINE_METHOD(m) inline m

#elif defined(_MSC_VER)

#if defined(WINDOWS_PLATFORM)

#define NOMINMAX
#include <Windows.h>

#else

#error "Only windows platform is supported on Microsoft Visul Studio"

#endif

#define __PRETTY_FUNCTION__ __FUNCSIG__
#define INLINE_METHOD(m) __forceinline m

#endif

#ifndef __STDC_LIB_EXT1__

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)

#define timer_localtime(a, b) localtime_r(a, b);
#define strcpy_s(a, b, c) strcpy(a, c)
#define strncpy_s(a, b, c, d) strncpy(a, c, b);

// #define sprintf_s(a,b,c,...) sprintf(a,c, __VA_ARGS__)
#define sprintf_s(a, b, ...) sprintf(a, __VA_ARGS__)
#define localtime_s(a, b) localtime_r(b, a)
#elif defined(_MSC_VER)

#define timer_localtime(a, b) localtime_s(b, a);

#endif

#else

#define timer_localtime std::localtime_s

#endif

#define SAFE_RELEASE(a) \
	if (a)              \
		a->release();
#define SAFE_RELEASE2(a) \
	if (a)               \
		a->Release();
#define SAFE_DELETE(a) \
	if (a)             \
		delete a;
#define SAFE_DELETE_ARRAY(a) \
	if (a)                   \
		delete[] a;
#define SAFE_INVOKE(a, b) \
	if (a)                \
	a->b
#define SAFE_DELETE_LIST(l) \
	for (auto *p : l)       \
	{                       \
		SAFE_DELETE(p);     \
	}

#ifdef DEBUG

#define ASSERT_EXPR(expr) assert(expr)
#define ASSERT_PTR(expr) ASSERT_EXPR(expr != nullptr)
#define ASSERT_NULL(expr) ASSERT_EXPR(expr == nullptr)

#define UNUSED_PARAM_ON_RELEASE(a) a
#define UNUSED_PARAM_ON_DEBUG(a)

#else

#define ASSERT_EXPR(expr)
#define ASSERT_PTR(expr)
#define ASSERT_NULL(expr)

#define UNUSED_PARAM_ON_RELEASE(a)
#define UNUSED_PARAM_ON_DEBUG(a) a

#endif

#define UNUSED_PARAM(a)

#define STRINGIFY_EXT(X) #X
#define STRINGIFY(X) STRINGIFY_EXT(X)

#define PREPROCESSOR_COMBINE_STRING_EXT(X, Y) X##Y // helper macro
#define PREPROCESSOR_COMBINE_STRING(X, Y) PREPROCESSOR_COMBINE_STRING_EXT(X, Y)
#define PREPROCESSOR_CONCATENATE_STRING(a, b) a #b

#include <sys/types.h>
#include <sys/stat.h>

#ifdef INCLUDE_OPENCV
// #define HAVE_OPENCV_DNN
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/video/tracking.hpp>
// #include <opencv2/objdetect/objdetect.hpp>
// #include <opencv2/dnn.hpp>
#else
namespace cv
{

	class Mat;
};
#endif

#include <limits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <stack>
#include <list>
#include <queue>
#include <semaphore>
#include <mutex>
#include <chrono>
#include <thread>
#include <algorithm>
#include <memory>
#include <exception>
#include <map>
#include <unordered_map>
#include <atomic>
#include <iomanip>
#include <ctime>
#include <assert.h>
#include <cmath>
#include <functional>
#include <filesystem>
#include <typeinfo>
#include <stdarg.h>
#ifdef WINDOWS_PLATFORM
#define _USE_MATH_DEFINES
#include <math.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define __STDC_WANT_LIB_EXT1__ 1

#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif

std::string GetCurrentThreadID();
void SetCurrentThreadName(const std::string &name);

#if defined(ANDROID_GNU_STL)

namespace std
{

#if !defined(_GLIBCXX_USE_C99_MATH_TR1)

	float round(float arg);
	double round(double arg);
	long double round(long double arg);

#endif

	string to_string(int value);
	string to_string(long value);
	string to_string(long long value);
	string to_string(unsigned value);
	string to_string(unsigned long value);
	string to_string(unsigned long long value);
	string to_string(float value);
	string to_string(double value);
	string to_string(long double value);
}
#endif

#ifndef MAX_PATH

#define MAX_PATH 260

#endif // !MAX_PATH

#ifdef ANDROID_PLATFORM

#define LD(label, ...) __android_log_print(ANDROID_LOG_DEBUG, label, __VA_ARGS__)

#else

#define LD(label, ...) printf(__VA_ARGS__)

#endif

#define LOG_D(format, ...) LD(__PRETTY_FUNCTION__, format, __VA_ARGS__)

void msg_box(void *parent_window, const char *title, const char *message);
void set_thread_affinity(const std::vector<unsigned int> &cores);

#endif

#include "file_system.hpp"

#ifdef WINDOWS_PLATFORM

std::filesystem::path get_executable_directory() {

	char exePath[MAX_PATH];
	// Get the full path of the current executable
	if (GetModuleFileNameA(NULL, exePath, MAX_PATH) != 0) {
		// Use C++17 filesystem library to get the parent directory
		std::filesystem::path path_obj(exePath);
		return path_obj.parent_path();
	}
	perror("Failed to get executable path.");
	return std::filesystem::path();
}

#else

#include <unistd.h>
#include <limits.h>
std::filesystem::path get_executable_directory() {
	char exePath[PATH_MAX];
	// Get the full path of the current executable
	ssize_t count = readlink("/proc/self/exe", exePath, PATH_MAX);
	if (count != -1) {
		exePath[count] = '\0'; // Null-terminate the string
		// Use C++17 filesystem library to get the parent directory
		std::filesystem::path path_obj(exePath);
		return path_obj.parent_path();
	}
	perror("Failed to get executable path.");
	return std::filesystem::path();
}

#endif
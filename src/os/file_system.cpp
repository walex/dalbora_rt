#include "file_system.hpp"

#ifdef WINDOWS_PLATFORM

std::filesystem::path file_system_get_current_executable_directory() {

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
std::filesystem::path file_system_get_current_executable_directory() {
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

std::vector<char> file_system_file_to_bin(const char* const path)
{
	std::ifstream file(path, std::ios::binary);
	if (file.is_open() == false) {
		throw std::runtime_error("Failed to open file: " + std::string(path));
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> data(size);
	file.read(data.data(), size);
	return data;
}

std::filesystem::path file_system_path_join(const std::filesystem::path& p1, const std::filesystem::path& p2) {
	return p1 / p2;
}
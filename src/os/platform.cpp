#include "platform.hpp"
#include <sstream>

#ifdef WINDOWS_PLATFORM
#else
#include <unistd.h>
#include <sys/wait.h>
    #ifdef __ANDROID__
    #include <sys/prctl.h>
    #endif
#endif

#ifdef WINDOWS_PLATFORM

bool run_program(const std::string& programa_path, const std::vector<std::string>& args) {

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Construir la línea de comando completa: "programa arg1 arg2"
    std::string comandoCompleto = programa_path;
    for (const auto& arg : args) {
        comandoCompleto += " " + arg;
    }

    // Windows requiere un buffer de caracteres modificable (char*)
    std::vector<char> cmdBuffer(comandoCompleto.begin(), comandoCompleto.end());
    cmdBuffer.push_back('\0');

    // Crear el proceso
    if (!CreateProcessA(
        NULL,               // Aplicación (NULL usa la línea de comandos)
        cmdBuffer.data(),   // Línea de comando
        NULL,               // Atributos del proceso
        NULL,               // Atributos del hilo
        FALSE,              // Heredar handles
        0,                  // Flags de creación
        NULL,               // Entorno
        NULL,               // Directorio actual
        &si,                // Información de inicio
        &pi                 // Información del proceso
    )) {
        return false;
    }

    // Esperar a que el programa termine
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Cerrar handles para evitar fugas de memoria
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

std::string get_executable_folder(const std::string& concat_path) {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::filesystem::path exe_path(buffer);
	exe_path = exe_path.parent_path() / concat_path;
    return exe_path.string();
}

void msg_box(void* parant_window, const char* title, const char* message) {

    MessageBoxA((HWND)parant_window, message, title, MB_OK);
}

void set_thread_affinity(const std::vector<unsigned int>& cores) {

    HANDLE thread = GetCurrentThread();

    // En Windows, la máscara es un bitmask
    // (ojo: DWORD_PTR suele ser de 64 bits → máx 64 CPUs por grupo)
    DWORD_PTR mask = 0;
    for (int c : cores) {
        mask |= (1ull << c);
    }

    SetThreadAffinityMask(thread, mask);
}

#else

bool run_program(const std::string& programa_path, const std::vector<std::string>& args) {

    // --- IMPLEMENTACIÓN PARA LINUX / MACOS (POSIX) ---
    pid_t pid = fork();

    if (pid == -1) {
        return false; // Error al hacer el fork
    }
    else if (pid == 0) {
        // Proceso Hijo: Preparar argumentos para execvp
        // execvp requiere un array de char* que termine en NULL
        std::vector<char*> argsChar;
        argsChar.push_back(const_cast<char*>(programa.c_str()));

        for (const auto& arg : argumentos) {
            argsChar.push_back(const_cast<char*>(arg.c_str()));
        }
        argsChar.push_back(nullptr);

        // Reemplaza el proceso hijo con el programa objetivo
        execvp(argsChar[0], argsChar.data());

        // Si execvp llega aquí, significa que hubo un error (no se encontró el programa)
        _exit(127);
    }
    else {
        // Proceso Padre: Esperar a que el hijo termine
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
}

void set_thread_affinity(const std::vector<unsigned int>& cores) {

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    for (int c : cores) {
        CPU_SET(c, &cpuset);
    }

    pthread_setaffinity_np(pthread_self(),
        sizeof(cpu_set_t),
        &cpuset);
}

#endif


std::string GetCurrentThreadID() {

    auto myid = std::this_thread::get_id();
    std::stringstream ss;
    ss << myid;
    return ss.str();
}

void SetCurrentThreadName(const std::string& name) {

#ifndef WINDOWS_PLATFORM
    // 16 byte limit for the thread name, including null terminator.
    prctl(PR_SET_NAME, (unsigned long)name.substr(0, 15).c_str(), 0, 0, 0);
#else

    // SetThreadDescription takes a wide character string (PCWSTR)
    std::wstring wname(std::begin(name), std::end(name));
    HRESULT hr = SetThreadDescription(GetCurrentThread(), wname.c_str());
    if (FAILED(hr)) {
        std::wcerr << L"Failed to set thread name: " << hr << std::endl;
    }
#endif
}

#if defined(ANDROID_PLATFORM)

#if defined(ANDROID_GNU_STL)

namespace std {

#if !defined(_GLIBCXX_USE_C99_MATH_TR1)

    float round(float arg) { return ::round(arg); }
    double round(double arg) { return ::round(arg); }
    long double round(long double arg) { return ::round(arg); }

#endif

    template<typename t>
    string to_string_t(t value) {

        std::ostringstream ss;
        ss.setf(std::ios::fixed);
        ss.precision(6);
        ss << value;
        return ss.str();
    }

    string to_string(int value) { return to_string_t(value); }
    string to_string(long value) { return to_string_t(value); }
    string to_string(long long value) { return to_string_t(value); }
    string to_string(unsigned value) { return to_string_t(value); }
    string to_string(unsigned long value) { return to_string_t(value); }
    string to_string(unsigned long long value) { return to_string_t(value); }
    string to_string(float value) { return to_string_t(value); }
    string to_string(double value) { return to_string_t(value); }
    string to_string(long double value) { return to_string_t(value); }
}
#endif

#endif

#if defined(_MSC_VER)

// to avoid warning LNK4221
void dummy() {}

#endif


void print_fps(const char* label, const double inteval_secs) {
    using clock = std::chrono::high_resolution_clock;
    static std::mutex s_mutex;
    static clock::time_point s_start = clock::now();
    static size_t s_frames = 0;

    std::lock_guard<std::mutex> lock(s_mutex);
    ++s_frames;
    auto now = clock::now();
    std::chrono::duration<double> elapsed = now - s_start;
    if (elapsed.count() >= inteval_secs) {
        double fps = static_cast<double>(s_frames) / elapsed.count();
        char buf[128];
        if (label && label[0] != '\0') {
            std::snprintf(buf, sizeof(buf), "%s: %.2f fps\n", label, fps);
        }
        else {
            std::snprintf(buf, sizeof(buf), "FPS: %.2f\n", fps);
        }
#ifndef WINDOWS_PLATFORM
        OutputDebugStringA(buf);
#endif
        std::printf("%s", buf);
        s_frames = 0;
        s_start = now;
    }
}


std::string load_string_from_file(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("could not read the file: ");
    }

    return std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
}

void save_binary_to_file(const char* const path, const uint8_t* const content, size_t size) {
	std::ofstream file(path, std::ios::out | std::ios::binary);
	if (!file) {
		throw std::runtime_error("could not write to the file: ");
	}
	file.write(reinterpret_cast<const char*>(content), size);
}
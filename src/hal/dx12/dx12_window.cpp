#include "dx12_window.hpp"

#ifdef WINDOWS_PLATFORM

std::unique_ptr<HAL_HANDLE> dx12_create_window(const HAL_WINDOW_DESC& desc) {
	// Create a simple window using the Win32 API
	WNDCLASS wc = {};
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = "MyWindowClass";
	RegisterClass(&wc);
	HWND hwnd = CreateWindowEx(
		0,
		"MyWindowClass",
		desc.title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, (int)desc.width, (int)desc.height,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr
	);
	if (!hwnd) {
		throw std::exception("Failed to create window");
	}
	ShowWindow(hwnd, SW_SHOW);

	return std::make_unique<DX_WINDOW_HANDLE>(hwnd);
}

#else
#error "dx12 window handle implemented only on Windows"
#endif
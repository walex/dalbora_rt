#include "dx12_window.hpp"

#ifdef WINDOWS_PLATFORM

LRESULT CALLBACK dx12_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        // Cerrar ventana
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    // Teclado presionado
    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }

        // Ejemplo: pasar input a tu sistema
        // Input::OnKeyDown((uint32_t)wParam);

        return 0;
    }

    case WM_KEYUP:
    {
        // Input::OnKeyUp((uint32_t)wParam);
        return 0;
    }

    // Mouse
    case WM_MOUSEMOVE:
    {
        //int x = GET_X_LPARAM(lParam);
        //int y = GET_Y_LPARAM(lParam);

        // Input::OnMouseMove(x, y);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        // Input::OnMouseDown(0);
        return 0;
    }

    case WM_LBUTTONUP:
    {
        // Input::OnMouseUp(0);
        return 0;
    }

    // Resize (muy importante para swapchain en DX12)
    case WM_SIZE:
    {
        //UINT width = LOWORD(lParam);
        //UINT height = HIWORD(lParam);

        if (wParam != SIZE_MINIMIZED)
        {
            // Renderer::OnResize(width, height);
        }

        return 0;
    }

    // Evitar flicker
    case WM_ERASEBKGND:
    {
        return 1;
    }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

RHI_WINDOW* dx12_window_create(const RHI_WINDOW_DESC* const desc) {

    ASSERT_NULL(desc);

	// Create a simple window using the Win32 API
	WNDCLASS wc = {};
	wc.lpfnWndProc = dx12_window_proc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = "MyWindowClass";
	RegisterClass(&wc);
	HWND hwnd = CreateWindowEx(
		0,
		"MyWindowClass",
		desc->title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, static_cast<int>(desc->width), static_cast<int>(desc->height),
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr
	);
	if (!hwnd) {
		throw std::exception("Failed to create window");
	}

    return new DX_WINDOW(hwnd, desc->callbacks);
}

static std::atomic<bool> window_running;

void dx12_window_main_loop(const RHI_WINDOW* const window) {

    ASSERT_NULL(window);

    auto wnd_handle = reinterpret_cast<HWND>(static_cast<const RHI_VOID_PTR>(*window));
    ShowWindow(wnd_handle, SW_SHOW);

    MSG msg = {};

    auto& callback = static_cast<RHI_WINDOW_CALLBACKS&>(*window);
    window_running.store(true);
    callback.on_init(*window);
    while (window_running.load() == true)
    {
        // Procesar todos los mensajes pendientes
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                window_running.store(false);
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!window_running)
            break;

        callback.main_loop(*window);
    }
    callback.on_end(*window);
}

#else
#error "dx12 window handle implemented only on Windows"
#endif
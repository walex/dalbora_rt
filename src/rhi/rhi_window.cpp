#include "rhi_window.hpp"

RhiWindow::RhiWindow(RHI_WINDOW* handle)
: RhiImpl<RHI_WINDOW>(handle){}

void RhiWindow::create(const std::string& title, const size_t width, 
	const size_t height, const bool full_screen,
	const RHI_WINDOW_CALLBACKS& callbacks) {
		
	// create window params
	RHI_WINDOW_DESC desc;
	desc.title = const_cast<char*>(title.c_str());
	desc.width = width;
	desc.height = height;
	desc.full_screen = full_screen;
	desc.callbacks = const_cast<RHI_WINDOW_CALLBACKS*>(&callbacks);
	this->set_handle(rhi_create_window(&desc));
	// set params

}

void RhiWindow::mainLoop() { rhi_window_main_loop(static_cast<RHI_WINDOW*>(*this)); }

size_t RhiWindow::get_width() const { 
	return static_cast<RHI_WINDOW*>(*this)->width;
}

size_t RhiWindow::get_height() const { 
	return static_cast<RHI_WINDOW*>(*this)->height;
}
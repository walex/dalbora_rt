#ifndef __rhi_window_hpp__
#define __rhi_window_hpp__

#include "rhi_impl.hpp"

class RhiWindow
	: public ICreateRhiObject<const std::string&, const size_t,
			size_t, const bool,	const RHI_WINDOW_CALLBACKS&>
	, public RhiImpl<RHI_WINDOW> {
public:

	RhiWindow(RHI_WINDOW* handle = nullptr);
	virtual ~RhiWindow() = default;
	RhiWindow(const RhiWindow&) = delete;
	RhiWindow& operator=(const RhiWindow&) = delete;
	RhiWindow(RhiWindow&&) noexcept = default;
	RhiWindow& operator=(RhiWindow&&) noexcept = default;

	void create(const std::string& title, const size_t width,
		const size_t height, const bool full_screen,
		const RHI_WINDOW_CALLBACKS& callbacks);

	void mainLoop();
	size_t get_width() const;
	size_t get_height() const;
};

#endif // __rhi_window_hpp__

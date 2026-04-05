#include "Application.hpp"
#include "logger/Log.hpp"
#include "core/core_common.h"
#include "core/pty.h"
#include "core/tty_man.h"
#include "gl/Utils.hpp"
#include <atomic>

#define LOG_KEY_EV

namespace Thr 
{

IOAppClient Application::_client;

void Application::winErrorCallback(ErrorEvent ev)
{
	const auto& err_data = ev.getErrorParams();
	THR_LOG_ERROR("GLFW error detected: code {}, description \"{}\"", err_data.code, err_data.desc);
}

void Application::winResizeCallback(WindowResizeEvent ev)
{
	const auto& winsize = ev.getWindowSizeParams();
	THR_LOG_DEBUG("Window resize callback: width {}, height {}", winsize.width, winsize.height);
}

void Application::winMoveCallback(WindowMoveEvent ev)
{
	const auto& winpos = ev.getWindowPosParams();
	THR_LOG_DEBUG("Window move callback: x {}, y {}", winpos.x, winpos.y);
}

void Application::winFocusCallback(WindowFocusEvent ev)
{
	const auto& winfocus = ev.isFocused();
	THR_LOG_DEBUG("Window focus callback: focus {}", winfocus ? "gained" : "lost");
}

void Application::winCloseCallback()
{
	THR_LOG_DEBUG("Window callback: window close");
}

void Application::winKeyPressCallback(KeyPressEvent ev)
{
#if defined(LOG_KEY_EV)
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Event press callback: keycode {}, mods {}", state.keycode, state.mods);
#endif

	_client.sendEvent(ev);
}

void Application::winKeyReleaseCallback(KeyReleaseEvent ev)
{
	markUnused(ev);
}

void Application::winKeyRepeatCallback(KeyRepeatEvent ev) 
{
#if defined(LOG_KEY_EV)
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Event repeat callback: keycode {}, mods {}", state.keycode, state.mods);
#endif

	_client.sendEvent(ev);
}

void Application::winKeyTypeCallback(KeyTypeEvent ev)
{
#if defined(LOG_KEY_EV)
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Event type callback: keycode {}, mods {}", state.keycode, state.mods);
#endif

	_client.sendEvent(ev);
}

void Application::winMousePressCallback(MousePressEvent ev)
{ 
	markUnused(ev);
}

void Application::winMouseReleaseCallback(MouseReleaseEvent ev)
{
	markUnused(ev);
}

void Application::winMouseMoveCallback(MouseMoveEvent ev)
{
	markUnused(ev);
}

void Application::winMouseScrollCallback(MouseScrollEvent ev)
{
	markUnused(ev);
}

Application::Application(int argc, char* argv[]) 
	: _cwd(FilePath::getCurrentDirectory()) 
	, _window(std::make_unique<Window>())
	, _monitor_width(-1)
	, _monitor_height(-1)
	, _scrollback(std::make_shared<ScrollbackBuffer>())
	, _grid(nullptr)
	, _render_fmt(
			RenderFormat::DefaultPaddingPixX,
			RenderFormat::DefaultPaddingPixY, 
			RenderFormat::DefaultCellCountY,
			RenderFormat::DefaultCellCountX
		)
	, _regular_font(std::make_shared<Font>())
	, _colored_font(std::make_shared<Font>())
	, _datlas(std::make_shared<DoubleAtlas>(1024, 1024))
	, _io_bridge(std::make_shared<IOBridge>(512, 4096))
{
   init();

   std::stringstream ss;

   for (int i = 0; i < argc; i++) {
	  ss << argv[i] << ' ';
   }

   THR_LOG_DEBUG("Got from cmd: {}", ss.str());
}

void Application::run() 
{
	THR_LOG_INFO("Welcome to Therminal!");

	while (_window->isOpen() && _shell.running()) {
		_text_render.clearScreen(Color4f{ 0.1f, 0.1f, 0.1f, 1.f });

		BytesBuf buf { nullptr, 0 };
		_client.readBytes(buf);

		if (buf.n > 0) {
			_parser.parseToGrid(buf.ptr, buf.n);

			const RenderFramePacket packet = {
				_grid->getLineView()
			};

			_text_render.submitCurrFrame(packet);
		}
		
		_text_render.renderText();
		_window->update();
	}
}

void Application::init() 
{
	// Initialize GLFW context
	if (!glfwIsInitialized()) { 
		if (glfwInit() == GLFW_FALSE) {
			THR_LOG_FATAL_FRAME_INFO("Failed to initialize GLFW context!");
			return;
		}
	}

	_regular_font->init(FilePath("Therminal/assets/fonts/DejaVuSansMono.ttf"), _FontBitmapHeightPix);
	//_colored_font->init(FilePath("Therminal/assets/fonts/NotoColorEmoji.ttf"), _FontBitmapHeightPix);

	Font::Metrics font_metrics;
	const bool success = _regular_font->getGlyphMetrics(font_metrics);

	if (!success) {
		THR_LOG_FATAL("Failed to get glyph metrics");
		return;
	}

	const float font_scale = static_cast<float>(_CellHeightPix) / _FontBitmapHeightPix;
	_regular_font->markScale(font_scale);

	const glm::i32vec2 font_size_pix{font_scale * font_metrics.max_advance_pix, 
								     font_scale * font_metrics.height_pix};

	_render_fmt.setCellSize(font_size_pix);

	// Note, that we need to include lower margin of a font for the last row, 
	// so we don't see any font cutting on lower edge of the screen
	const int y_margin = font_size_pix.y - font_scale * font_metrics.ascender_pix;

	/* Setup window size and initialize glfw window */
	const uint window_width  = font_size_pix.x * _render_fmt.getCellCount().x;
	const uint window_height = font_size_pix.y * _render_fmt.getCellCount().y + y_margin;

	_window->init(window_width, window_height, "Hello GLFW!");

	/* Setup event callbacks */
	_window->setErrorCallback(winErrorCallback);
	_window->setWindowResizeCallback(winResizeCallback);
	_window->setWindowMoveCallback(winMoveCallback);
	_window->setWindowFocusCallback(winFocusCallback);
	_window->setWindowCloseCallback(winCloseCallback);
	_window->setKeyPressCallback(winKeyPressCallback);
	_window->setKeyReleaseCallback(winKeyReleaseCallback);
	_window->setKeyRepeatCallback(winKeyRepeatCallback);
	_window->setKeyTypeCallback(winKeyTypeCallback);
	_window->setMousePressCallback(winMousePressCallback);
	_window->setMouseReleaseCallback(winMouseReleaseCallback);
	_window->setMouseMoveCallback(winMouseMoveCallback);
	_window->setMouseScrollCallback(winMouseScrollCallback);

	/* Setup grid and scrollback buffer */
	_scrollback->specifyRenderFormat(_render_fmt);
	_grid = std::make_shared<Grid>(_render_fmt);
	_grid->specifyScrollbackBuffer(_scrollback);

	/* Specify input parser from shell proc */
	_parser.writeTo(_grid);

	/* Setup render format and finally create renderer buffers */
	_render_fmt.setWindowSize(glm::ivec2(_window->getWidth(), _window->getHeight()));

	_text_render.init(_render_fmt, _datlas, _regular_font, _colored_font);

	/* Get true text render format. */
	_text_render.getRenderFormat(_render_fmt);

	/* Create shell stream workflow */
	_client.bindBridge(_io_bridge);
	_shell.init(_io_bridge, _render_fmt);

	_shell.createFork();
}

void Application::getPrimaryMonitorRes(int& width, int& height)
{
	width = -1;
	height = -1;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	THR_HARD_ASSERT_LOG(monitor != nullptr, "Failed to fetch primary monitor");

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	THR_HARD_ASSERT_LOG(mode != nullptr, "Failed to fetch mode of the primary monitor");

	width = mode->width;
	height = mode->height;
}

} // namespace Thr

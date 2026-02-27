#include "Application.hpp"
#include "logger/Log.hpp"

namespace Thr 
{

static constexpr size_t InputRingSize = 512;

Application::_IO Application::_io {
	InputEvTransl  (),
	InputRingBuffer(InputRingSize),
	OutputBuffer   (),
};

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
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Key press callback: key {}, scancode {}, mods {}", state.keycode, state.scancode, state.mods);

	const std::string data = _io.input_ev_transl.translate(state.keycode, state.mods);
	
#if defined(THR_DEBUG)
	std::stringstream ss;
	ss << "Key data stream: ";

	for (char c : data) 
		ss << static_cast<int>(c) << ' ';

	THR_LOG_DEBUG(ss.str());
#endif

	for (char c : data)
		_io.input_circ_buff.put(static_cast<in_char_t>(c));
}

void Application::winKeyReleaseCallback(KeyReleaseEvent ev)
{
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Key release callback: key {}, scancode {}, mods {}", state.keycode, state.scancode, state.mods);
}

void Application::winKeyRepeatCallback(KeyRepeatEvent ev) 
{
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Key repeat callback: key {}, scancode {}, mods {}", state.keycode, state.scancode, state.mods);

	const std::string data = _io.input_ev_transl.translate(state.keycode, state.mods);

#if defined(THR_DEBUG)
	std::stringstream ss;
	ss << "Key data stream: ";

	for (char c : data) 
		ss << static_cast<int>(c) << ' ';

	THR_LOG_DEBUG(ss.str());
#endif

	for (char c : data)
		_io.input_circ_buff.put(static_cast<in_char_t>(c));
}

void Application::winKeyTypeCallback(KeyTypeEvent ev)
{
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Key type callback: code {}", state.keycode);
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

	while (_window->isOpen()) {

		if (!_window->isSuspended()){
			;
		}

		_window->update();
	}
}

void Application::init() 
{
	getPrimaryMonitorSize(_monitor_width, _monitor_height);
	THR_HARD_ASSERT(_monitor_width != -1 && _monitor_height != -1);

	static constexpr float MonitorWidthDiv  = 2.6f;
	static constexpr float MonitorHeightDiv = 2.35f;

	const long window_width  = std::lroundf(_monitor_width / MonitorWidthDiv);
	const long window_height = std::lroundf(_monitor_height / MonitorHeightDiv);

   _window->init(static_cast<uint>(window_width), 
					  static_cast<uint>(window_height), 
					  "Hello GLFW!");

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
}

void Application::getPrimaryMonitorSize(int& width, int& height)
{
	width = -1;
	height = -1;

	if (!glfwIsInitialized()) {
		if (glfwInit() == GLFW_FALSE) {
         THR_LOG_FATAL_FRAME_INFO("Failed to initialize GLFW context!");
         return;
		}
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	THR_HARD_ASSERT_LOG(monitor != nullptr, "Failed to fetch primary monitor");

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	THR_HARD_ASSERT_LOG(mode != nullptr, "Failed to fetch mode of the primary monitor");

   width = mode->width;
   height = mode->height;
}

} // namespace Thr

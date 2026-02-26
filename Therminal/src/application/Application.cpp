#include "Application.hpp"
#include "logger/Log.hpp"

namespace Thr 
{

void winErrorCallback(ErrorEvent ev)
{
	const auto& err_data = ev.getErrorParams();
	THR_LOG_ERROR("GLFW error detected: code {}, description \"{}\"", err_data.code, err_data.desc);
}

void winResizeCallback(WindowResizeEvent ev)
{
	const auto& winsize = ev.getWindowSizeParams();
	THR_LOG_DEBUG("Window resize callback: width {}, height {}", winsize.width, winsize.height);
}

void winMoveCallback(WindowMoveEvent ev)
{
	const auto& winpos = ev.getWindowPosParams();
	THR_LOG_DEBUG("Window move callback: x {}, y {}", winpos.x, winpos.y);
}

void winFocusCallback(WindowFocusEvent ev)
{
	const auto& winfocus = ev.isFocused();
	THR_LOG_DEBUG("Window focus callback: focus {}", winfocus ? "gained" : "lost");
}

void winCloseCallback()
{
	THR_LOG_DEBUG("Window callback: window close");
}

void winKeyPressCallback(KeyPressEvent ev)
{
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Key press callback: key {}", state.keycode);
}

void winKeyReleaseCallback(KeyReleaseEvent ev)
{
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Key release callback: key {}", state.keycode);
}

void winKeyRepeatCallback(KeyRepeatEvent ev) 
{
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Key repeat callback: key {}", state.keycode);
}

void winKeyTypeCallback(KeyTypeEvent ev)
{
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Key type callback: code {}", static_cast<unsigned int>(state.keycode));
}

void winMousePressCallback(MousePressEvent ev)
{ 
	const auto& mice = ev.getMouseButtonParams();
	THR_LOG_DEBUG("Mouse press callback: button {}, mods {}", mice.button, mice.mods);
}

void winMouseReleaseCallback(MouseReleaseEvent ev)
{
	const auto& mice = ev.getMouseButtonParams();
	THR_LOG_DEBUG("Mouse release callback: button {}, mods {}", mice.button, mice.mods);
}

void winMouseMoveCallback(MouseMoveEvent ev)
{
	const auto& cursor = ev.getMouseMoveParams();
	THR_LOG_DEBUG("Mouse move callback: x {}, y {}", cursor.x, cursor.y);
}

void winMouseScrollCallback(MouseScrollEvent ev)
{
	const auto& scroll = ev.getScrollParams();
	THR_LOG_DEBUG("Mouse scoll callback: xoff {}, yoff {}", scroll.xoff, scroll.yoff);
}

Application::Application(int argc, char* argv[]) 
   : _cwd(FilePath::getCurrentDirectory()) 
   , _window(std::make_unique<Window>())
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
   Log::init();

   _window->init(800, 600, "Hello GLFW!");

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

} // namespace Thr

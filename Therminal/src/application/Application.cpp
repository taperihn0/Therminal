#include "Application.hpp"
#include "logger/Log.hpp"
#include "core/core_common.h"
#include "core/pty.h"
#include "core/tty_man.h"

namespace Thr 
{

static constexpr size_t InputRingSize = 512;

static struct termios orig_termios;
static struct winsize size;

Application::_IO Application::_io = { 
	InputEvTransl(),
	InputRingBuffer(InputRingSize),
	OutputBuffer(),
	ThreadWorker()
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
	//const auto& state = ev.getKeyParams();
	//THR_LOG_DEBUG("Event press callback: keycode {}, mods {}", state.keycode, state.mods);

	const std::string data = _io.input_ev_transl.translate(ev);

	if (!ev.isHandled())
		return;

	for (char c : data) {
		_io.input_circ_buff.put(static_cast<in_char_t>(c));
	}
}

void Application::winKeyReleaseCallback(KeyReleaseEvent ev)
{
	markUnused(ev);
}

void Application::winKeyRepeatCallback(KeyRepeatEvent ev) 
{
	//const auto& state = ev.getKeyParams();
	//THR_LOG_DEBUG("Event repeat callback: keycode {}, mods {}", state.keycode, state.mods);

	const std::string data = _io.input_ev_transl.translate(ev);

	if (!ev.isHandled())
		return;

	for (char c : data) {
		_io.input_circ_buff.put(static_cast<in_char_t>(c));
	}
}

void Application::winKeyTypeCallback(KeyTypeEvent ev)
{
	//const auto& state = ev.getKeyParams();
	//THR_LOG_DEBUG("Event type callback: keycode {}, mods {}", state.keycode, state.mods);

	const std::string data = _io.input_ev_transl.translate(ev);

	if (!ev.isHandled())
		return;

	for (char c : data) {
		_io.input_circ_buff.put(static_cast<in_char_t>(c));
	}
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
	, _interactive(false)
	, _fdm(-1)
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
		
		int n = 0;
		const byte* ptr = _io.output_buff.read(n);
		_io.output_buff.swap();

		if (n > 0) {
			writen(STDOUT_FILENO, ptr, n);
			fflush(stdout);
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

	createShellFork();
}

void Application::createShellFork()
{
	pid_t pid;
	char slave_name[20];

	_interactive = isatty(STDIN_FILENO);

	if (_interactive) { /* fetch current termios and window size */
		if (save_termios(STDIN_FILENO, &orig_termios) < 0)
			THR_LOG_ERROR("Failed to save origin termios attributes");

		if (save_winsize(STDIN_FILENO, &size) < 0)
			THR_LOG_ERROR("Failed to save origin winsize");

		pid = pty_fork(&_fdm, slave_name, sizeof(slave_name),
						&orig_termios, &size);
	} 
	else {
		pid = pty_fork(&_fdm, slave_name, sizeof(slave_name),
						nullptr, nullptr);
	}

	if (pid < 0) {
		THR_HARD_ASSERT_LOG(false, "fork error");
	} 
	else if (pid == 0) { /* child */ 
		char shell[] = "zsh";

		if (execlp(shell, "-zsh", "-l", (char*)0) < 0) {
			THR_LOG_FATAL_FRAME_INFO("Can't execute: %s", shell);
			THR_HARD_ASSERT(false);
		}
	}

	THR_LOG_DEBUG("Slave name = {}", slave_name);
	THR_LOG_DEBUG("Interactive = {}", _interactive);

	_io.worker.init(std::addressof(_io.input_circ_buff), 
					std::addressof(_io.output_buff), 
					_fdm);

	_io.worker.spawn();
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

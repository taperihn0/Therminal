#include "Application.hpp"
#include "logger/Log.hpp"
#include "core/core_common.h"
#include "core/pty.h"
#include "core/tty_man.h"
#include "gl/Utils.hpp"

//#define LOG_KEY_EV

namespace Thr 
{

static constexpr size_t InputRingSize = 512;

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
#if defined(LOG_KEY_EV)
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Event press callback: keycode {}, mods {}", state.keycode, state.mods);
#endif

	const std::string data = _io.input_ev_transl.translate(ev);

	if (!ev.isHandled())
		return;

	for (char c : data) {
		_io.input_circ_buff.put(c);
	}
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

	const std::string data = _io.input_ev_transl.translate(ev);

	if (!ev.isHandled())
		return;

	for (char c : data) {
		_io.input_circ_buff.put(c);
	}
}

void Application::winKeyTypeCallback(KeyTypeEvent ev)
{
#if defined(LOG_KEY_EV)
	const auto& state = ev.getKeyParams();
	THR_LOG_DEBUG("Event type callback: keycode {}, mods {}", state.keycode, state.mods);
#endif

	const std::string data = _io.input_ev_transl.translate(ev);

	if (!ev.isHandled())
		return;

	for (char c : data) {
		_io.input_circ_buff.put(c);
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

struct SignalSharedData
{
	std::atomic<pid_t> shell_id;
	std::atomic<bool>  running;
};

static SignalSharedData SharedData = { 0, true };

void onSigChild(THR_UNUSED int sig)
{
	int status;
	pid_t pid;

	while ((pid = waitpid(-1, std::addressof(status), WNOHANG)) > 0) {
		if (pid == SharedData.shell_id) {
			SharedData.running.store(false, std::memory_order_relaxed);
		}
	}
}

void onSigTerm(THR_UNUSED int sig) 
{
	SharedData.running.store(false, std::memory_order_relaxed);
}

Application::Application(int argc, char* argv[]) 
	: _cwd(FilePath::getCurrentDirectory()) 
	, _window(std::make_unique<Window>())
	, _monitor_width(-1)
	, _monitor_height(-1)
	, _fdm(-1)
	, _grid(std::make_shared<Grid>())
	, _render_fmt(
			0,
			0,
			0, 
			24,
			1,
			1
		)
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

	while (_window->isOpen() && 
		   SharedData.running.load(std::memory_order_relaxed)) {
		_text_render.clearScreen(Color4f{ 0.1f, 0.1f, 0.1f, 1.f });

		int n = 0;
		const byte* ptr = _io.output_buff.read(n);
		_io.output_buff.swap();

		if (n > 0) {
			_parser.parseToGrid(ptr, n);

			const RenderFramePacket packet = {
				_grid->getVisibleLines()
			};

			_text_render.submitCurrFrame(packet);
		}
		
		_text_render.renderText();

		_window->update();
	}

	onFinish();
}

void Application::init() 
{
	/* Setup window size and initialize glfw window */
	getPrimaryMonitorRes(_monitor_width, _monitor_height);
	THR_HARD_ASSERT(_monitor_width != -1 && _monitor_height != -1);

	static constexpr float MonitorWidthFrac  = 0.33f;
	static constexpr float MonitorHeightFrac = 0.5f;

	const long window_width = std::lroundf(_monitor_width * MonitorWidthFrac);
	const long window_height = std::lroundf(_monitor_height * MonitorHeightFrac);

	_window->init(static_cast<uint>(window_width), 
				  static_cast<uint>(window_height), 
				  "Hello GLFW!");

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

	/* Specify input parser from shell proc */
	_parser.writeTo(_grid);

	/* Setup render format and initialize text rendering */
	_render_fmt.setWindowSize(glm::ivec2(_window->getWidth(),
										 _window->getHeight()));
										
	_text_render.init(_render_fmt);

	/* Get true text render format. */
	_text_render.getRenderFormat(_render_fmt);
	
	_grid->specifyRenderFormat(_render_fmt);

	/* Create shell stream workflow */
	createShellFork();
}

void Application::createShellFork()
{
#if defined(THR_PLATFORM_WINDOWS)

// WINDOWS IMPLEMENTATION HERE

#else

	pid_t pid;
	char slave_name[20];

	/* Block incoming signals for now.
	*  Make sure it got handled by the parent properly after the fork.
	*/

	sigset_t sigset, prev_sigset;

	if (sigemptyset(std::addressof(sigset)) < 0)
		THR_LOG_ERROR("Failed to 'sigemptyset'");

	if (sigaddset(std::addressof(sigset), SIGCHLD) < 0 ||
		sigaddset(std::addressof(sigset), SIGUSR1) < 0)
		THR_LOG_ERROR("Failed to 'sigaddset'");

	if (sigprocmask(SIG_BLOCK, 
					std::addressof(sigset), 
					std::addressof(prev_sigset)) < 0) 
		THR_LOG_ERROR("Failed to 'sigprocmask'");

	/* setup terminal file for slave */

	struct termios slave_termios;
	struct winsize slave_winsize;

	if (isatty(STDIN_FILENO)) { /* use native termios */
		if (save_termios(STDIN_FILENO, std::addressof(slave_termios)) < 0)
			THR_LOG_ERROR("Failed to save origin termios attributes");
	}
	else if (interactive_termios(std::addressof(slave_termios)) < 0) {
		THR_LOG_ERROR("Failed to generate interactive termios");
	}

	slave_winsize.ws_col = _render_fmt.getCellCountHorizontal();
	slave_winsize.ws_row = _render_fmt.getCellCountVertical();

	pid = pty_fork(std::addressof(_fdm), slave_name, sizeof(slave_name),
				   std::addressof(slave_termios), std::addressof(slave_winsize));

	if (pid < 0) {
		THR_HARD_ASSERT_LOG(false, "fork error");
	} 
	else if (pid == 0) { /* child */ 
		if (sigprocmask(SIG_SETMASK, std::addressof(prev_sigset), nullptr) < 0)
			THR_LOG_ERROR("Failed to 'sigprocmask'"); // might not display properly

		char shell[] = "sh";

		if (execlp(shell, "-sh", "-l", nullptr) < 0) {
			THR_LOG_FATAL_FRAME_INFO("Can't execute: {}", shell);
			
			if (kill(getppid(), SIGTERM) < 0) {
				exit(-1);
			}

			exit(0);
		}
		
		/* Never returns */
	}

	/* parent */

	SharedData.shell_id = pid;

	Signal(SIGCHLD).handle(onSigChild);
	Signal(SIGUSR1).handle(onSigTerm);

	/* Unblock pending signals */

	if (sigprocmask(SIG_SETMASK, std::addressof(prev_sigset), nullptr) < 0)
		THR_LOG_ERROR("Failed to 'sigprocmask'");
	
	THR_LOG_DEBUG("Slave name = {}", slave_name);

	_io.worker.init(std::addressof(_io.input_circ_buff), 
					std::addressof(_io.output_buff), 
					_fdm);

	_io.worker.spawn();

#endif // THR_PLATFORM_WINDOWS
}

void Application::getPrimaryMonitorRes(int& width, int& height)
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

void Application::onFinish()
{
	if (SharedData.shell_id > 0 && kill(SharedData.shell_id, SIGHUP) < 0) {
		THR_LOG_ERROR("Failed to send SIGHUP to fork: {}", SharedData.shell_id);
	}

	while (wait(nullptr) > 0);
}

} // namespace Thr

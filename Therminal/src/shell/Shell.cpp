#include "Shell.hpp"
#include "core/core_common.h"
#include "core/pty.h"
#include "core/tty_man.h"
#include "Signal.hpp"
#include <atomic>

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

namespace Thr
{

Shell::Shell()
    : _io_bridge(nullptr)
    , _fdm(-1)
    , _render_fmt(
		0,
		0,
		0, 
		24,
		1,
		1
	    )
    , _initialized(false)
{}

Shell::~Shell()
{
    if (SharedData.shell_id > 0 && kill(SharedData.shell_id, SIGHUP) < 0) {
		THR_LOG_ERROR("Failed to send SIGHUP to fork: {}", SharedData.shell_id);
	}

	while (wait(nullptr) > 0);
}

void Shell::init(std::shared_ptr<IOBridge>& bridge, 
			     const RenderFormat& fmt)
{
    if (!bridge) {
        THR_LOG_FATAL("Invalid bridge pointer");
        return;
    }

    _io_bridge = bridge;
    _render_fmt = fmt;

    _initialized = true;
}

void Shell::createFork()
{
    if (!_initialized) {
        THR_LOG_ERROR("Failed to create fork on unitialized shell");
        return;
    }

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

    _client.bindBridge(_io_bridge);
    
	_io_worker.init(std::addressof(_client), 
					_fdm);

	_io_worker.spawn();

#endif // THR_PLATFORM_WINDOWS
}

bool Shell::running()
{
    return SharedData.running;
}

} // namespace Thr

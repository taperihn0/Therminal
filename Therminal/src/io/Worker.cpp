#include "Worker.hpp"
#include "core/core_common.h"

namespace Thr
{
   
ThreadWorker::ThreadWorker()
	: _shell_client(nullptr)
	, _running(false)
	, _ptymfd(-1)
{}

ThreadWorker::~ThreadWorker()
{
	stop();
}

void ThreadWorker::init(Ptr<IOShellClient> shell_client,
						int ptym)
{
	if (!shell_client) {
		THR_LOG_FATAL("Invalid shell client pointer");
		return;
	}

	_shell_client = shell_client;
	_ptymfd = ptym;   
}

void ThreadWorker::spawn()
{
	_running = true;
	_thr = std::thread(
		[this]() {
			this->thrExecution();
		});
}

void ThreadWorker::stop()
{
	if (_thr.joinable()) {
		_running = false;
		_thr.join();
	}
}

void ThreadWorker::thrExecution()
{

#if defined(THR_PLATFORM_WINDOWS)

// WINDOWS IMPLEMENTATION HERE

#else

	int nread;

	const size_t output_buf_size = _shell_client->getOutputBuf()
												  .getSize();
	const size_t input_buf_size = _shell_client->getInputBuf()
												 .getCapacity();

	std::unique_ptr<byte[]> output_buf = std::make_unique<byte[]>(output_buf_size);
	std::unique_ptr<byte[]> input_buf = std::make_unique<byte[]>(input_buf_size);

	struct pollfd pfd;
	pfd.fd = _ptymfd;
	pfd.events = POLLIN;

	static constexpr int Timeout = 1;

	while (_running) {
		if (poll(&pfd, 1, Timeout) < 0)
			break;

		if (pfd.revents & (POLLIN | POLLHUP | POLLERR)) { /* copies ptym to output */
			if ((nread = read(pfd.fd, output_buf.get(), sizeof(output_buf))) <= 0)
				break;
			
			const BytesBuf bytes = { output_buf.get(), nread };
			_shell_client->writeBytes(bytes);
		}

		MutBytesBuf read_buf = { input_buf.get(), static_cast<int>(input_buf_size) };

		if (_shell_client->readBytes(read_buf)) {
			if (writen(pfd.fd, read_buf.ptr, read_buf.n) != read_buf.n)
				THR_LOG_ERROR("writen error to master pty");
		}
	}

	/*
	*  We should terminate.
	*/

#endif // THR_PLATFORM_WINDOWS
}

} // namespace Thr

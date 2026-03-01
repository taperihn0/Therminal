#include "Worker.hpp"
#include "core/core_common.h"

namespace Thr
{
   
ThreadWorker::ThreadWorker()
	: _is_buff(nullptr)
	, _os_buff(nullptr)
	, _running(false)
	, _ptymfd(-1)
{}

ThreadWorker::~ThreadWorker()
{
	stop();
}

void ThreadWorker::init(Ptr<InputRingBuffer> is, 
						Ptr<OutputBuffer> os,
						int ptym)
{
	_is_buff = is;
	_os_buff = os;
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
	int nread;

	byte buff[OutputBuffer::BuffSize];

	struct pollfd pfd;
	pfd.fd = _ptymfd;
	pfd.events = POLLIN;

	static constexpr int Timeout = 1;

	while (_running) {
		if (poll(&pfd, 1, Timeout) < 0)
			break;

		if (pfd.revents & (POLLIN | POLLHUP | POLLERR)) { /* copies ptym to output */
			if ((nread = read(pfd.fd, buff, sizeof(buff))) <= 0)
				break;
			
			_os_buff->write(buff, nread);
		}
 
		if (_is_buff->isReady()) { /* copies input to ptym */
			const in_char_t ch = _is_buff->get();
			
			// TODO: handle EOF

			if (writen(pfd.fd, &ch, sizeof(ch)) != sizeof(ch))
				THR_LOG_ERROR("writen error to master pty");
		}
	}

	/*
	*  We should terminate.
	*/
}

} // namespace Thr

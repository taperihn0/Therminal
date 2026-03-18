#pragma once

#include "Common.hpp"
#include "IOBridge.hpp"
#include <thread>

namespace Thr
{
	
class ThreadWorker
{
public:
	ThreadWorker();
	~ThreadWorker();

	void init(Ptr<IOShellClient> shell_client,
			  int ptym);
	
	void spawn();
	void stop();
private:
	void thrExecution();

	std::thread        _thr;
	Ptr<IOShellClient> _shell_client;
	bool               _running;
	int                _ptymfd;
};

} // namespace Thr


#pragma once

#include "Common.hpp"
#include "io/IOBridge.hpp"
#include "gl/RenderFormat.hpp"
#include "io/Worker.hpp"

namespace Thr
{
 
/* Manage shell fork via Shell class
*/
class Shell
{
public:
	Shell();
	~Shell();

	void init(std::shared_ptr<IOBridge>& bridge, 
			  const RenderFormat& fmt);

	void createFork();

	bool running();
private:
	std::shared_ptr<IOBridge> _io_bridge;
	IOShellClient             _client;
	int 					  _fdm;
	RenderFormat 			  _render_fmt;
	bool 					  _initialized;
	ThreadWorker			  _io_worker;
};

} // namespace Thr

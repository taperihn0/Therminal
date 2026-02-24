#include "Application.hpp"
#include "logger/Log.hpp"

namespace Thr 
{

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

   _window->init(800, 600, "Hello GLFW!");
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
}

} // namespace Thr

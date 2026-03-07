#include "application/Application.hpp"
#include "memory/Memory.hpp"

int main(int argc, char* argv[]) 
{
	std::unique_ptr<Thr::Application> app = std::make_unique<Thr::Application>(argc, argv);
	app->run();
}

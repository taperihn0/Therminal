#pragma once

#include "Common.hpp"
#include "window/Window.hpp"
#include "filesys/Filepath.hpp"
#include "memory/Memory.hpp"

namespace Thr 
{

class Application 
{
public:
   Application() = default;
   Application(int argc, char* argv[]);
   ~Application() = default;

   void run();
private:
   void init();
   
   FilePath                _cwd;
   std::unique_ptr<Window> _window;
};

} // namespace Thr

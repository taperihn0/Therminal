#include "InputQueue.hpp"
#include "logger/Log.hpp"
#include "window/Window.hpp"

namespace Thr 
{

WinInputQueue::WinInputQueue()
{
   const Window* globwin;

   if ((globwin = Window::getGlobWindow()) == nullptr) {
      THR_LOG_ERROR("No window created for the window input queue");
      return;
   }

   if (!glfwIsInitialized()) {
      THR_LOG_ERROR("GLFW is not initialized");
      return;
   }
}

bool WinInputQueue::waitEvents()
{
   const Window* globwin;

   if ((globwin = Window::getGlobWindow()) == nullptr) {
      THR_LOG_ERROR("No window created for the window input queue");
      return false;
   }
   
   if (!glfwIsInitialized()) {
      THR_LOG_ERROR("GLFW is not initialized");
      return false;
   }

   glfwWaitEvents();
   return true;
}

bool WinInputQueue::pollEvents() 
{
   const Window* globwin;

   if ((globwin = Window::getGlobWindow()) == nullptr) {
      THR_LOG_ERROR("No window created for the window input queue");
      return false;
   }

   if (!glfwIsInitialized()) {
      THR_LOG_ERROR("GLFW is not initialized");
      return false;
   }

   glfwPollEvents();
   return true;
}

} // namespace Thr

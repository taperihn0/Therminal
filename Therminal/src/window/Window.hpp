#pragma once

#include "Common.hpp"

namespace Thr 
{

THR_DECLARE class WinInputQueue;

/* Cross-platform window handler class based on GLFW library.
*  Only one instance of valid and initialized window is accepted.
*/
class Window
{
public:
   Window();
   Window(uint width, uint height, 
          const std::string& title = std::string(_DefaultWindowTitle));
   ~Window();

   static Window* getGlobWindow();

   bool init(uint width, uint height, const std::string& title);
   /* That function might sleep when we got empty event queue.
   *  We use glfwWaitEvent() here.
   */
   void update();
   bool isOpen() const;
   bool isSuspended() const;
private:
   static constexpr std::string_view  _DefaultWindowTitle  = "Title";

   int                                _width;
   int                                _height;
   bool                               _focus;
   bool                               _suspended;
   std::string                        _title;
   bool                               _initialized;
   bool                               _close;
   GLFWwindow*                        _native_window;
   WinInputQueue*                     _input;

   //std::unique_ptr<PlatformSwapchain> _swapchain;
   //__CallbackData                     _callback_ptrs;

   /* We're operating on one window only.
   *  We check if we've bounded another window on new Window initialization.
   */
   static Window*                     _glob_bind;
};

} // namespace Thr

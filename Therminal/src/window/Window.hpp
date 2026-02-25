#pragma once

#include "Common.hpp"
#include "ev/Event.hpp"
#include <functional>
#include <any>

namespace Thr 
{

/* We handle event via callbacks internally in Window class.
*/

using ErrorCallback         = std::function<void(ErrorEvent)>;
using WindowResizeCallback  = std::function<void(WindowResizeEvent)>;
using WindowMoveCallback    = std::function<void(WindowMoveEvent)>;
using WindowFocusCallback   = std::function<void(WindowFocusEvent)>;
using WindowCloseCallback   = std::function<void(WindowCloseEvent)>;
using KeyPressCallback      = std::function<void(KeyPressEvent)>;
using KeyReleaseCallback    = std::function<void(KeyReleaseEvent)>;
using KeyRepeateCallback    = std::function<void(KeyRepeatEvent)>;
using KeyTypeCallback       = std::function<void(KeyTypeEvent)>;
using MousePressCallback    = std::function<void(MousePressEvent)>;
using MouseReleaseCallback  = std::function<void(MouseReleaseEvent)>;
using MouseMoveCallback     = std::function<void(MouseMoveEvent)>;
using MouseScrollCallback   = std::function<void(MouseScrollEvent)>;

THR_INTERNAL void __defaultEventCallback(THR_UNUSED std::any) {}

struct EventCallbacks
{
   ErrorCallback         error_callback         = __defaultEventCallback;

   WindowResizeCallback  window_resize_callback = __defaultEventCallback;
   WindowMoveCallback    window_move_callback   = __defaultEventCallback;
   WindowFocusCallback   window_focus_callback  = __defaultEventCallback;
   WindowCloseCallback   window_close_callback  = __defaultEventCallback;

   KeyPressCallback      key_press_callback     = __defaultEventCallback;
   KeyReleaseCallback    key_release_callback   = __defaultEventCallback;
   KeyRepeateCallback    key_repeat_callback    = __defaultEventCallback;
   KeyTypeCallback       key_type_callback      = __defaultEventCallback;

   MousePressCallback    mouse_press_callback   = __defaultEventCallback;
   MouseReleaseCallback  mouse_release_callback = __defaultEventCallback;
   MouseMoveCallback     mouse_move_callback    = __defaultEventCallback;
   MouseScrollCallback   mouse_scroll_callback  = __defaultEventCallback;
};

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

   /* Internal data for callbacks.
   *  window pointer should be set to this.
   */
   struct __CallbackData
   {
      Window* const   window;
      EventCallbacks* callbacks;
   };
   
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

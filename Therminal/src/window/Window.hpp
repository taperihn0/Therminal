#pragma once

#include "Common.hpp"
#include "ev/Event.hpp"
#include "memory/Memory.hpp"
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
using WindowCloseCallback   = std::function<void()>;
using KeyPressCallback      = std::function<void(KeyPressEvent)>;
using KeyReleaseCallback    = std::function<void(KeyReleaseEvent)>;
using KeyRepeatCallback     = std::function<void(KeyRepeatEvent)>;
using KeyTypeCallback       = std::function<void(KeyTypeEvent)>;
using MousePressCallback    = std::function<void(MousePressEvent)>;
using MouseReleaseCallback  = std::function<void(MouseReleaseEvent)>;
using MouseMoveCallback     = std::function<void(MouseMoveEvent)>;
using MouseScrollCallback   = std::function<void(MouseScrollEvent)>;

THR_INTERNAL void __defaultDoNothing(THR_UNUSED std::any) {}
THR_INTERNAL void __defaultDoNothingZ() {}

struct _EventCallbacks
{
   inline static ErrorCallback  error_callback  = __defaultDoNothing;

   WindowResizeCallback  window_resize_callback = __defaultDoNothing;
   WindowMoveCallback    window_move_callback   = __defaultDoNothing;
   WindowFocusCallback   window_focus_callback  = __defaultDoNothing;
   WindowCloseCallback   window_close_callback  = __defaultDoNothingZ;

   KeyPressCallback      key_press_callback     = __defaultDoNothing;
   KeyReleaseCallback    key_release_callback   = __defaultDoNothing;
   KeyRepeatCallback     key_repeat_callback    = __defaultDoNothing;
   KeyTypeCallback       key_type_callback      = __defaultDoNothing;

   MousePressCallback    mouse_press_callback   = __defaultDoNothing;
   MouseReleaseCallback  mouse_release_callback = __defaultDoNothing;
   MouseMoveCallback     mouse_move_callback    = __defaultDoNothing;
   MouseScrollCallback   mouse_scroll_callback  = __defaultDoNothing;
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
   
   /* __setWidth / __setHeight methods do not really *resize* the window, but
   *  rather change internal state fields to match the actual size.
   *  Used internally by callbacks function on resize event.
   */
   void __setWidth(uint width);
   void __setHeight(uint height);
   /* Internal usage method.
   *  For more explanations, see Phs::Window::__setWidth and Phs::Window::__setHeight.
   */
   void __setFocus(bool value);
   /* Internal usage method.
   *  For more explanations on internal usage, see Phs::Window::__setWidth and Phs::Window::__setHeight.
   */
   void __setSuspended(bool value);
   /* Internal usage method.
   *  For more explanations on internal usage, see Phs::Window::__setWidth and Phs::Window::__setHeight.
   */
   void __setClose(bool value);

   void setErrorCallback(ErrorCallback callbck);

   void setWindowResizeCallback(WindowResizeCallback callbck);
   void setWindowMoveCallback(WindowMoveCallback callbck);
   void setWindowFocusCallback(WindowFocusCallback callbck);
   void setWindowCloseCallback(WindowCloseCallback callbck);
   
   void setKeyPressCallback(KeyPressCallback callbck);
   void setKeyReleaseCallback(KeyReleaseCallback callbck);
   void setKeyRepeatCallback(KeyRepeatCallback callbck);
   void setKeyTypeCallback(KeyTypeCallback callbck);
   
   void setMousePressCallback(MousePressCallback callbck);
   void setMouseReleaseCallback(MouseReleaseCallback callbck);
   void setMouseMoveCallback(MouseMoveCallback callbck);
   void setMouseScrollCallback(MouseScrollCallback callbck);

private:
   void setEventCallbacks();

   /* Internal data for callbacks.
   */
   struct _CallbackData
   {
      _CallbackData(Window* win);

      Window* const                    window;
      std::shared_ptr<_EventCallbacks> callbacks;
   };

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
   std::unique_ptr<_CallbackData>     _user_callback_data;

   //std::unique_ptr<PlatformSwapchain> _swapchain;

   /* We're operating on one window only.
   *  We check if we've bounded another window on new Window initialization.
   */
   static Window*                     _glob_bind;
};

} // namespace Thr

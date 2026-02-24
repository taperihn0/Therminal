#pragma once

#include "Common.hpp"
#include "memory/Memory.hpp"
#include "window/Window.hpp"

namespace Thr 
{

/* Input system managing the current state of the input devices for 
*  single window. It's a part of the window implementation.
*/
class WinInputQueue 
{
public:
   WinInputQueue();

   /* We have to poll collected events as they occur on the GLFW side.
   *  We wait for the next event.
   *  From the https://www.glfw.org/docs/3.3/input_guide.html docs:
   *  "It puts the thread to sleep until at least one event has been received and then processes all received events. 
   *  This saves a great deal of CPU cycles and is useful for, for example, editing tools."
   *  Return value: true on success, false on failure.
   */
   bool waitEvents();

   /* Poll received events only and return.
   *  Return value: true on success, false on failure.
   */
   bool pollEvents();
};
   
} // namespace Thr 

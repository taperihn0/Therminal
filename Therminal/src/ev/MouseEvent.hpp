#pragma once

#include "BaseEvent.hpp"

namespace Thr
{
   
////////////// Base 'WindowEvent' event //////////////////

class MouseEvent : public Event 
{
public:
   THR_INLINE MouseEvent();
   THR_INLINE ~MouseEvent() = default;
protected:
   THR_INLINE MouseEvent(EventCode evcode);
};

THR_INLINE MouseEvent::MouseEvent() 
   : Event(EV_NONE, EV_CATEGORY_MOUSE)
{}

THR_INLINE MouseEvent::MouseEvent(EventCode evcode)
   : Event(evcode, EV_CATEGORY_MOUSE)
{}

////////////// Derived window-categories events //////////////////

template <EventCode MouseButtonEvCode>
class MouseButtonEvent : public MouseEvent
{
public:
   THR_STATIC_ASSERT(MouseButtonEvCode == EV_MOUSE_BUTTON_PRESSED ||
                     MouseButtonEvCode == EV_MOUSE_BUTTON_RELEASED);

   THR_INLINE MouseButtonEvent(int button, int mods);
   THR_INLINE ~MouseButtonEvent() = default;

   struct MouseButtonParams;
   
   THR_INLINE MouseButtonParams getMouseButton() const;
   
   struct MouseButtonParams
   {
      int button;
      int mods;
   };

private:
   const MouseButtonParams _params;
};

template <EventCode MouseButtonEvCode>
THR_INLINE MouseButtonEvent<MouseButtonEvCode>::MouseButtonEvent(int button, int mods)
   : _params { button, mods }
{}

template <EventCode MouseButtonEvCode>
THR_INLINE typename MouseButtonEvent<MouseButtonEvCode>::MouseButtonParams 
MouseButtonEvent<MouseButtonEvCode>::getMouseButton() const 
{
   return _params;
}

using MousePressEvent   = MouseButtonEvent<EV_MOUSE_BUTTON_PRESSED>;
using MouseReleaseEvent = MouseButtonEvent<EV_MOUSE_BUTTON_RELEASED>;

class MouseMoveEvent : public MouseEvent 
{
public:
   struct MouseMoveParams 
   {
      double xpos;
      double ypos;
   };

   THR_INLINE MouseMoveEvent(double x, double y);
   THR_INLINE ~MouseMoveEvent() = default;

   THR_INLINE MouseMoveParams getMouseMoveParams() const { return _params; }

private:
   const MouseMoveParams _params;
};

THR_INLINE MouseMoveEvent::MouseMoveEvent(double x, double y)
   : MouseEvent(EV_MOUSE_MOVED)
   , _params{ x, y }
{}

class MouseScrollEvent : public MouseEvent 
{
public:
   THR_INLINE MouseScrollEvent(double xoff, double yoff);
   THR_INLINE ~MouseScrollEvent() = default;
   
   struct MouseScrollParams;

   THR_INLINE MouseScrollParams getScrollParams() const;

   struct MouseScrollParams 
   {
      double xoffset;
      double yoffset;
   };

private:
   const MouseScrollParams _params;
};

THR_INLINE MouseScrollEvent::MouseScrollEvent(double xoff, double yoff)
   : MouseEvent(EV_MOUSE_SCROLLED)
   , _params{ xoff, yoff }
{}

THR_INLINE MouseScrollEvent::MouseScrollParams 
MouseScrollEvent::getScrollParams() const 
{ 
   return _params; 
}

} // namespace Thr

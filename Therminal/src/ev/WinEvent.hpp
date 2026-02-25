#pragma once

#include "BaseEvent.hpp"

namespace Thr
{

////////////// Base 'WindowEvent' event //////////////////

class WindowEvent : public Event 
{
public:
   THR_INLINE WindowEvent();
   THR_INLINE ~WindowEvent() = default;
protected:
   THR_INLINE WindowEvent(EventCode evcode);
};

THR_INLINE WindowEvent::WindowEvent() 
   : Event(EV_NONE, EV_CATEGORY_WINDOW)
{}

THR_INLINE WindowEvent::WindowEvent(EventCode evcode)
   : Event(evcode, EV_CATEGORY_WINDOW)
{}

////////////// Derived window-categories events //////////////////

class WindowResizeEvent : public WindowEvent
{
public:
   THR_INLINE WindowResizeEvent(int nwidth, int nheight);
   THR_INLINE ~WindowResizeEvent() = default;

   struct WindowSizeParams;
   
   THR_INLINE WindowSizeParams getWindowSizeParams() const;
   
   struct WindowSizeParams
   {
      int winheight;
      int winwidth;
   };

private:
   const WindowSizeParams _params;
};

WindowResizeEvent::WindowResizeEvent(int nwidth, int nheight)
   : WindowEvent(EV_WINDOW_RESIZE)
   , _params { nheight, nwidth }
{}

THR_INLINE WindowResizeEvent::WindowSizeParams 
WindowResizeEvent::getWindowSizeParams() const 
{
   return _params;
}

class WindowMoveEvent : public WindowEvent
{
public:
   THR_INLINE WindowMoveEvent(int x, int y);
   THR_INLINE ~WindowMoveEvent() = default;

   struct WindowPosParams;
   
   THR_INLINE WindowPosParams getWindowPosParams() const;

   struct WindowPosParams 
   {
      int winx;
      int winy;
   };

private:
   const WindowPosParams _params;
};

THR_INLINE WindowMoveEvent::WindowMoveEvent(int x, int y)
   : WindowEvent(EV_WINDOW_MOVE)
   , _params{ x, y }
{}

THR_INLINE WindowMoveEvent::WindowPosParams 
WindowMoveEvent::getWindowPosParams() const 
{
   return _params;
}

class WindowFocusEvent : public WindowEvent
{
public:
   THR_INLINE WindowFocusEvent(bool focused);
   THR_INLINE ~WindowFocusEvent() = default;

   THR_INLINE bool isFocused() const;
private:
   const bool _focused;
};

THR_INLINE WindowFocusEvent::WindowFocusEvent(bool focused)
   : WindowEvent(EV_WINDOW_FOCUS)
   , _focused(focused)
{}

THR_INLINE bool WindowFocusEvent::isFocused() const { 
   return _focused; 
}

class WindowCloseEvent : public WindowEvent
{
public:
   THR_INLINE WindowCloseEvent();
   THR_INLINE ~WindowCloseEvent() = default;
};

THR_INLINE WindowCloseEvent::WindowCloseEvent()
   : WindowEvent(EV_WINDOW_CLOSE)
{}

} // namespace Thr

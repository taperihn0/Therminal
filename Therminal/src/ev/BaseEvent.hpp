#pragma once

#include "Common.hpp"

namespace Thr
{

/* Numeric codes for Event class template.
*  Each group is numered that each of them contain different shared bit.
*/
enum EventCode : uint16_t
{
   // Invalid event
   EV_NONE                  = 0x0,
   
   // Error event
   EV_ERROR                 = 0xF,

   // EV_WINDOW_MIN_VALUE is equivalent EV_WINDOW_RESIZE
   EV_WINDOW_MIN_VALUE      = 0x1,
   EV_WINDOW_RESIZE         =       EV_WINDOW_MIN_VALUE,
   EV_WINDOW_MOVE           = 0x2 | EV_WINDOW_MIN_VALUE,
   EV_WINDOW_FOCUS          = 0x4 | EV_WINDOW_MIN_VALUE,
   EV_WINDOW_CLOSE          = 0x8 | EV_WINDOW_MIN_VALUE, 

   // EV_KEY_MIN_VALUE is equivalent to EV_KEY_PRESSED
   EV_KEY_MIN_VALUE         = 0x10,
   EV_KEY_PRESSED           =        EV_KEY_MIN_VALUE,
   EV_KEY_RELEASED          = 0x20 | EV_KEY_MIN_VALUE,
   EV_KEY_REPEATED          = 0x40 | EV_KEY_MIN_VALUE,
   EV_KEY_TYPED             = 0x80 | EV_KEY_MIN_VALUE,

   // EV_MOUSE_MIN_VALUE is equivalent to EV_MOUSE_BUTTON_PRESSED
   EV_MOUSE_MIN_VALUE       = 0x100,
   EV_MOUSE_BUTTON_PRESSED  =         EV_MOUSE_MIN_VALUE,
   EV_MOUSE_BUTTON_RELEASED = 0x200 | EV_MOUSE_MIN_VALUE,
   EV_MOUSE_MOVED           = 0x400 | EV_MOUSE_MIN_VALUE,
   EV_MOUSE_SCROLLED        = 0x800 | EV_MOUSE_MIN_VALUE,
};

/* Event categories raw numeric values.
*  Corresponds to minimal value of each of the groups representing
*  different event (action) codes.
*/
enum EventCategory : uint16_t 
{
   EV_CATEGORY_NONE   = EV_NONE,
   EV_CATEGORY_ERROR  = EV_ERROR,
   EV_CATEGORY_WINDOW = EV_WINDOW_MIN_VALUE, 
   EV_CATEGORY_KEY    = EV_KEY_MIN_VALUE,    
   EV_CATEGORY_MOUSE  = EV_MOUSE_MIN_VALUE,  
};

THR_FORCEINLINE constexpr EventCategory evCatFromEvCode(EventCode evcode) {
   for (EventCategory evcat : { EV_CATEGORY_NONE, 
                                EV_CATEGORY_ERROR, 
                                EV_CATEGORY_WINDOW, 
                                EV_CATEGORY_KEY, 
                                EV_CATEGORY_MOUSE }) {
      if (evcode & evcat) 
         return evcat;
   }

   return EV_CATEGORY_NONE;
}

////////////// Base 'Event' class //////////////////

class Event 
{
public:
   Event() = delete;

   THR_INLINE void handleEvent();
   THR_INLINE void unhandleEvent();
   THR_INLINE bool isHandled();

   THR_INLINE EventCategory getEvCategory();
   THR_INLINE EventCode     getEvCode();
protected:
   THR_INLINE Event(EventCode evcode, EventCategory evcat);

   bool                _handle = false;
   const EventCode     _evcode = EV_NONE;
   const EventCategory _evcat  = EV_CATEGORY_NONE;
};

THR_INLINE void Event::handleEvent() 
{
   _handle = true;
}

THR_INLINE void Event::unhandleEvent() 
{
   _handle = false;
}

THR_INLINE bool Event::isHandled() 
{
   return _handle;
}

THR_INLINE EventCategory Event::getEvCategory() 
{
   return _evcat;
}

THR_INLINE EventCode     Event::getEvCode() 
{
   return _evcode;
}

THR_INLINE Event::Event(EventCode evcode, EventCategory evcat)
   : _evcode(evcode)
   , _evcat(evcat)
{}

////////////// Error Event //////////////////

class ErrorEvent : public Event 
{
public:
   THR_INLINE ErrorEvent(int code, const char* desc);
   THR_INLINE ~ErrorEvent() = default;

   struct ErrorParams;

   THR_INLINE ErrorParams getErrorParams() const;

   struct ErrorParams
   {
      int         code;
      const char* desc;
   };
   
private:
   ErrorParams _params;
};

THR_INLINE ErrorEvent::ErrorEvent(int code, const char* desc)
   : Event(EV_ERROR, EV_CATEGORY_ERROR)
   , _params { code, desc }
{}

THR_INLINE ErrorEvent::ErrorParams
ErrorEvent::getErrorParams() const 
{
   return _params;
}

} // namespace Thr

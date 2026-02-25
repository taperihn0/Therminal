#pragma once

#include "BaseEvent.hpp"

namespace Thr
{
   
////////////// Base 'KeyEvent' event //////////////////

class KeyEvent : public Event 
{
public:
   THR_INLINE KeyEvent();
   THR_INLINE ~KeyEvent() = default;
protected:
   THR_INLINE KeyEvent(EventCode evcode);
};

THR_INLINE KeyEvent::KeyEvent() 
   : Event(EV_NONE, EV_CATEGORY_KEY)
{}

THR_INLINE KeyEvent::KeyEvent(EventCode evcode)
   : Event(evcode, EV_CATEGORY_KEY)
{}

////////////// Derived keyboard-categories events //////////////////

template <EventCode KeyEvCode>
class KeyButtonEvent : public KeyEvent
{
public:
   THR_STATIC_ASSERT(KeyEvCode == EV_KEY_PRESSED  ||
                     KeyEvCode == EV_KEY_RELEASED ||
                     KeyEvCode == EV_KEY_REPEATED ||
                     KeyEvCode == EV_KEY_TYPED);

   THR_INLINE KeyButtonEvent(int keycode);
   THR_INLINE ~KeyButtonEvent() = default;

   struct KeyButtonParams 
   {
      char16_t keycode;
   };

   THR_INLINE KeyButtonParams getKeyParams() const;

private:
   const KeyButtonParams _params;
};

template <EventCode KeyEvCode>
THR_INLINE KeyButtonEvent<KeyEvCode>::KeyButtonEvent(int keycode)
   : KeyEvent(KeyEvCode)
   , _params { keycode }
{}

template <EventCode KeyEvCode>
THR_INLINE typename KeyButtonEvent<KeyEvCode>::KeyButtonParams
KeyButtonEvent<KeyEvCode>::getKeyParams() const 
{ 
   return _params; 
}

using KeyPressEvent   = KeyButtonEvent<EV_KEY_PRESSED>;
using KeyReleaseEvent = KeyButtonEvent<EV_KEY_RELEASED>;
using KeyRepeatEvent  = KeyButtonEvent<EV_KEY_REPEATED>;
using KeyTypeEvent    = KeyButtonEvent<EV_KEY_TYPED>;

} // namespace Thr

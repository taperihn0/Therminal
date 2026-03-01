#include "InputTranslator.hpp"
#include "logger/Log.hpp"
#include "Keymap.hpp"

namespace Thr
{
	
InputEvTransl::InputEvTransl()
{
	createMapping();
}

THR_FORCEINLINE bool isAlpha(int keycode) 
{
	return keycode >= THR_KEY_A and keycode <= THR_KEY_Z;
}

THR_FORCEINLINE int toLower(int keycode) 
{
	return tolower(keycode);
}

/* Translate key-pressed or key-repeated event 'ev' to
*  stream of bytes. That translation
*  is needed for events with mod keys,
*  ie. SHIFT+a or GLFW_MOD_CAPS_LOCK+g
*/
template <EventCode C>
std::string InputEvTransl::translate(KeyButtonEvent<C>& ev)
{
	if constexpr (C == EV_KEY_RELEASED) {
		ev.unhandleEvent();
		return "";
	}

	else if constexpr (C == EV_KEY_REPEATED ||
					   C == EV_KEY_PRESSED) {

		const auto& state = ev.getKeyParams();

		/* We'll handle that as KeyTypedEvent */
		if (isAlpha(state.keycode) && 
			!state.mods) {
			ev.unhandleEvent();
			return "";
		}

		else if (isAlpha(state.keycode) &&
				 (state.mods & ~THR_MOD_SHIFT) == 0) {
			ev.unhandleEvent();
			return "";
		}

		ev.handleEvent();

		EvKey key { state.keycode, state.mods };

		/* First, we check for control codes, since
		*  they are easy to encode.
		*/
		if (state.mods & THR_MOD_CONTROL) {
			if (isAlpha(state.keycode)) {
				const char ch = state.keycode - THR_KEY_A + 1;
				return std::string(1, ch);
			}
		}

		const auto it = _map.find(key);

		if (it != _map.end()) {
			return it->second;
		}

		return "";
	}

	else if constexpr (C == EV_KEY_TYPED) {
		ev.handleEvent();
		const auto& state = ev.getKeyParams();
		return std::string(1, static_cast<char>(state.keycode));
	}

	else THR_STATIC_ASSERT_LOG(false, "Unknown keybutton event");
}

/* Create mapping for easy key and mod lookup
*  in order to generate char sequence code.
*/
void InputEvTransl::createMapping()
{
	THR_LOG_ERROR("Creating ANSI Escape codes mapping");

	// Tab
	_map[{ THR_KEY_TAB, THR_MOD_SHIFT 		}] = "\t";
	_map[{ THR_KEY_TAB, 0			  		}] = "\t";
	// Backspace
	_map[{ THR_KEY_BACKSPACE, THR_MOD_SHIFT }] = "\b";
	_map[{ THR_KEY_BACKSPACE, 0			    }] = "\b";
	// Enter
	_map[{ THR_KEY_ENTER, THR_MOD_SHIFT 	}] = "\r";
	_map[{ THR_KEY_ENTER, THR_MOD_CONTROL 	}] = "\r";
	_map[{ THR_KEY_ENTER, THR_MOD_ALT 	    }] = "\n";
	_map[{ THR_KEY_ENTER, 0			    	}] = "\r";
}

} // namespace Thr

template std::string Thr::InputEvTransl::translate<Thr::EV_KEY_PRESSED>(Thr::KeyPressEvent&);
template std::string Thr::InputEvTransl::translate<Thr::EV_KEY_TYPED>  (Thr::KeyTypeEvent&);
template std::string Thr::InputEvTransl::translate<Thr::EV_KEY_RELEASED>(Thr::KeyReleaseEvent&);
template std::string Thr::InputEvTransl::translate<Thr::EV_KEY_REPEATED>(Thr::KeyRepeatEvent&);

#include "InputTranslator.hpp"
#include "logger/Log.hpp"
#include "Keymap.hpp"

namespace Thr
{
   
InputEvTransl::InputEvTransl()
{
   createMapping();
}

/* Translate key-pressed or key-repeated event 'ev' to
*  stream of bytes. That translation
*  is needed for events with mod keys,
*  ie. SHIFT+a or GLFW_MOD_CAPS_LOCK+g
*/
std::string InputEvTransl::translate(int keycode, int mods)
{
   /* Sometimes we got just raw type event.
   */
   if (!mods) {
      return std::string(1, keycode);
   }

   EvKey key { keycode, mods };

   /* First, we check for control codes, since
   *  they are easy to encode.
   */
   if (mods & THR_MOD_CONTROL) {
      if (keycode >= THR_KEY_A && keycode <= THR_KEY_Z) {
         const char ctrl_code = static_cast<char>(keycode - THR_KEY_A + 1);
         return std::string(1, ctrl_code);
      }
   }

   const auto it = _map.find(key);

   if (it != _map.end()) {
      return it->second;
   }

   return "";
}

/* Create mapping for easy key and mod lookup
*  in order to generate char sequence code.
*/
void InputEvTransl::createMapping()
{
   THR_LOG_ERROR("No event mapping yet");
}

} // namespace Thr

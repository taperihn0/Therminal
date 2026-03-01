#pragma once

#include "ev/Event.hpp"
#include "memory/Memory.hpp"
#include <unordered_map>

namespace Thr
{
	
struct EvKey
{
	EvKey() = default;

	THR_INLINE constexpr bool operator==(const EvKey& k) const;
	THR_INLINE constexpr bool operator!=(const EvKey& k) const;

	int key;
	int mods;
};

THR_INLINE constexpr bool EvKey::operator==(const EvKey& k) const
{
	return key == k.key && mods == k.mods;
}

THR_INLINE constexpr bool EvKey::operator!=(const EvKey& k) const
{
	return !(*this == k);
}

} // namespace Thr

template <>
struct std::hash<Thr::EvKey> 
{
	size_t operator()(const Thr::EvKey& k) const 
	{
		const uint64_t m = (static_cast<uint64_t>(k.key) << 32) | k.mods;
		return hash<uint64_t>()(m);
	}
};

namespace Thr
{

/* Keyboard Events -> raw bytes translator.
*/
class InputEvTransl 
{
public:
	InputEvTransl();

	/* Translate key-pressed or key-repeated event 'ev' to
	*  stream of bytes. That translation
	*  is needed for events with mod keys,
	*  ie. SHIFT+a or GLFW_MOD_CAPS_LOCK+g
	*/
	template <EventCode C>
	std::string translate(KeyButtonEvent<C>& ev);
private:
	void createMapping();

	std::unordered_map<EvKey, std::string> _map;
};

} // namespace Thr

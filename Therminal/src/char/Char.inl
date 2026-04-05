#include "Char.hpp"
#include <cwchar>

namespace Thr
{

template <typename T>
THR_INLINE Char<T>::Char(T codepoint)
	: codepoint(codepoint)
{
	THR_STATIC_ASSERT((is_same_v<T, char32_t>) && "Unimplemented");
}

template <typename T>
THR_INLINE int Char<T>::getWidth() const
{
	return wcwidth(static_cast<wchar_t>(codepoint));
}

template <typename T>
THR_INLINE bool Char<T>::isPrintable() const
{
	const int width = getWidth();
	return width > 0;
}

template <typename T>
THR_INLINE bool Char<T>::isControl0() const
{
	return codepoint == U'\a' ||
		   codepoint == U'\b' ||
		   codepoint == U'\t' ||
		   codepoint == U'\n' ||
		   codepoint == U'\f' ||
		   codepoint == U'\r' ||
		   codepoint == U'\x1B';
}

template <typename T>
THR_INLINE bool Char<T>::isControl1() const
{
	return codepoint == U'\x8E' ||
		   codepoint == U'\x8F' ||
		   codepoint == U'\x90' ||
		   codepoint == U'\x9B' ||
		   codepoint == U'\x9C' ||
		   codepoint == U'\x9D' ||
		   codepoint == U'\x98' ||
		   codepoint == U'\x9E' ||
		   codepoint == U'\x9F';
}

template <typename T>
THR_INLINE bool Char<T>::isEmoji() const
{
	return (U'\x1F600' >= codepoint && codepoint <= U'\x1F64F') || // Emoticons
		   (U'\x1F300' >= codepoint && codepoint <= U'\x1F5FF') || // Misc Symbols and Pictographs
		   (U'\x1F680' >= codepoint && codepoint <= U'\x1F6FF') || // Transport and Map
		   (U'\x2600'  >= codepoint && codepoint <= U'\x26FF')  || // Misc symbols
		   (U'\x2700'  >= codepoint && codepoint <= U'\x27BF')  || // Dingbats
		   (U'\xFE00'  >= codepoint && codepoint <= U'\xFE0F')  || // Variation Selectors
		   (U'\x1F900' >= codepoint && codepoint <= U'\x1F9FF') || // Supplemental Symbols and Pictographs
		   (U'\x1F1E6' >= codepoint && codepoint <= U'\x1F1FF');   // Flags
}

template <typename T>
THR_INLINE Char<T>& Char<T>::operator=(Char<T> c) 
{
	codepoint = c.codepoint;
	return *this;
}

template <typename T>
THR_INLINE bool Char<T>::operator==(Char<T> c) const 
{
	return codepoint == c.codepoint;
}

template <typename T>
THR_INLINE bool Char<T>::operator!=(Char<T> c) const
{
	return !(*this == c);
}

template <typename T>
THR_INLINE bool Char<T>::operator>(Char<T> c) const
{
	return codepoint > c.codepoint;
}

template <typename T>
THR_INLINE bool Char<T>::operator>=(Char<T> c) const
{
	return codepoint >= c.codepoint;
}

template <typename T>
THR_INLINE bool Char<T>::operator<(Char<T> c) const
{
	return codepoint < c.codepoint;
}

template <typename T>
THR_INLINE bool Char<T>::operator<=(Char<T> c) const
{
	return codepoint <= c.codepoint;
}

template <typename T>
THR_INLINE Char<T>::operator T() const
{
	return codepoint;
}

} // namespace Thr


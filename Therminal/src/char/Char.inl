#include "Char.hpp"
#include <cwchar>

namespace Thr
{

template <typename T>
THR_INLINE Char<T>::Char(T codepoint)
	: codepoint(codepoint)
{}

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
THR_INLINE Char<T>& Char<T>::operator=(Char<T> c) 
{
	codepoint = c.codepoint;
	return *this;
}

template <typename T>
THR_INLINE Char<T> Char<T>::operator==(Char<T> c) const 
{
	return codepoint == c.codepoint;
}

template <typename T>
THR_INLINE Char<T> Char<T>::operator!=(Char<T> c) const
{
	return !(*this == c);
}

template <typename T>
THR_INLINE Char<T> Char<T>::operator>(Char<T> c) const
{
	return codepoint > c.codepoint;
}

template <typename T>
THR_INLINE Char<T> Char<T>::operator>=(Char<T> c) const
{
	return codepoint >= c.codepoint;
}

template <typename T>
THR_INLINE Char<T> Char<T>::operator<(Char<T> c) const
{
	return codepoint < c.codepoint;
}

template <typename T>
THR_INLINE Char<T> Char<T>::operator<=(Char<T> c) const
{
	return codepoint <= c.codepoint;
}

template <typename T>
THR_INLINE Char<T>::operator T() const
{
	return codepoint;
}

} // namespace Thr


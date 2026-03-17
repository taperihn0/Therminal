#pragma once

#include "Common.hpp"

namespace Thr
{

template <typename T>
struct Char
{
	THR_STATIC_ASSERT((is_same_v<T, char8_t> ||
					   is_same_v<T, char16_t> ||
					   is_same_v<T, char32_t>));

	Char() = default;
	THR_INLINE Char(T codepoint);

	THR_INLINE int getWidth() const;
	THR_INLINE bool isPrintable() const;

	THR_INLINE Char<T>& operator=(Char<T> c);
	THR_INLINE Char<T>  operator==(Char<T> c) const;
	THR_INLINE Char<T>  operator!=(Char<T> c) const;
	THR_INLINE Char<T>  operator>(Char<T> c) const;
	THR_INLINE Char<T>  operator>=(Char<T> c) const;
	THR_INLINE Char<T>  operator<(Char<T> c) const;
	THR_INLINE Char<T>  operator<=(Char<T> c) const;

	THR_INLINE operator T() const;

	T codepoint;
};

using Char8 = Char<char8_t>;
using Char16 = Char<char16_t>;
using Char32 = Char<char32_t>;

} // namespace Thr

#include "Char.inl"

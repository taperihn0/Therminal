#pragma once

#include "Common.hpp"

namespace Thr
{
	
struct Locale
{
	void setLocale() 
	{
		std::string_view cloc = std::setlocale(LC_ALL, nullptr);

		if (cloc != loc) {
			std::setlocale(LC_ALL, loc.data());
		}
	}

	std::string_view loc;
};

static inline Locale LocUTF32 = []() -> Locale {
	Locale loc{ "en_US.UTF-8" };
	loc.setLocale();
	return loc;
}();

} // namespace Thr

#pragma once

#include "Common.hpp"
#include <csignal>

namespace Thr
{

class Signal {
public:
	using handler = void (*)(int);
	explicit Signal(int sig);
	int getSignal() const;
	handler handle(handler handl) const;
	int raise() const;
private:
	int _sig;
};

THR_INTERNAL Signal::Signal(int sig)
	: _sig(sig) 
{}

THR_INTERNAL int Signal::getSignal() const
{
	return _sig;
}

THR_INTERNAL Signal::handler Signal::handle(handler handl) const
{
	return std::signal(_sig, handl);
}

THR_INTERNAL int Signal::raise() const
{
	return std::raise(_sig);
}

} // namespace Thr

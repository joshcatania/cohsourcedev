#include "utilitieslib/UtilsCXX/timing2.hpp"


timer::timer()
	: _clock(), _start()
{ }


void timer::start()
{
	this->_start = _clock.now();
}

hrclock::duration timer::reset()
{
	hrclock::time_point now = this->_clock.now();
	hrclock::duration duration = now - this->_start;
	this->_start = now;
	return duration;
}

timer::microrep timer::reset_micro()
{
	hrclock::duration duration = this->reset();
	return std::chrono::duration_cast<timer::micro>(duration).count();
}
#ifndef _TIMING2_HPP
#define _TIMING2_HPP

#include <chrono>

typedef std::chrono::high_resolution_clock hrclock;

/* The following is defined in c++20 */
typedef std::chrono::duration<uint32_t, std::ratio<86400>> days;
typedef std::chrono::duration<uint32_t, std::ratio<604800>> weeks;
typedef std::chrono::duration<uint32_t, std::ratio<2629746>> months;
typedef std::chrono::duration<uint32_t, std::ratio<31556952>> years;
/* */
typedef std::chrono::seconds seconds;

class timer
{
public:
	typedef std::chrono::microseconds micro;
	typedef micro::rep microrep;

private:
	hrclock _clock;
	hrclock::time_point _start;

public:
	timer();

	void start();

	hrclock::duration reset();

	microrep reset_micro();
};

class time_utils
{
public:
	//constexpr static hrclock::time_point y2k()
	//{
	//	constexpr years dur = years(2000 - 1970);
	//	constexpr hrclock::duration hrdur = std::chrono::duration_cast<hrclock::duration>(dur);
	//	return hrclock::time_point(hrdur);
	//}

	template <class _Duration>
	constexpr static _Duration fromSeconds(seconds::rep secondsValue)
	{
		return std::chrono::duration_cast<_Duration>(seconds(secondsValue));
	}
};

#endif /* _TIMING2_HPP */
/*
 * Timer.cpp
 *
 *  Created on: 2008-jul-04
 *      Author: hoglet
 */

#include "Timer.h"
#include <gtkmm.h>

Timer::Timer()
{
	Glib::signal_timeout().connect( sigc::mem_fun(*this, &Timer::on_signal_1_second), 1000 );
}

Timer::~Timer()
{
}


void Timer::attach(TimerObserver* observer)
{
	observers.push_back(observer);
}

void Timer::detach(TimerObserver* observer)
{
	observers.remove(observer);
}

bool Timer::on_signal_1_second()
{
	signalSender();
	return true;
}

//This function is called every second
void Timer::signalSender()
{
	static int TenSecondCounter = 10;
	TenSecondCounter--;
	bool signal10Seconds=false;
	if(TenSecondCounter < 1)
	{
		signal10Seconds=true;
		TenSecondCounter = 10;
	}
	std::list<TimerObserver*>::iterator iter;
	for (iter = observers.begin(); iter != observers.end(); iter++)
	{
		TimerObserver* observer = *iter;
		observer->on_signal_1_second();
		if(signal10Seconds)
		{
			observer->on_signal_10_seconds();
		}
	}
}


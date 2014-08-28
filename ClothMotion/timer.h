// wunf 's timer class
#ifndef __TIMER_H
#define __TIMER_H



struct Timer {
	time_t then;
	double last, total;
	Timer ();
	void tick (), tock ();
};
#endif

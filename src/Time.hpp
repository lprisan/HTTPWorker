/*******************************************************************************
*   Copyright 2013 EPFL                                                        *
*                                                                              *
*   This file is part of metroscope.                                           *
*                                                                              *
*   Metroscope is free software: you can redistribute it and/or modify         *
*   it under the terms of the GNU General Public License as                    *
*   published by the Free Software Foundation, either version 3 of the         *
*   License, or (at your option) any later version.                            *
*                                                                              *
*   Metroscope is distributed in the hope that it will be useful,              *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*   GNU General Public License for more details.                               *
*                                                                              *
*   You should have received a copy of the GNU General Public License          *
*   along with Metroscope.  If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************/

#ifndef Time_HPP
#define Time_HPP

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace Time {

namespace {
class PreciseTimer
{
public:
	//order of initialization doesn't really matter here, does it?
	static PreciseTimer Singleton;
	long GetTime() {
#ifdef WIN32
		return timeGetTime();
#else
		gettimeofday(&mTimeval, 0);
		return mTimeval.tv_sec*1000l + (long) (mTimeval.tv_usec/1000.0f + 0.5f);
#endif
	}
private:
#ifndef WIN32
	struct timeval mTimeval;
#endif
	PreciseTimer() {
#ifdef WIN32
		timeBeginPeriod(1);
#endif
	}
	~PreciseTimer(){
#ifdef WIN32
		timeEndPeriod(1);
#endif
	}
	PreciseTimer(const PreciseTimer&);
	PreciseTimer& operator=(const PreciseTimer&);
};

PreciseTimer PreciseTimer::Singleton;
}

long inline MillisTimestamp() { return PreciseTimer::Singleton.GetTime(); }

void inline Sleep(long pMillis)
{
#ifdef WIN32
		Sleep(pMillis);
#else
		usleep(pMillis*1000l);
#endif
}

}

#endif

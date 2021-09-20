/*
 *  Copyright (C) 2004-2010 Terence M. Welsh
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#ifndef RS_TIMER_H
#define RS_TIMER_H

#include <thread>
#include <chrono>

class rsTimer{
public:
	// Time since last call to tick()
	double elapsed_time;
	// Time waited so far by wait()
	double waited_time;
	// Wait() would take a hair too long, if we didn't keep track of the
	// extra time it was spending.  That's what wait_overhead is for.
	double wait_function_overhead;
	std::chrono::high_resolution_clock::time_point curr_ts;
	std::chrono::high_resolution_clock::time_point prev_ts;


	rsTimer(){
		elapsed_time = 0.0;
		waited_time = 0.0;
		wait_function_overhead = 0.0;
		prev_ts = std::chrono::high_resolution_clock::now();
	}

	~rsTimer(){}

	// return time elapsed since last call to tick()
	inline double tick(){
		curr_ts = std::chrono::high_resolution_clock::now();
		elapsed_time = std::chrono::duration<double>(curr_ts - prev_ts).count();
		prev_ts = curr_ts;
		return elapsed_time;
	}

	// Wait until target_time has elapsed, then return.
	// If you call this function after target_time has already elapsed,
	// it will return immediately.  Think of target_time as a lower limit
	// on frame time.
	// Returns actual time elapsed since last call to wait().
	inline double wait(double target_time){
		if(wait_function_overhead < -target_time)
			wait_function_overhead = -target_time;
		if(wait_function_overhead > target_time)
			wait_function_overhead = target_time;

		waited_time = tick();
		const double actual_waited_time(waited_time + wait_function_overhead);
		if(actual_waited_time < target_time){
			std::this_thread::sleep_for(std::chrono::milliseconds(int(1000.0 * (target_time - actual_waited_time))));
			waited_time += tick();
		}
		wait_function_overhead += waited_time - target_time;
		return waited_time;
	}
};



#endif

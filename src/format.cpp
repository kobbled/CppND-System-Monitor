#include <string>
#include <iostream>
#include <iomanip>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {

    // declare time variables
    int time = seconds;
    int hour = time/3600;
	time = time%3600;
	int min = time/60;
	time = time%60;
	int sec = time;

    //(ref: https://www.includehelp.com/cpp-programs/cpp-class-program-to-read-time-in-hh-mm-ss-and-convert-into-total-seconds-using-class.aspx)  
    std::ostringstream sTime;
    sTime << std::setw(2) << std::setfill('0') << hour << ":"
    << std::setw(2) << std::setfill('0') << min << ":"
    << std::setw(2) << std::setfill('0') << sec;

    return sTime.str(); 
}
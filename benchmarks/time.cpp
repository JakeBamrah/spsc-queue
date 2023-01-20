#include <chrono>
#include <iostream>

#include "time.h"


using std::chrono::milliseconds;


TimePoint getTimePoint()
{
    return Clock::now();
}

int getTimeDelta(TimePoint start)
{
    TimePoint end = getTimePoint();
    milliseconds diff = std::chrono::duration_cast<milliseconds>(end - start);
    return diff.count();
}

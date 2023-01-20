#include <chrono>


using std::chrono::time_point;
using std::chrono::steady_clock;
using std::chrono::milliseconds;

typedef std::chrono::steady_clock Clock;
typedef std::chrono::time_point<Clock> TimePoint;


time_point<steady_clock> getTime();
int getDelta(time_point<steady_clock> start);

#include <Sys.h>

uint64_t Sys::_upTime;

#include <Log.h>
#include <string.h>
#include <unistd.h>

char Sys::_hostname[30] = "";
//_____________________________________________________________ LINUX and CYGWIN
#if defined(__linux__) || defined(__CYGWIN__)
#include <chrono>
#include <time.h>

uint64_t Sys::millis() // time in msec since boot, only increasing
{
    using namespace std::chrono;
    milliseconds ms =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    Sys::_upTime = system_clock::now().time_since_epoch().count() / 1000000;
    /*
struct timespec deadline;
int erc = clock_gettime((int)CLOCK_MONOTONIC, &deadline);
if (erc)
    WARN("clock_gettime() failed.");
Sys::_upTime = deadline.tv_sec * 1000L + deadline.tv_nsec / 1000000L;*/
    return _upTime;
}

void Sys::init() { gethostname(_hostname, sizeof(_hostname) - 1); }

void Sys::hostname(const char* hostname) {
    strncpy(_hostname, hostname, sizeof(_hostname));
}

const char* Sys::hostname() {
    if (_hostname[0] == 0)
        Sys::init();
    return _hostname;
}

void Sys::delay(uint32_t msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec - ts.tv_sec * 1000) * 1000000;
    nanosleep(&ts, NULL);
};

#endif

    //________________________________________________________ ARDUINO

#ifdef ARDUINO

uint64_t Sys::millis() // time in msec since boot, only increasing
{
    return ::millis();
}

void Sys::hostname(const char* hostname) {
    strncpy(_hostname, hostname, sizeof(_hostname));
}

const char* Sys::hostname() { return _hostname; }

#endif

#ifdef __ESP8266__

void Sys::delay(uint32_t delta) {
    uint64_t t1 = Sys::millis() + delta;
    while (Sys::millis() < t1)
        ;
}

#endif

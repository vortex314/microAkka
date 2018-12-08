#include <Sys.h>
#include <time.h>
uint64_t Sys::_upTime;

#include <string.h>
#include <time.h>
#include <unistd.h>

char Sys::_hostname[30] = "";
//_____________________________________________________________ LINUX and CYGWIN
#if defined(__linux__) || defined(__CYGWIN__)

uint64_t Sys::millis() // time in msec since boot, only increasing
{
    struct timespec deadline;
    clock_gettime((int)CLOCK_MONOTONIC, &deadline);
    Sys::_upTime = deadline.tv_sec * 1000L + deadline.tv_nsec / 1000000L;
    return _upTime;
}

void Sys::init() { gethostname(_hostname, 30); }

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

#include <Sys.h>
#include <time.h>
uint64_t Sys::_upTime;

#include <string.h>
#include <time.h>
#include <unistd.h>

char Sys::_hostname[30];
//_____________________________________________________________ LINUX and CYGWIN
#if defined(__linux__) || defined(__CYGWIN__)

uint64_t Sys::millis() // time in msec since boot, only increasing
{
    struct timespec deadline;
    clock_gettime((int)CLOCK_MONOTONIC, &deadline);
    Sys::_upTime = deadline.tv_sec * 1000 + deadline.tv_nsec / 1000000;
    return _upTime;
}

void Sys::init() { gethostname(_hostname, 30); }

void Sys::hostname(const char* hostname) {
    strncpy(_hostname, hostname, sizeof(_hostname));
}

const char* Sys::hostname() { return _hostname; }


void Sys::delay(uint32_t delta){
	uint64_t t1=Sys::millis()+delta;
	while( Sys::millis()<t1);
}

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

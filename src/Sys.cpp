#include <Sys.h>
#include <time.h>
uint64_t Sys::_upTime;

#include <time.h>
#include <unistd.h>
#include <string.h>

char Sys::_hostname[30];

uint64_t Sys::millis()   // time in msec since boot, only increasing
{
	struct timespec deadline;
	clock_gettime((int)CLOCK_MONOTONIC, &deadline);
	Sys::_upTime= deadline.tv_sec*1000 + deadline.tv_nsec/1000000;
	return _upTime;
}


void Sys::init()
{
	gethostname(_hostname,30);
}

void Sys::hostname(const char* hostname)
{
	strncpy(_hostname, hostname,sizeof(_hostname));
}

const char* Sys::hostname()
{
	return _hostname;
}

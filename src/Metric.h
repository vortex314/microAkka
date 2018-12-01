#ifndef METRIC_H
#define METRIC_H
#include <Str.h>
#include <Sys.h>
#include <stdint.h>
namespace Metric {
class Metric {
    const char* _name;
    const char* _unit;
    uint32_t _sampleCount;

  public:
    Metric(const char* name, const char* unit);
    Metric(const char* name);
    void reset();
    uint32_t sampleCount();
    void sample();
    virtual void toStr(Str& str) = 0;
};

class Counter : public Metric {
    uint64_t _start;
    uint64_t _stop;
    uint64_t _counter;

  public:
    Counter(const char* name, const char* unit);
    void inc();
    void inc(uint32_t delta);
    void dec();
    void dec(uint32_t delta);
    void start();
    void stop();
    void delta();
    uint64_t count();
    void toStr(Str& str);
};

class Gauge : public Metric {
    uint32_t _count;
    double _sum, _min, _max;

  public:
    Gauge(const char* name);
    void set(double value);
    double avg();
    double max();
    double min();
    uint32_t count();
    void reset();
};

class Meter : public Metric {
    uint32_t _count;
    const char* _name;
    const char* _unit;
    uint64_t _startTime;

  public:
    Meter(const char* name, const char* unit);
    void tick();
    void reset();
    void toStr(Str& str);
};

class Timer : public Metric {
  public:
    Timer(const char* name, const char* unit);
    void start();
    void reset();
    void stop();
    void toStr(Str& str);
    double avg();
    double min();
    double max();
    uint32_t count();
};

}; // namespace Metric

#endif // METRIC_H

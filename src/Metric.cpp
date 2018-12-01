#include "Metric.h"
#include "limits"
namespace Metric {
Metric::Metric(const char* name, const char* unit) {
    _name = name;
    _unit = unit;
    _sampleCount = 0;
}

Metric::Metric(const char* name) {
    _name = name;
    _unit = "";
    _sampleCount = 0;
}

void Metric::reset() { _sampleCount = 0; }
} // namespace Metric
#pragma once

#include "securezone/domain/Alarm.h"
namespace securezone::alarm { class AlarmLifecycleService { public: void activate(domain::Alarm& alarm) const; void acknowledge(domain::Alarm& alarm, const std::string& acknowledgedBy = {}) const; void resolve(domain::Alarm& alarm) const; }; }

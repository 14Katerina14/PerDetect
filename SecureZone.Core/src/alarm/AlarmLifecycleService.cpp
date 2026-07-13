#include "securezone/alarm/AlarmLifecycleService.h"
namespace securezone::alarm {
void AlarmLifecycleService::activate(domain::Alarm& alarm) const { if (alarm.status == domain::AlarmStatus::Created) { alarm.status = domain::AlarmStatus::Active; alarm.stillInside = true; } }
void AlarmLifecycleService::acknowledge(domain::Alarm& alarm, const std::string& acknowledgedBy) const { if (alarm.status == domain::AlarmStatus::Active) { alarm.status = domain::AlarmStatus::Acknowledged; alarm.acknowledgedBy = acknowledgedBy; } }
void AlarmLifecycleService::resolve(domain::Alarm& alarm) const { if (alarm.status == domain::AlarmStatus::Active || alarm.status == domain::AlarmStatus::Acknowledged) { alarm.status = domain::AlarmStatus::Resolved; alarm.stillInside = false; alarm.exitedAt = std::chrono::system_clock::now(); } }
}

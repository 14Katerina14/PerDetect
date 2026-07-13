#pragma once

#include <chrono>
#include <string>
namespace securezone::domain {
enum class BindingStatus { Bound, Uncertain, Expired };
enum class BindingSource { QrCheckIn, Manual, Unknown };
struct TrackIdentityBinding { std::string trackId, employeeId, checkInId; BindingStatus status{BindingStatus::Uncertain}; double confidence{}; BindingSource source{BindingSource::Unknown}; std::chrono::system_clock::time_point boundAt{}; };
}

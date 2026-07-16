#pragma once

#include "securezone/alarm/AlarmPersistenceService.h"
#include "securezone/decision/DecisionEngine.h"
#include "securezone/repository/IAccessPolicyRepository.h"
#include "securezone/repository/IAlarmRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/xprotect/XProtectLineCrossingService.h"

#include <functional>

namespace securezone::xprotect {

class XProtectPolicyAlarmService {
public:
    using AlarmCreatedNotifier = std::function<void(const domain::Alarm&)>;

    XProtectPolicyAlarmService(
        repository::IEmployeeRepository& employeeRepository,
        repository::IAccessPolicyRepository& accessPolicyRepository,
        repository::IMachineRepository& machineRepository,
        repository::IAlarmRepository& alarmRepository,
        AlarmCreatedNotifier alarmCreatedNotifier = {}
    );

    XProtectLineCrossingDecision evaluate(
        const XProtectLineCrossingCommand& command,
        const domain::Zone& zone,
        const std::optional<domain::TrackIdentityBinding>& binding
    );

private:
    repository::IEmployeeRepository& employeeRepository_;
    repository::IAccessPolicyRepository& accessPolicyRepository_;
    repository::IMachineRepository& machineRepository_;
    repository::IAlarmRepository& alarmRepository_;
    decision::DecisionEngine decisionEngine_;
    alarm::AlarmPersistenceService alarmPersistenceService_;
    AlarmCreatedNotifier alarmCreatedNotifier_;
};

}

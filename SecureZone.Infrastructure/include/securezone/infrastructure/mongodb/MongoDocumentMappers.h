#pragma once

#include <bsoncxx/document/view.hpp>

#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/Alarm.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/QrCheckIn.h"
#include "securezone/domain/WebhookTarget.h"
#include "securezone/domain/Zone.h"

namespace securezone::infrastructure::mongodb {

domain::Employee mapEmployeeDocument(bsoncxx::document::view document);
domain::Zone mapZoneDocument(bsoncxx::document::view document);
domain::MachineState mapMachineDocument(bsoncxx::document::view document);
domain::AccessPolicy mapAccessPolicyDocument(bsoncxx::document::view document);
domain::Alarm mapAlarmDocument(bsoncxx::document::view document);
domain::QrCheckin mapQrCheckinDocument(bsoncxx::document::view document);
domain::PresenceSession mapPresenceSessionDocument(bsoncxx::document::view document);
domain::WebhookTarget mapWebhookTargetDocument(bsoncxx::document::view document);

}

#pragma once

#include <bsoncxx/document/view.hpp>

#include <optional>

#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/Alarm.h"
#include "securezone/domain/AppUser.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/QrCheckIn.h"
#include "securezone/domain/WebhookTarget.h"
#include "securezone/domain/Zone.h"
#include "securezone/domain/CameraObjectTrack.h"
#include "securezone/domain/TrackIdentityBinding.h"

namespace securezone::infrastructure::mongodb {

domain::Employee mapEmployeeDocument(bsoncxx::document::view document);
std::optional<domain::AppUser> mapAppUserDocument(bsoncxx::document::view document) noexcept;
domain::Zone mapZoneDocument(bsoncxx::document::view document);
domain::CameraObjectTrack mapCameraObjectTrackDocument(bsoncxx::document::view document);
domain::TrackIdentityBinding mapTrackIdentityBindingDocument(bsoncxx::document::view document);
domain::MachineState mapMachineDocument(bsoncxx::document::view document);
domain::AccessPolicy mapAccessPolicyDocument(bsoncxx::document::view document);
domain::Alarm mapAlarmDocument(bsoncxx::document::view document);
domain::QrCheckin mapQrCheckinDocument(bsoncxx::document::view document);
domain::PresenceSession mapPresenceSessionDocument(bsoncxx::document::view document);
domain::WebhookTarget mapWebhookTargetDocument(bsoncxx::document::view document);

}

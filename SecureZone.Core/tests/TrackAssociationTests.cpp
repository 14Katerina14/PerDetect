#include <cassert>
#include "securezone/identity/TrackAssociationService.h"
int main() {
    using namespace securezone::domain; const auto now = std::chrono::system_clock::now();
    const Zone zone{"z", "Zone", "cam"}; const Detection detection{"track", "cam", ObjectClass::Person};
    TrackIdentityBinding bound = securezone::identity::TrackAssociationService{}.associate(detection, zone, {{"id","e","z","device",QrCheckInStatus::Active,now,now + std::chrono::minutes(1)}}, now);
    assert(bound.status == BindingStatus::Bound && bound.employeeId == "e");
    assert(securezone::identity::TrackAssociationService{}.associate(detection, zone, {{"id","e","z","device",QrCheckInStatus::Active,now - std::chrono::minutes(2),now - std::chrono::minutes(1)}}, now).status == BindingStatus::Expired);
    assert(securezone::identity::TrackAssociationService{}.associate(detection, zone, {{"id","e","other","device",QrCheckInStatus::Active,now,now + std::chrono::minutes(1)}}, now).status == BindingStatus::Expired);
    assert(securezone::identity::TrackAssociationService{}.associate(detection, zone, {{"a","e1","z","device",QrCheckInStatus::Active,now,now + std::chrono::minutes(1)},{"b","e2","z","device",QrCheckInStatus::Active,now,now + std::chrono::minutes(1)}}, now).status == BindingStatus::Uncertain);
}

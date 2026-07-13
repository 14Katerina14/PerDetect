#include <cassert>
#include "securezone/geometry/ZoneGeometryService.h"

int main() {
    const securezone::domain::Zone zone{"danger", "Danger", "cam", securezone::domain::ZoneType::Dangerous,
        securezone::domain::ZoneStatus::Active, {{0,0},{1,0},{1,1},{0,1}}, "machine"};
    const securezone::geometry::ZoneGeometryService service;
    assert(service.containsPoint(zone, {0.5, 0.5}));
    assert(!service.containsPoint(zone, {1.5, 0.5}));
    assert(service.containsBoundingBoxCenter(zone, {0.4,0.4,0.2,0.2}));
    auto invalid = zone; invalid.polygon = {{0,0},{1,1}};
    assert(!service.containsPoint(invalid, {0.5,0.5}));
}

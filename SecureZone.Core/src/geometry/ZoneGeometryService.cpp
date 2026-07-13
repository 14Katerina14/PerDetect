#include "securezone/geometry/ZoneGeometryService.h"

namespace securezone::geometry {
bool ZoneGeometryService::containsPoint(const domain::Zone& zone, const domain::Point& point) const {
    const auto& polygon = zone.polygon;
    if (polygon.size() < 3) return false;
    bool inside = false;
    for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const auto& a = polygon[i]; const auto& b = polygon[j];
        const bool intersects = ((a.y > point.y) != (b.y > point.y)) &&
            (point.x < (b.x - a.x) * (point.y - a.y) / (b.y - a.y) + a.x);
        if (intersects) inside = !inside;
    }
    return inside;
}
bool ZoneGeometryService::containsBoundingBoxCenter(const domain::Zone& zone, const domain::BoundingBox& bbox) const {
    return containsPoint(zone, bbox.center());
}
}

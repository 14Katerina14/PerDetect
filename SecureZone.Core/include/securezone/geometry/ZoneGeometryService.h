#pragma once

#include "securezone/domain/BoundingBox.h"
#include "securezone/domain/Zone.h"

namespace securezone::geometry {
class ZoneGeometryService {
public:
    bool containsPoint(const domain::Zone& zone, const domain::Point& point) const;
    bool containsBoundingBoxCenter(const domain::Zone& zone, const domain::BoundingBox& bbox) const;
};
}

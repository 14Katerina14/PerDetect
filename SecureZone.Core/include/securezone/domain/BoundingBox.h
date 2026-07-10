#pragma once

#include "securezone/domain/Point.h"

namespace securezone::domain {
struct BoundingBox {
    double x{}; double y{}; double width{}; double height{};
    Point center() const { return {x + width / 2.0, y + height / 2.0}; }
};
}

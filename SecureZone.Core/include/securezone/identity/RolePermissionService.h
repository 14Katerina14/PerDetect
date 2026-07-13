#pragma once

#include <vector>
#include "securezone/domain/RolePermission.h"
namespace securezone::identity { class RolePermissionService { public: bool hasPermission(const domain::RolePermission&, domain::UiPermission) const; }; }

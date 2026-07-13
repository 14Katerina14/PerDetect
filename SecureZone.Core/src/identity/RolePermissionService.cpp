#include "securezone/identity/RolePermissionService.h"
#include <algorithm>
namespace securezone::identity { bool RolePermissionService::hasPermission(const domain::RolePermission& role, domain::UiPermission permission) const { return std::find(role.permissions.begin(), role.permissions.end(), permission) != role.permissions.end(); } }

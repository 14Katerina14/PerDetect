#pragma once

#include <optional>
#include <string>

#include "securezone/domain/AccessPolicy.h"

namespace securezone::repository {

class IAccessPolicyRepository {
public:
    virtual ~IAccessPolicyRepository() = default;

    virtual std::optional<domain::AccessPolicy> findByZoneId(
        const std::string& zoneId
    ) const = 0;

    virtual bool save(const domain::AccessPolicy& policy) = 0;
};

}

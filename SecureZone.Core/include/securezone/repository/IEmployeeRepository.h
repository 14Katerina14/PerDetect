#pragma once

#include <optional>
#include <string>

#include "securezone/domain/Employee.h"

namespace securezone::repository {

class IEmployeeRepository {
public:
    virtual ~IEmployeeRepository() = default;

    virtual std::optional<domain::Employee> findByEmployeeId(
        const std::string& employeeId
    ) const = 0;
};

}

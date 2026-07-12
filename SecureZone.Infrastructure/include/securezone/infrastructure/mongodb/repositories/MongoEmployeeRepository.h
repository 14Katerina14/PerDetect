#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>

#include "securezone/domain/Employee.h"
#include "securezone/repository/IEmployeeRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoEmployeeRepository final : public repository::IEmployeeRepository {
public:
    explicit MongoEmployeeRepository(mongocxx::collection employeesCollection);

    std::optional<domain::Employee> findByEmployeeId(
        const std::string& employeeId
    ) const override;

private:
    mongocxx::collection employeesCollection_;
};

}

#include "securezone/infrastructure/mongodb/repositories/MongoEmployeeRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr char EmployeeIdField[] = "employeeId";

}

MongoEmployeeRepository::MongoEmployeeRepository(
    mongocxx::collection employeesCollection
) : employeesCollection_{std::move(employeesCollection)} {
}

std::optional<domain::Employee> MongoEmployeeRepository::findByEmployeeId(
    const std::string& employeeId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(EmployeeIdField, employeeId));

    auto result = employeesCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapEmployeeDocument(result->view());
}

}

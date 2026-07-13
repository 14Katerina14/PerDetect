#pragma once

#include <string>
#include <vector>
namespace securezone::domain {
enum class EmployeeStatus { Active, Inactive };
struct Employee { std::string employeeId, fullName, position; std::vector<std::string> roles; EmployeeStatus status{EmployeeStatus::Active}; };
}

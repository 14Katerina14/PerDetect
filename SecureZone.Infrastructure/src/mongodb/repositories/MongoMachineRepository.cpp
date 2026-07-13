#include "securezone/infrastructure/mongodb/repositories/MongoMachineRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include <chrono>
#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr const char* MachineIdField = "machineId";

const char* machineStatusToString(domain::MachineStatus status) {
    switch (status) {
        case domain::MachineStatus::Running:
            return "running";
        case domain::MachineStatus::Stopped:
            return "stopped";
        case domain::MachineStatus::Maintenance:
            return "maintenance";
    }

    return "stopped";
}

bsoncxx::types::b_date toBsonDate(std::chrono::system_clock::time_point timePoint) {
    return bsoncxx::types::b_date{
        std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch())
    };
}

}

MongoMachineRepository::MongoMachineRepository(
    mongocxx::collection machinesCollection
) : machinesCollection_{std::move(machinesCollection)} {
}

std::optional<domain::MachineState> MongoMachineRepository::findByMachineId(
    const std::string& machineId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(MachineIdField, machineId));

    auto result = machinesCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapMachineDocument(result->view());
}

bool MongoMachineRepository::updateStatus(
    const std::string& machineId,
    domain::MachineStatus status,
    std::chrono::system_clock::time_point updatedAt
) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(MachineIdField, machineId));

    bsoncxx::builder::basic::document fields;
    fields.append(
        bsoncxx::builder::basic::kvp("status", machineStatusToString(status)),
        bsoncxx::builder::basic::kvp("updatedAt", toBsonDate(updatedAt))
    );

    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", fields.extract()));

    const auto result = machinesCollection_.update_one(filter.view(), update.view());
    return result.has_value() && result->matched_count() == 1;
}

}

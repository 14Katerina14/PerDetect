#include "securezone/infrastructure/mongodb/repositories/MongoMachineRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr const char* MachineIdField = "machineId";

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

}

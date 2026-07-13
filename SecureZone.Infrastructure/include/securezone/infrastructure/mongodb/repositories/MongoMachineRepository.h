#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>

#include "securezone/domain/MachineState.h"
#include "securezone/repository/IMachineRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoMachineRepository final : public repository::IMachineRepository {
public:
    explicit MongoMachineRepository(mongocxx::collection machinesCollection);

    std::optional<domain::MachineState> findByMachineId(
        const std::string& machineId
    ) const override;

    bool updateStatus(
        const std::string& machineId,
        domain::MachineStatus status,
        std::chrono::system_clock::time_point updatedAt
    ) override;

private:
    mongocxx::collection machinesCollection_;
};

}

#include "securezone/query/ZoneStatusQueryService.h"

#include <algorithm>
#include <utility>

namespace securezone::query {
namespace {

std::size_t normalizedLimit(std::size_t limit) {
    return std::min<std::size_t>(limit == 0U ? 50U : limit, 100U);
}

}

ZoneStatusQueryService::ZoneStatusQueryService(
    repository::IZoneStatusReadRepository& zoneRepository,
    repository::IMachineRepository& machineRepository
) : zoneRepository_{zoneRepository},
    machineRepository_{machineRepository} {
}

std::vector<ZoneStatusView> ZoneStatusQueryService::list(
    std::size_t limit,
    const std::optional<std::string>& cameraId
) const {
    std::vector<ZoneStatusView> views;
    for (const auto& zone : zoneRepository_.findAll(normalizedLimit(limit), cameraId)) {
        ZoneStatusView view{};
        view.zoneId = zone.zoneId;
        view.zoneName = zone.name;
        view.cameraId = zone.cameraId;
        view.type = zone.type;
        view.status = zone.status;
        view.machineId = zone.relatedMachineId;

        if (!zone.relatedMachineId.empty()) {
            const auto machine = machineRepository_.findByMachineId(zone.relatedMachineId);
            if (machine.has_value()) {
                view.machineName = machine->name;
                view.machineStatus = machine->status;
            }
        }
        views.push_back(std::move(view));
    }
    return views;
}

}

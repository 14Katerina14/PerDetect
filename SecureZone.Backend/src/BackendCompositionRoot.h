#pragma once

#include "BackendApplication.h"
#include "BackendRuntimeOptions.h"

namespace securezone::backend {

BackendApplication composeBackendApplication(const BackendRuntimeOptions& options);

}

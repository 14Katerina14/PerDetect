#pragma once

#include <string>

#include "securezone/api/http/HttpResponse.h"

namespace securezone::api {

HttpResponse jsonResponse(int statusCode, std::string body);
HttpResponse jsonOk(std::string body);
HttpResponse jsonCreated(std::string body);
HttpResponse jsonBadRequest(std::string body);
HttpResponse jsonNotFound(std::string body);
HttpResponse jsonMethodNotAllowed(std::string body);
HttpResponse jsonInternalError(std::string body);
HttpResponse jsonNotImplemented(std::string body);

}

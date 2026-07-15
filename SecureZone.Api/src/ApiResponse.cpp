#include "securezone/api/ApiResponse.h"

#include <utility>

namespace securezone::api {

HttpResponse jsonResponse(int statusCode, std::string body) {
    return HttpResponse{statusCode, "application/json", std::move(body)};
}

HttpResponse jsonOk(std::string body) {
    return jsonResponse(200, std::move(body));
}

HttpResponse jsonCreated(std::string body) {
    return jsonResponse(201, std::move(body));
}

HttpResponse jsonBadRequest(std::string body) {
    return jsonResponse(400, std::move(body));
}

HttpResponse jsonNotFound(std::string body) {
    return jsonResponse(404, std::move(body));
}

HttpResponse jsonMethodNotAllowed(std::string body) {
    return jsonResponse(405, std::move(body));
}

HttpResponse jsonInternalError(std::string body) {
    return jsonResponse(500, std::move(body));
}

HttpResponse jsonNotImplemented(std::string body) {
    return jsonResponse(501, std::move(body));
}

}

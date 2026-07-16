#pragma once

#include "securezone/api/http/HttpRequest.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>

namespace securezone::api {

inline bool headerNameEquals(std::string_view left, std::string_view right) {
    return left.size() == right.size()
        && std::equal(left.begin(), left.end(), right.begin(), [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a))
                == std::tolower(static_cast<unsigned char>(b));
        });
}

inline std::optional<std::string> findHeaderValue(
    const HttpRequest& request,
    std::string_view name
) {
    for (const auto& [headerName, value] : request.headers) {
        if (headerNameEquals(headerName, name)) {
            return value;
        }
    }
    return std::nullopt;
}

}

//
// Created by hohaia on 02/09/2026.
//
#ifndef WXCLIENT_HELPERS_H
#define WXCLIENT_HELPERS_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ict
{
    using ResponseTable = std::vector<std::pair<std::string, std::string>>;

    std::string trim(const std::string& str);
    std::string toHex(const std::vector<std::uint8_t>& bytes);
    std::vector<std::uint8_t> fromHex(const std::string& hexStr);
    std::string urlDecode(const std::string& str);
    ResponseTable parseQueryString(const std::string& query);
}

#endif //WXCLIENT_HELPERS_H

//
// Created by hohaia on 02/09/2026.
//

#ifndef WXCLIENT_HELPERS_H
#define WXCLIENT_HELPERS_H

#include <cstdint>
#include <string>
#include <vector>

namespace ict
{
    [[nodiscard]] std::string trim(const std::string& str);
    [[nodiscard]] std::string toHex(const std::vector<std::uint8_t>& bytes);
    [[nodiscard]] std::vector<std::uint8_t> fromHex(const std::string& hexStr);
    [[nodiscard]] std::string sha1Hex(const std::string& inputString);
}

#endif //WXCLIENT_HELPERS_H

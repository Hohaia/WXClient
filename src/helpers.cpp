//
// Created by hohaia on 02/09/2026.
//
#include <iomanip>
#include <sstream>
#include <string_view>

#include "helpers.h"

//remove leading and trailing whitespace from a string
std::string trim(const std::string& str)
{
    constexpr std::string_view whitespace = " \t\r\n";
    const auto first = str.find_first_not_of(whitespace);
    if (first == std::string::npos)
    {
        return {};
    }
    const auto last = str.find_last_not_of(whitespace);
    return str.substr(first, last - first + 1);
}

//convert a byte string to a hex string
std::string toHex(const std::vector<std::uint8_t>& bytes)
{
    std::ostringstream oss;
    for (const auto& byte : bytes)
    {
        oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(byte);
    }
    return oss.str();
}

//convert a hex string to a byte string
std::vector<std::uint8_t> fromHex(const std::string& hexStr)
{
    std::vector<std::uint8_t> bytes;
    bytes.reserve(hexStr.size() / 2);

    for (std::size_t i = 0; i < hexStr.size(); i += 2)
    {
        const std::string byteStr = hexStr.substr(i, 2);
        bytes.push_back(static_cast<std::uint8_t>(std::stoi(byteStr, nullptr, 16)));
    }

    return bytes;
}

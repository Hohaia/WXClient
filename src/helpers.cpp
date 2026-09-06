//
// Created by hohaia on 02/09/2026.
//
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string_view>

#include "helpers.h"

//convert a single hex degit to its values, or -1 if its not hex
static int hexDigit(const unsigned char ch)
{
    if (ch >= '0' && ch <= '9') { return ch - '0'; }
    if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
    if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
    return -1;
}

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

//decode the %XX escapes and '+' separators in a url encoded string
std::string urlDecode(const std::string &str)
{
    std::string decodedStr;
    decodedStr.reserve(str.size());
    for (std::size_t i = 0; i < str.size(); i++)
    {
        const char ch = str[i];
        if ('+' == ch)
        {
            decodedStr.push_back(' ');
            continue;
        }
        if ('%' == ch && i + 2 < str.size())
        {
            const int high = hexDigit(static_cast<unsigned char>(str[i + 1]));
            const int low = hexDigit(static_cast<unsigned char>(str[i + 2]));
            if (0 <= high && 0 <= low)
            {
                decodedStr.push_back(static_cast<char>(high * 16 + low));
                i += 2;
                continue;
            }
        }
        decodedStr.push_back(ch);
    }
    return decodedStr;
}

//split a url encoded "key=value&key=value" string into decoded key/value pairs
std::multimap<std::string, std::string> parseQueryString(const std::string& query)
{
    std::multimap<std::string, std::string> params;
    std::size_t start = 0;
    while (start <= query.size())
    {
        std::size_t end = query.find('&', start);
        if (end == std::string::npos)
        {
            end = query.size();
        }
        const std::string pair = query.substr(start, end - start);
        start = end + 1;
        if (pair.empty())
        {
            continue;
        }
        const auto separator = pair.find('=');
        std::string key = urlDecode(separator == std::string::npos ? pair : pair.substr(0, separator));
        std::string value = separator == std::string::npos ? std::string{} : urlDecode(pair.substr(separator + 1));
        params.emplace(trim(key), std::move(value));
    }
    return params;
}


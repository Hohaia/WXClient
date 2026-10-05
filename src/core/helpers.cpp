//
// Created by hohaia on 02/09/2026.
//

#include "helpers.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <openssl/evp.h>

namespace ict::core
{
    // Remove leading and trailing whitespace from a string.
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

    // Lowercase a string (ASCII only).
    std::string toLower(std::string str)
    {
        std::ranges::transform(str, str.begin(),
                               [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return str;
    }

    // Convert a byte string to a hex string.
    std::string toHex(const std::span<const std::uint8_t> bytes)
    {
        std::ostringstream oss;
        for (const auto& byte : bytes)
        {
            oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(byte);
        }
        return oss.str();
    }

    // Format a hex fingerprint for display, a colon between each byte ("AB12CD" -> "AB:12:CD").
    std::string formatFingerprint(const std::string& hex)
    {
        std::string formatted;
        formatted.reserve(hex.size() * 3 / 2);
        for (std::size_t i = 0; i < hex.size(); i += 2)
        {
            if (i > 0)
                formatted += ':';
            formatted += hex.substr(i, 2);
        }
        return formatted;
    }

    // Convert a hex string to a byte string.
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

    // Create a sha1 checksum from a string.
    std::string sha1Hex(const std::string& inputString)
    {
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int digestLength = 0;
        const std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
        if (!ctx)
        {
            throw std::runtime_error("Failed to create EVP context");
        }
        if (EVP_DigestInit_ex(ctx.get(), EVP_sha1(), nullptr) != 1
            || EVP_DigestUpdate(ctx.get(), inputString.data(), inputString.size()) != 1
            || EVP_DigestFinal_ex(ctx.get(), digest, &digestLength) != 1)
        {
            throw std::runtime_error("Failed to compute SHA-1");
        }
        return toHex(std::span(digest, digestLength));   // The first digestLength bytes of 'digest'.
    }

    // Where wxclient keeps its state (logs, trusted certificates), regardless of the process's cwd.
    std::filesystem::path stateDirectory()
    {
        #ifdef _WIN32
        #error "Windows support: resolve %LOCALAPPDATA%\\wxclient here once console.cpp no longer depends on termios."
        #else
        const char* xdgState = std::getenv("XDG_STATE_HOME");
        const char* home = std::getenv("HOME");
        const std::filesystem::path base = (xdgState && *xdgState)
            ? std::filesystem::path(xdgState)
            : std::filesystem::path(home ? home : ".") / ".local" / "state";
        return base / "wxclient";
        #endif
    }
}

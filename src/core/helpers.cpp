//
// Created by hohaia on 02/09/2026.
//

#include "helpers.h"

#include <cstddef>
#include <iomanip>
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

    // Convert a byte string to a hex string.
    std::string toHex(const std::vector<std::uint8_t>& bytes)
    {
        std::ostringstream oss;
        for (const auto& byte : bytes)
        {
            oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(byte);
        }
        return oss.str();
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
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx)
        {
            throw std::runtime_error("Failed to create EVP context");
        }
        if (EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr) != 1
            || EVP_DigestUpdate(ctx, inputString.data(), inputString.size()) != 1
            || EVP_DigestFinal_ex(ctx, digest, &digestLength) != 1)
        {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to compute SHA-1");
        }
        EVP_MD_CTX_free(ctx);
        return toHex(std::vector<std::uint8_t>(digest, digest + digestLength));
    }
}

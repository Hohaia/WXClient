//
// Created by hohaia on 24/09/2026.
//

#include "status.h"

#include <algorithm>
#include <charconv>
#include <optional>
#include <span>

#include "status_code_tables.h"

namespace ict
{
    namespace
    {
        const StatusCodeTable* findTable(std::string_view tableName)
        {
            const auto it = std::ranges::find_if(statusCodeTables,
                            [&](const StatusCodeTable& table)
                            { return table.name == tableName;});
            if (it == statusCodeTables.end())
                return nullptr;
            return &*it;
        }

        std::optional<std::string_view> findLabel(std::span<const StatusCodeLabel> labels, const int code)
        {
            const auto it = std::ranges::find_if(labels,
                            [code](const StatusCodeLabel& label) { return label.code == code; });
            if (it == labels.end())
                return std::nullopt;
            return it->label;
        }

        std::string decodeValue(const StatusCodeField& field, const int code)
        {
            if (const auto label = findLabel(field.labels, code))
                return std::string(*label);
            return "Unknown (" + std::to_string(code) + ")";
        }

        std::string decodeBits(const StatusCodeField& field, const int value)
        {
            if (value == 0)
                return "None";
            const auto bits = static_cast<unsigned>(value);
            std::string text;
            for (int bit = 0; bit < 32; ++bit)
            {
                if ((bits & (1u << bit)) == 0)
                    continue;
                if (!text.empty())
                    text += ", ";
                const auto label = findLabel(field.labels, bit);
                text += label ? std::string(*label) : "Bit " + std::to_string(bit);
            }
            return text;
        }

        std::string decodeField(const StatusCodeField& field, const std::string_view token)
        {
            int value = 0;
            const auto* const end = token.data() + token.size();
            const auto [ptr, ec] = std::from_chars(token.data(), end, value);
            if (ec != std::errc{} || ptr != end || field.type == StatusType::Raw)
                return std::string(token);
            if (field.type == StatusType::Bits)
                return decodeBits(field, value);
            return decodeValue(field, value);
        }
    }

    // Decode the status tables.
    std::vector<DecodedField> decodeStatus(const std::string_view tableName, const std::string_view rawValue)
    {
        std::vector<DecodedField> decoded;
        if (rawValue.empty())
            return decoded;
        const StatusCodeTable* table = findTable(tableName);
        std::size_t start = 0;
        for (std::size_t i = 0; start <= rawValue.size(); ++i)
        {
            std::size_t end = rawValue.find(',', start);
            if (end == std::string_view::npos)
                end = rawValue.size();
            const std::string_view token = rawValue.substr(start, end - start);
            start = end + 1;
            if (!table || i >= table->fields.size())
            {
                decoded.emplace_back("", std::string(token));
                continue;
            }
            const auto& field = table->fields[i];
            decoded.emplace_back(field.name, decodeField(field, token));
        }
        return decoded;
    }
}
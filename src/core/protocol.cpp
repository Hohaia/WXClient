//
// Created by hohaia on 27/09/2026.
//

#include "protocol.h"

#include "helpers.h"

namespace ict::core
{
    // Replace the characters the controller reserves in request values (see overview-api-requests.md).
    std::string encodeStringValue(const std::string& value)
    {
        std::string encoded;
        encoded.reserve(value.size());
        for (const char ch : value)
        {
            switch (ch)
            {
                case '&':  encoded += "%11"; break;
                case '=':  encoded += "%12"; break;
                case '\n': encoded += "%0A"; break;
                case '\r': break; // CR LF becomes a single %0A.
                default:   encoded += ch;
            }
        }
        return encoded;
    }

    // Undo the controller's response delimiter swap: 0xE0 -> '&', 0xCD -> '=' (see overview-api-requests.md).
    std::string decodeStringValue(const std::string& str)
    {
        std::string decoded;
        decoded.reserve(str.size());
        for (const char ch : str)
        {
            switch (static_cast<unsigned char>(ch))
            {
                case 0xE0: decoded += '&'; break;
                case 0xCD: decoded += '='; break;
                default:   decoded += ch;
            }
        }
        return decoded;
    }

    // Split a encoded "key=value&key=value" string into decoded key/value pairs.
    KeyValueList parseQueryString(const std::string& query)
    {
        KeyValueList params;
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
            std::string key = decodeStringValue(separator == std::string::npos ? pair : pair.substr(0, separator));
            std::string value = separator == std::string::npos ? std::string{} : decodeStringValue(pair.substr(separator + 1));
            params.emplace_back(trim(key), std::move(value));
        }
        return params;
    }

    // Convert a RequestType to a string for ControllerApi::sendRequest().
    std::string toString(const RequestType type)
    {
        switch (type)
        {
            case RequestType::List:             return "List";
            case RequestType::Detail:           return "Detail";
            case RequestType::Events:           return "Events";
            case RequestType::Status:           return "Status";
            case RequestType::Health:           return "Health";
            case RequestType::Modules:          return "Modules";
            case RequestType::DuplicateCheck:   return "DuplicateCheck";
            case RequestType::System:           return "System";
        }
        return {};
    }

    // Convert a CommandType to a string for ControllerApi::sendCommand().
    std::string toString(const CommandType type)
    {
        switch (type)
        {
            case CommandType::Submit:               return "Submit";
            case CommandType::Delete:               return "Delete";
            case CommandType::Modules:              return "Modules";
            case CommandType::Control:              return "Control";
            case CommandType::RestartController:    return "RestartController";
        }
        return {};
    }
}
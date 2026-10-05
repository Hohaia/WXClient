//
// Created by hohaia on 05/10/2026.
//

#ifndef WXCLIENT_KNOWN_CERTIFICATES_H
#define WXCLIENT_KNOWN_CERTIFICATES_H

#include <optional>
#include <string>

namespace ict::core
{
    std::optional<std::string> loadTrustedFingerprint(const std::string& host);
    bool saveTrustedFingerprint(const std::string& host, const std::string& fingerprint);
}

#endif //WXCLIENT_KNOWN_CERTIFICATES_H

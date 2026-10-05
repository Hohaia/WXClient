//
// Created by hohaia on 05/10/2026.
//

#include "known_certificates.h"

#include <filesystem>
#include <fstream>
#include <utility>
#include <vector>

#include "helpers.h"
#include "logger.h"

namespace ict::core
{
    namespace
    {
        // The trusted certificates file: one "host fingerprint" pair per line.
        std::filesystem::path knownCertificatesPath()
        {
            return stateDirectory() / "known_certificates";
        }
    }

    // Look up the trusted certificate fingerprint for 'host'; nullopt if there isn't one.
    std::optional<std::string> loadTrustedFingerprint(const std::string& host)
    {
        std::ifstream file(knownCertificatesPath());
        std::string fileHost, fingerprint;
        while (file >> fileHost >> fingerprint)
        {
            if (fileHost == host)
                return fingerprint;
        }
        return std::nullopt;
    }

    // Save 'fingerprint' as the trusted certificate for 'host', replacing any old one.
    bool saveTrustedFingerprint(const std::string& host, const std::string& fingerprint)
    {
        std::vector<std::pair<std::string, std::string>> entries;
        std::ifstream in(knownCertificatesPath());
        std::string fileHost, fileFingerprint;
        while (in >> fileHost >> fileFingerprint)
        {
            if (fileHost != host)
                entries.emplace_back(fileHost, fileFingerprint);
        }
        in.close();
        entries.emplace_back(host, fingerprint);

        std::error_code ec;
        std::filesystem::create_directories(stateDirectory(), ec);   // A failure shows up as the ofstream failing.
        std::ofstream out(knownCertificatesPath(), std::ios::trunc);
        for (const auto& [entryHost, entryFingerprint] : entries)
            out << entryHost << ' ' << entryFingerprint << '\n';
        if (!out)
        {
            logMessage(LogLevel::Error, "saveTrustedFingerprint", "Could not write " + knownCertificatesPath().string());
            return false;
        }
        return true;
    }
}

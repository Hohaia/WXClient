//
// Created by hohaia on 02/09/2026.
//
#ifndef WXCLIENT_HELPERS_H
#define WXCLIENT_HELPERS_H

#include <cstdint>
#include <string>
#include <vector>

std::string trim(const std::string& str);
std::string toHex(const std::vector<std::uint8_t>& bytes);
std::vector<std::uint8_t> fromHex(const std::string& hexStr);

#endif //WXCLIENT_HELPERS_H

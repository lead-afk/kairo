#pragma once
#include <ctime>
#include <random>
#include <string>

namespace kairo::fn
{

/**
 * @brief Generates a UUID string.
 * @return A string representing the generated UUID.
 */
inline std::string generate_uuid()
{
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_int_distribution<uint32_t> dist(
        0, 0xFFFFFFFF);

    uint32_t data[4];
    for (int i = 0; i < 4; ++i)
        data[i] = dist(gen);

    uint8_t *bytes = reinterpret_cast<uint8_t *>(data);

    // Version 4
    bytes[6] = (bytes[6] & 0x0F) | 0x40;
    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    char out[37];
    std::snprintf(
        out, sizeof(out),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6],
        bytes[7], bytes[8], bytes[9], bytes[10], bytes[11], bytes[12],
        bytes[13], bytes[14], bytes[15]);

    return out;
}

} // namespace kairo::fn
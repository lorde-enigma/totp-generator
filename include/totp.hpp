#ifndef TOTP_HPP
#define TOTP_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace totp {

struct TOTPResult {
    std::string code;
    int time_remaining;
};

struct TOTPEntry {
    std::string name;
    std::string secret;
    int time_step;
};

std::vector<uint8_t> base32_decode(const std::string& base32);
std::vector<uint8_t> hmac_sha1(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data);
TOTPResult generate_totp(const std::string& secret, int time_step = 30);
void run_generator(const std::string& secret, int time_step = 30);

}

#endif

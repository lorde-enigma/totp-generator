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

/**
 * decode base32 encoded string to raw bytes
 * @param base32 base32 encoded secret key
 * @return decoded bytes
 */
std::vector<uint8_t> base32_decode(const std::string& base32);

/**
 * generate hmac-sha1 hash
 * @param key secret key bytes
 * @param data data to hash
 * @return hmac-sha1 hash (20 bytes)
 */
std::vector<uint8_t> hmac_sha1(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data);

/**
 * generate totp code from secret
 * @param secret base32 encoded secret key
 * @param time_step time step in seconds (default 30)
 * @return TOTPResult containing code and time remaining
 */
TOTPResult generate_totp(const std::string& secret, int time_step = 30);

/**
 * run continuous totp generation loop
 * @param secret base32 encoded secret key
 * @param time_step time step in seconds (default 30)
 */
void run_generator(const std::string& secret, int time_step = 30);

} // namespace totp

#endif // TOTP_HPP

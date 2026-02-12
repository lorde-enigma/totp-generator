#include "totp.hpp"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace totp {

static const char BASE32_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

std::vector<uint8_t> base32_decode(const std::string& base32) {
    std::string input;
    for (char c : base32) {
        if (!std::isspace(c)) {
            input += std::toupper(c);
        }
    }

    std::string bits;
    for (char c : input) {
        const char* pos = std::strchr(BASE32_CHARS, c);
        if (pos == nullptr) continue;
        
        int val = pos - BASE32_CHARS;
        for (int i = 4; i >= 0; --i) {
            bits += ((val >> i) & 1) ? '1' : '0';
        }
    }

    std::vector<uint8_t> bytes;
    for (size_t i = 0; i + 8 <= bits.length(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; ++j) {
            byte = (byte << 1) | (bits[i + j] == '1' ? 1 : 0);
        }
        bytes.push_back(byte);
    }

    return bytes;
}

std::vector<uint8_t> hmac_sha1(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) {
    unsigned int len = SHA_DIGEST_LENGTH;
    std::vector<uint8_t> result(len);

    HMAC(EVP_sha1(), 
         key.data(), static_cast<int>(key.size()),
         data.data(), data.size(),
         result.data(), &len);

    return result;
}

TOTPResult generate_totp(const std::string& secret, int time_step) {
    std::vector<uint8_t> key = base32_decode(secret);

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    
    uint64_t counter = seconds / time_step;
    int time_remaining = time_step - (seconds % time_step);

    std::vector<uint8_t> counter_bytes(8);
    for (int i = 7; i >= 0; --i) {
        counter_bytes[i] = counter & 0xff;
        counter >>= 8;
    }

    std::vector<uint8_t> hash = hmac_sha1(key, counter_bytes);

    int offset = hash[19] & 0x0f;
    uint32_t code = ((hash[offset] & 0x7f) << 24) |
                    ((hash[offset + 1] & 0xff) << 16) |
                    ((hash[offset + 2] & 0xff) << 8) |
                    (hash[offset + 3] & 0xff);

    code = code % 1000000;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(6) << code;

    return { oss.str(), time_remaining };
}

void run_generator(const std::string& secret, int time_step) {
    std::string last_code;

    std::cout << "\n";
    std::cout << "  ┌─────────────────────────────────────┐\n";
    std::cout << "  │         TOTP CODE GENERATOR         │\n";
    std::cout << "  └─────────────────────────────────────┘\n";
    std::cout << "\n";
    std::cout << "  [*] secret: " << secret << "\n";
    std::cout << "  [*] time step: " << time_step << "s\n";
    std::cout << "  [*] press ctrl+c to exit\n";
    std::cout << "\n";

    while (true) {
        TOTPResult result = generate_totp(secret, time_step);

        if (result.code != last_code) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm* tm = std::localtime(&time);

            std::cout << "  [" << std::setfill('0') << std::setw(2) << tm->tm_hour << ":"
                      << std::setfill('0') << std::setw(2) << tm->tm_min << ":"
                      << std::setfill('0') << std::setw(2) << tm->tm_sec << "] ";
            
            std::cout << "\033[1;32m" << result.code << "\033[0m";
            std::cout << " (expires in " << result.time_remaining << "s)\n";
            
            last_code = result.code;
        }

        int elapsed = time_step - result.time_remaining;
        int bar_width = 30;
        int filled = (elapsed * bar_width) / time_step;

        std::cout << "\r  [";
        for (int i = 0; i < bar_width; ++i) {
            if (i < filled) {
                std::cout << "\033[1;36m█\033[0m";
            } else {
                std::cout << "░";
            }
        }
        std::cout << "] " << std::setw(2) << result.time_remaining << "s " << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

}

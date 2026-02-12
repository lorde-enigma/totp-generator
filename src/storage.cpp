#include "storage.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>

namespace storage {

static const int SALT_SIZE = 16;
static const int IV_SIZE = 16;
static const int KEY_SIZE = 32;
static const int ITERATIONS = 100000;

std::string get_storage_path() {
    const char* home = std::getenv("HOME");
    if (!home) home = "/tmp";
    return std::string(home) + "/.totp_vault";
}

bool storage_exists() {
    struct stat st;
    return stat(get_storage_path().c_str(), &st) == 0;
}

std::vector<uint8_t> derive_key(const std::string& password, const std::vector<uint8_t>& salt) {
    std::vector<uint8_t> key(KEY_SIZE);
    PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                      salt.data(), salt.size(),
                      ITERATIONS, EVP_sha256(),
                      KEY_SIZE, key.data());
    return key;
}

std::vector<uint8_t> encrypt_data(const std::string& data, const std::string& password) {
    std::vector<uint8_t> salt(SALT_SIZE);
    std::vector<uint8_t> iv(IV_SIZE);
    RAND_bytes(salt.data(), SALT_SIZE);
    RAND_bytes(iv.data(), IV_SIZE);

    std::vector<uint8_t> key = derive_key(password, salt);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

    std::vector<uint8_t> ciphertext(data.length() + EVP_MAX_BLOCK_LENGTH);
    int len = 0, total_len = 0;

    EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                      reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
    total_len = len;

    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
    total_len += len;

    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(total_len);

    std::vector<uint8_t> result;
    result.insert(result.end(), salt.begin(), salt.end());
    result.insert(result.end(), iv.begin(), iv.end());
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());

    return result;
}

std::string decrypt_data(const std::vector<uint8_t>& encrypted, const std::string& password) {
    if (encrypted.size() < SALT_SIZE + IV_SIZE + 1) {
        return "";
    }

    std::vector<uint8_t> salt(encrypted.begin(), encrypted.begin() + SALT_SIZE);
    std::vector<uint8_t> iv(encrypted.begin() + SALT_SIZE, encrypted.begin() + SALT_SIZE + IV_SIZE);
    std::vector<uint8_t> ciphertext(encrypted.begin() + SALT_SIZE + IV_SIZE, encrypted.end());

    std::vector<uint8_t> key = derive_key(password, salt);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    std::vector<uint8_t> plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH);
    int len = 0, total_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    total_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    total_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return std::string(reinterpret_cast<char*>(plaintext.data()), total_len);
}

std::string entries_to_string(const std::vector<totp::TOTPEntry>& entries) {
    std::ostringstream oss;
    for (const auto& e : entries) {
        oss << e.name << "\t" << e.secret << "\t" << e.time_step << "\n";
    }
    return oss.str();
}

std::vector<totp::TOTPEntry> string_to_entries(const std::string& data) {
    std::vector<totp::TOTPEntry> entries;
    std::istringstream iss(data);
    std::string line;

    while (std::getline(iss, line)) {
        if (line.empty()) continue;

        totp::TOTPEntry entry;
        size_t pos1 = line.find('\t');
        size_t pos2 = line.find('\t', pos1 + 1);

        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            entry.name = line.substr(0, pos1);
            entry.secret = line.substr(pos1 + 1, pos2 - pos1 - 1);
            entry.time_step = std::stoi(line.substr(pos2 + 1));
            entries.push_back(entry);
        }
    }

    return entries;
}

bool save_entries(const std::vector<totp::TOTPEntry>& entries, const std::string& password) {
    std::string data = entries_to_string(entries);
    std::vector<uint8_t> encrypted = encrypt_data(data, password);

    std::ofstream file(get_storage_path(), std::ios::binary);
    if (!file) return false;

    file.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
    return file.good();
}

std::vector<totp::TOTPEntry> load_entries(const std::string& password) {
    std::ifstream file(get_storage_path(), std::ios::binary);
    if (!file) return {};

    std::vector<uint8_t> encrypted((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());

    std::string data = decrypt_data(encrypted, password);
    if (data.empty()) return {};

    return string_to_entries(data);
}

bool verify_password(const std::string& password) {
    auto entries = load_entries(password);
    std::ifstream file(get_storage_path(), std::ios::binary);
    if (!file) return true;

    std::vector<uint8_t> encrypted((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
    file.close();

    std::string data = decrypt_data(encrypted, password);
    return !data.empty() || encrypted.size() == 0;
}

bool add_entry(const totp::TOTPEntry& entry, const std::string& password) {
    auto entries = load_entries(password);

    for (auto& e : entries) {
        if (e.secret == entry.secret) {
            e.name = entry.name;
            e.time_step = entry.time_step;
            return save_entries(entries, password);
        }
    }

    entries.push_back(entry);
    return save_entries(entries, password);
}

std::string prompt_password(bool confirm) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;

    std::cout << "  [>] password: " << std::flush;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string password;
    std::getline(std::cin, password);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << "\n";

    if (confirm) {
        std::cout << "  [>] confirm:  " << std::flush;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        std::string confirm_pw;
        std::getline(std::cin, confirm_pw);

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        std::cout << "\n";

        if (password != confirm_pw) {
            std::cerr << "  [!] passwords do not match\n";
            return "";
        }
    }

    return password;
}

void init_storage(const std::string& password) {
    std::vector<totp::TOTPEntry> empty;
    save_entries(empty, password);
}

}

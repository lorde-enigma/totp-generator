#ifndef STORAGE_HPP
#define STORAGE_HPP

#include "totp.hpp"
#include <string>
#include <vector>

namespace storage {

std::string get_storage_path();
bool storage_exists();
bool verify_password(const std::string& password);
std::vector<uint8_t> derive_key(const std::string& password, const std::vector<uint8_t>& salt);
std::vector<uint8_t> encrypt_data(const std::string& data, const std::string& password);
std::string decrypt_data(const std::vector<uint8_t>& encrypted, const std::string& password);
bool save_entries(const std::vector<totp::TOTPEntry>& entries, const std::string& password);
std::vector<totp::TOTPEntry> load_entries(const std::string& password);
bool add_entry(const totp::TOTPEntry& entry, const std::string& password);
std::string prompt_password(bool confirm = false);
void init_storage(const std::string& password);

}

#endif

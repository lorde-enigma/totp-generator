#include "totp.hpp"
#include "storage.hpp"
#include <iostream>
#include <cstring>

void print_header() {
    std::cout << "\n";
    std::cout << "  ┌─────────────────────────────────────┐\n";
    std::cout << "  │         TOTP CODE GENERATOR         │\n";
    std::cout << "  └─────────────────────────────────────┘\n";
    std::cout << "\n";
}

void print_help(const char* program) {
    print_header();
    std::cout << "  usage:\n";
    std::cout << "    " << program << " [secret] [options]\n";
    std::cout << "\n";
    std::cout << "  arguments:\n";
    std::cout << "    secret        base32 encoded totp secret (optional)\n";
    std::cout << "\n";
    std::cout << "  options:\n";
    std::cout << "    -n, --name    name for the totp entry\n";
    std::cout << "    -t, --time    time step in seconds (default: 30)\n";
    std::cout << "    -s, --single  generate single code and exit\n";
    std::cout << "    -l, --list    list saved entries\n";
    std::cout << "    -d, --delete  delete an entry\n";
    std::cout << "    -h, --help    show this help message\n";
    std::cout << "\n";
    std::cout << "  examples:\n";
    std::cout << "    " << program << "                           (select from saved)\n";
    std::cout << "    " << program << " CB6LUMRHEWPEYIXI -n github\n";
    std::cout << "    " << program << " CB6LUMRHEWPEYIXI --single\n";
    std::cout << "\n";
}

int select_entry(const std::vector<totp::TOTPEntry>& entries) {
    std::cout << "  ┌─────────────────────────────────────┐\n";
    std::cout << "  │          SAVED TOTP ENTRIES         │\n";
    std::cout << "  └─────────────────────────────────────┘\n";
    std::cout << "\n";

    for (size_t i = 0; i < entries.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << entries[i].name;
        std::cout << " (" << entries[i].time_step << "s)\n";
    }

    std::cout << "\n";
    std::cout << "  [0] exit\n";
    std::cout << "\n";
    std::cout << "  [>] select: " << std::flush;

    int choice;
    std::cin >> choice;
    std::cin.ignore();

    return choice;
}

int main(int argc, char* argv[]) {
    std::string secret;
    std::string name;
    int time_step = 30;
    bool single_mode = false;
    bool list_mode = false;
    bool delete_mode = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--time") == 0) {
            if (i + 1 < argc) {
                time_step = std::atoi(argv[++i]);
                if (time_step <= 0) {
                    std::cerr << "  [!] error: time step must be positive\n";
                    return 1;
                }
            }
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {
            if (i + 1 < argc) {
                name = argv[++i];
            }
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--single") == 0) {
            single_mode = true;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            list_mode = true;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delete") == 0) {
            delete_mode = true;
        } else if (argv[i][0] != '-') {
            secret = argv[i];
        }
    }

    print_header();

    bool first_run = !storage::storage_exists();
    std::string password;

    if (first_run) {
        std::cout << "  [*] first run - create master password\n";
        std::cout << "\n";
        password = storage::prompt_password(true);
        if (password.empty()) return 1;
        storage::init_storage(password);
        std::cout << "  [+] vault created\n";
        std::cout << "\n";
    } else {
        password = storage::prompt_password(false);
        if (!storage::verify_password(password)) {
            std::cerr << "  [!] invalid password\n";
            return 1;
        }
    }

    if (list_mode) {
        auto entries = storage::load_entries(password);
        if (entries.empty()) {
            std::cout << "  [*] no saved entries\n";
            return 0;
        }
        std::cout << "\n";
        for (size_t i = 0; i < entries.size(); ++i) {
            std::cout << "  [" << (i + 1) << "] " << entries[i].name;
            std::cout << " - " << entries[i].secret << " (" << entries[i].time_step << "s)\n";
        }
        std::cout << "\n";
        return 0;
    }

    if (delete_mode) {
        auto entries = storage::load_entries(password);
        if (entries.empty()) {
            std::cout << "  [*] no entries to delete\n";
            return 0;
        }
        int choice = select_entry(entries);
        if (choice == 0 || choice > static_cast<int>(entries.size())) return 0;
        entries.erase(entries.begin() + choice - 1);
        storage::save_entries(entries, password);
        std::cout << "  [+] entry deleted\n";
        return 0;
    }

    if (secret.empty()) {
        auto entries = storage::load_entries(password);
        if (entries.empty()) {
            std::cout << "  [*] no saved entries\n";
            std::cout << "  [*] usage: totp <secret> -n <name>\n";
            return 0;
        }

        std::cout << "\n";
        int choice = select_entry(entries);
        if (choice == 0 || choice > static_cast<int>(entries.size())) return 0;

        const auto& entry = entries[choice - 1];
        secret = entry.secret;
        time_step = entry.time_step;
        name = entry.name;
    } else {
        if (name.empty()) {
            name = secret.substr(0, 8) + "...";
        }

        totp::TOTPEntry entry{name, secret, time_step};
        storage::add_entry(entry, password);
        std::cout << "  [+] saved: " << name << "\n";
    }

    if (single_mode) {
        totp::TOTPResult result = totp::generate_totp(secret, time_step);
        std::cout << result.code << "\n";
        return 0;
    }

    totp::run_generator(secret, time_step);
    return 0;
}

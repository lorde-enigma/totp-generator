#include "totp.hpp"
#include <iostream>
#include <cstring>

void print_help(const char* program) {
    std::cout << "\n";
    std::cout << "  ┌─────────────────────────────────────┐\n";
    std::cout << "  │         TOTP CODE GENERATOR         │\n";
    std::cout << "  │     rfc 6238 compliant generator    │\n";
    std::cout << "  └─────────────────────────────────────┘\n";
    std::cout << "\n";
    std::cout << "  usage:\n";
    std::cout << "    " << program << " <secret> [options]\n";
    std::cout << "\n";
    std::cout << "  arguments:\n";
    std::cout << "    secret        base32 encoded totp secret key\n";
    std::cout << "\n";
    std::cout << "  options:\n";
    std::cout << "    -t, --time    time step in seconds (default: 30)\n";
    std::cout << "    -s, --single  generate single code and exit\n";
    std::cout << "    -h, --help    show this help message\n";
    std::cout << "\n";
    std::cout << "  examples:\n";
    std::cout << "    " << program << " CB6LUMRHEWPEYIXI\n";
    std::cout << "    " << program << " CB6LUMRHEWPEYIXI -t 60\n";
    std::cout << "    " << program << " CB6LUMRHEWPEYIXI --single\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    std::string secret;
    int time_step = 30;
    bool single_mode = false;

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
            } else {
                std::cerr << "  [!] error: -t requires a value\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--single") == 0) {
            single_mode = true;
        } else if (argv[i][0] != '-') {
            secret = argv[i];
        } else {
            std::cerr << "  [!] error: unknown option " << argv[i] << "\n";
            return 1;
        }
    }

    if (secret.empty()) {
        std::cerr << "  [!] error: secret key is required\n";
        print_help(argv[0]);
        return 1;
    }

    if (single_mode) {
        totp::TOTPResult result = totp::generate_totp(secret, time_step);
        std::cout << result.code << "\n";
        return 0;
    }

    totp::run_generator(secret, time_step);
    return 0;
}

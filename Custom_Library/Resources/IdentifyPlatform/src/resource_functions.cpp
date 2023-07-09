#include <iostream>
#include <unistd.h>
#include <cstring>

#include "resource_headers.h"

int getComputerIdentifier(const char* hostname) {
    if (std::strcmp(hostname, "spot-red") == 0) {
        return 1;
    } else if (std::strcmp(hostname, "spot-black") == 0) {
        return 2;
    } else if (std::strcmp(hostname, "spot-blue") == 0) {
        return 3;
    } else {
        return 0;  // No match found
    }
}

int WhoAmI() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        int identifier = getComputerIdentifier(hostname);
        return identifier;
    } else {
        std::cerr << "Failed to get the hostname." << std::endl;
        return 1;
    }

    return 0;
}

#include <iostream>
#include "minimal_build/ProfileManager.hpp"

int main() {
    // Get the ProfileManager instance
    auto& profileManager = openyolo::ProfileManager::instance();

    // Load existing profiles
    if (!profileManager.loadProfiles()) {
        std::cerr << "Failed to load profiles." << std::endl;
        return 1;
    }

    // Create a new profile
    if (!profileManager.createProfile("MyNewProfile")) {
        std::cerr << "Failed to create profile." << std::endl;
        return 1;
    }

    // Export the new profile
    if (auto err = profileManager.exportProfile("MyNewProfile", "MyNewProfile.json")) {
        std::cerr << "Failed to export profile: " << err.message() << std::endl;
        return 1;
    }

    std::cout << "Profile exported successfully." << std::endl;

    // Import the profile
    if (auto err = profileManager.importProfile("MyNewProfile.json")) {
        std::cerr << "Failed to import profile: " << err.message() << std::endl;
        return 1;
    }

    std::cout << "Profile imported successfully." << std::endl;

    return 0;
}

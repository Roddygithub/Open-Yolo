#include <gtest/gtest.h>

#include <fstream>

#include "minimal_build/ProfileManager.hpp"

class ProfileManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for profiles
        fs::create_directory("test_profiles");
    }

    void TearDown() override {
        // Remove the temporary directory
        fs::remove_all("test_profiles");
    }

    openyolo::ProfileManager& profileManager = openyolo::ProfileManager::instance();
};

TEST_F(ProfileManagerTest, CreateAndSaveProfile) {
    ASSERT_TRUE(profileManager.loadProfiles());
    ASSERT_TRUE(profileManager.createProfile("TestProfile"));
    ASSERT_TRUE(profileManager.saveCurrentProfile());

    // Check if the file was created
    ASSERT_TRUE(fs::exists("test_profiles/TestProfile.json"));
}

TEST_F(ProfileManagerTest, ExportAndImportProfile) {
    ASSERT_TRUE(profileManager.loadProfiles());
    ASSERT_TRUE(profileManager.createProfile("ExportTest"));

    // Export the profile
    ASSERT_FALSE(profileManager.exportProfile("ExportTest", "exported_profile.json"));

    // Import the profile
    ASSERT_FALSE(profileManager.importProfile("exported_profile.json"));

    // Check if the imported profile exists
    const auto& profiles = profileManager.getProfiles();
    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [](const openyolo::CursorProfile& p) { return p.name == "ExportTest"; });
    ASSERT_NE(it, profiles.end());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

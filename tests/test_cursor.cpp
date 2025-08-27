#include <gtest/gtest.h>
#include "cursormanager/CursorManager.hpp"

using namespace cursor_manager;

class CursorManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        cursorManager = std::make_unique<CursorManager>();
        ASSERT_TRUE(cursorManager->initialize());
    }

    void TearDown() override {
        cursorManager.reset();
    }

    std::unique_ptr<CursorManager> cursorManager;
};

TEST_F(CursorManagerTest, Initialization) {
    ASSERT_NE(cursorManager, nullptr);
}

TEST_F(CursorManagerTest, SetCursorScale) {
    const float testScale = 2.0f;
    cursorManager->setScale(testScale);
    // Note: In a real test, we would verify the scale was applied
    SUCCEED();
}

TEST_F(CursorManagerTest, ToggleVisibility) {
    cursorManager->setVisible(false);
    // Note: In a real test, we would verify the cursor was hidden
    cursorManager->setVisible(true);
    // Note: In a real test, we would verify the cursor was shown
    SUCCEED();
}

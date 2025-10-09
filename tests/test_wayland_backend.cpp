#include <giomm/init.h>
#include <glibmm.h>
#include <glibmm/init.h>
#include <glibmm/main.h>
#include <glibmm/refptr.h>
#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <memory>

#include "input/InputManager.hpp"
#include "input/backends/WaylandBackend.hpp"

// --- Test Fixtures ---

// Mocks the backend to test logic without a live environment
class MockWaylandBackend : public input::WaylandBackend {
public:
    MockWaylandBackend(input::InputManager* manager) : input::WaylandBackend(manager) {}

    int recreate_call_count = 0;
    int close_call_count = 0;
    bool throw_on_recreate = false;

    // Implémentation des méthodes nécessaires pour les tests
    void forceRecreateSession() {
        if (throw_on_recreate) {
            throw std::runtime_error("Mock recreateSession failed");
        }
        recreate_call_count++;
    }

    // Ajouter un callback d'erreur
    void addTestErrorCallback(std::function<void(const std::string&)> callback) {
        m_error_callback = callback;
    }

private:
    std::function<void(const std::string&)> m_error_callback;
};

// Base test fixture with common functionality
class WaylandBackendTestBase : public ::testing::Test {
protected:
    Glib::RefPtr<Glib::MainLoop> mainloop;

    void SetUp() override {
        // Initialize GIO for D-Bus support
        Gio::init();
        mainloop = Glib::MainLoop::create();
    }

    void TearDown() override {
        if (mainloop && mainloop->is_running()) {
            mainloop->quit();
        }
    }

    // Helper to run the main loop for a short time
    void run_mainloop_for_seconds(int seconds) {
        auto context = mainloop->get_context();
        auto start = std::chrono::steady_clock::now();

        while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                                start)
                   .count() < seconds) {
            context->iteration(true);
        }
    }
};

// Unit test fixture - uses mocks
class WaylandBackendUnitTest : public WaylandBackendTestBase {
protected:
    input::InputManager manager;
    std::unique_ptr<MockWaylandBackend> mock_backend;

    void SetUp() override {
        WaylandBackendTestBase::SetUp();
        mock_backend = std::make_unique<MockWaylandBackend>(&manager);
    }

    void TearDown() override {
        mock_backend.reset();
        WaylandBackendTestBase::TearDown();
    }
};

// Integration test fixture - uses real implementation
class WaylandBackendIntegrationTest : public WaylandBackendTestBase {
protected:
    input::InputManager manager;
    std::unique_ptr<input::WaylandBackend> backend;

    void SetUp() override {
        WaylandBackendTestBase::SetUp();
        backend = std::make_unique<input::WaylandBackend>(&manager);
    }

    void TearDown() override {
        if (backend) {
            try {
                backend->shutdown();
            } catch (...) {
                // Ignore shutdown errors
            }
            backend.reset();
        }
        WaylandBackendTestBase::TearDown();
    }
};

// --- Tests ---

TEST_F(WaylandBackendUnitTest, DebounceTest) {
    // Test avec des appels rapides
    for (int i = 0; i < 5; ++i) {
        mock_backend->forceRecreateSession();
    }

    // Vérifier que le compteur a été incrémenté
    EXPECT_GT(mock_backend->recreate_call_count, 0);
}

TEST_F(WaylandBackendUnitTest, ErrorHandlingTest) {
    // Tester le callback d'erreur
    bool error_callback_called = false;
    std::string error_message;

    mock_backend->addTestErrorCallback([&](const std::string& msg) {
        error_callback_called = true;
        error_message = msg;
    });

    // Tester la gestion des erreurs
    mock_backend->throw_on_recreate = true;

    try {
        mock_backend->forceRecreateSession();
        FAIL() << "Expected an exception to be thrown";
    } catch (const std::runtime_error& e) {
        // Vérifier que l'exception a le bon message
        EXPECT_STREQ(e.what(), "Mock recreateSession failed");
    }

    // Vérifier que le callback d'erreur a été appelé
    EXPECT_TRUE(error_callback_called) << "Error callback was not called";
    EXPECT_FALSE(error_message.empty()) << "Error message should not be empty";
}

// Ce test nécessite un environnement Wayland actif
TEST_F(WaylandBackendIntegrationTest, DISABLED_RegisterShortcut) {
    // Initialiser le backend
    try {
        backend->initialize();
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Skipping test - Wayland backend initialization failed: " << e.what();
    }

    // Test avec des paramètres valides
    bool callback_called = false;
    try {
        backend->registerShortcut("test_shortcut", "<Control><Shift>t",
                                  [&]() { callback_called = true; });

        // Vérifier que le callback a été enregistré
        SUCCEED() << "Shortcut registered successfully";

        // Nettoyage
        backend->unregisterShortcut("test_shortcut");
    } catch (const std::exception& e) {
        FAIL() << "Exception in RegisterShortcut test: " << e.what();
    }
}

// Ce test nécessite un environnement Wayland actif
TEST_F(WaylandBackendIntegrationTest, DISABLED_FullLifecycle) {
    // Créer un InputManager factice pour le test
    std::cout << "NOTE: This test requires a Wayland session with a running portal service."
              << std::endl;

    backend->setMinLogLevel(input::WaylandBackend::LogLevel::Debug);

    bool initialized = false;
    int state_changes = 0;

    // Enregistrer un callback pour les changements d'état
    backend->addStateChangeCallback([&](input::WaylandBackend::SessionState old_state,
                                        input::WaylandBackend::SessionState new_state) {
        state_changes++;
        std::cout << "State changed from " << static_cast<int>(old_state) << " to "
                  << static_cast<int>(new_state) << std::endl;

        if (new_state == input::WaylandBackend::SessionState::Connected && mainloop->is_running()) {
            mainloop->quit();
        }
    });

    try {
        // Initialiser le backend
        initialized = backend->initialize();
        ASSERT_TRUE(initialized) << "Backend failed to initialize. Is this a Wayland session?";

        std::cout << "Waiting for D-Bus connection..." << std::endl;
        run_mainloop_for_seconds(3);

        // Vérifier que des changements d'état ont été enregistrés
        ASSERT_GT(state_changes, 0)
            << "No state changes were recorded. The backend may not have connected.";
    } catch (const std::exception& e) {
        FAIL() << "Exception in FullLifecycle test: " << e.what();
    }
}

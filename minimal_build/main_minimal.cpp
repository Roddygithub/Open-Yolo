#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/frame.h>
#include <gtkmm/separator.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/entry.h>
#include <gtkmm/messagedialog.h>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include "Logger.hpp"
#include "ThemeManager.hpp"
#include "ProfileManager.hpp"

class MainWindow : public Gtk::Window {
public:
    MainWindow() {
        LOG_INFO("Création de la fenêtre principale");
        
        try {
            set_title("Open-Yolo - Gestionnaire de curseurs");
            set_default_size(1000, 700);
            
            // Créer un conteneur vertical principal
            Gtk::Box* mainBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));
            mainBox->set_border_width(10);
            
            // Cadre pour la section des thèmes
            Gtk::Frame* themeFrame = Gtk::manage(new Gtk::Frame("Thèmes de curseurs"));
            themeFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
            
            // Conteneur pour la section des thèmes
            Gtk::Box* themeBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));
            themeBox->set_border_width(10);
            
            // Charger les thèmes disponibles
            if (!openyolo::ThemeManager::instance().loadThemes()) {
                LOG_WARNING("Aucun thème de curseur trouvé");
            }
            
            // Sélecteur de thèmes
            Gtk::Box* themeSelectBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
            
            Gtk::Label* themeLabel = Gtk::manage(new Gtk::Label("Thème :"));
            themeSelectBox->pack_start(*themeLabel, Gtk::PACK_SHRINK);
            
            // Liste déroulante des thèmes
            m_themeCombo = Gtk::manage(new Gtk::ComboBoxText());
            const auto& themes = openyolo::ThemeManager::instance().getAvailableThemes();
            
            int currentIndex = -1;
            int index = 0;
            const auto* currentTheme = openyolo::ThemeManager::instance().getCurrentTheme();
            
            for (const auto& theme : themes) {
                m_themeCombo->append(theme.name);
                if (currentTheme && theme.name == currentTheme->name) {
                    currentIndex = index;
                }
                index++;
            }
            
            if (currentIndex >= 0) {
                m_themeCombo->set_active(currentIndex);
            } else if (!themes.empty()) {
                m_themeCombo->set_active(0);
            }
            
            themeSelectBox->pack_start(*m_themeCombo, Gtk::PACK_EXPAND_WIDGET);
            
            // Bouton d'application du thème
            m_applyButton = Gtk::manage(new Gtk::Button("Appliquer le thème"));
            m_applyButton->signal_clicked().connect(
                sigc::mem_fun(*this, &MainWindow::on_apply_theme_clicked));
            themeSelectBox->pack_start(*m_applyButton, Gtk::PACK_SHRINK);
            
            themeBox->pack_start(*themeSelectBox, Gtk::PACK_SHRINK);
            
            // Informations sur le thème sélectionné
            m_themeInfo = Gtk::manage(new Gtk::Label("Sélectionnez un thème pour voir ses détails"));
            m_themeInfo->set_line_wrap(true);
            m_themeInfo->set_halign(Gtk::ALIGN_START);
            m_themeInfo->set_valign(Gtk::ALIGN_START);
            m_themeInfo->set_margin_top(10);
            
            themeBox->pack_start(*m_themeInfo, Gtk::PACK_EXPAND_WIDGET);
            themeFrame->add(*themeBox);
            
            mainBox->pack_start(*themeFrame, Gtk::PACK_SHRINK);
            
            // Zone de logs
            Gtk::Frame* logFrame = Gtk::manage(new Gtk::Frame("Journal d'activité"));
            logFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
            
            // Zone de texte pour les logs
            m_logView = Gtk::manage(new Gtk::TextView());
            m_logView->set_editable(false);
            m_logView->set_wrap_mode(Gtk::WRAP_WORD);
            
            // Barre de défilement pour les logs
            m_scrolledWindow = Gtk::manage(new Gtk::ScrolledWindow());
            m_scrolledWindow->add(*m_logView);
            m_scrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
            m_scrolledWindow->set_size_request(-1, 200); // Hauteur fixe
            
            // Bouton pour effacer les logs
            Gtk::Button* clearButton = Gtk::manage(new Gtk::Button("Effacer les logs"));
            clearButton->signal_clicked().connect(
                sigc::mem_fun(*this, &MainWindow::on_clear_logs_clicked));
            
            Gtk::Box* logBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5));
            logBox->set_border_width(5);
logBox->pack_start(*m_scrolledWindow, Gtk::PACK_EXPAND_WIDGET);
            logBox->pack_start(*clearButton, Gtk::PACK_SHRINK, 5);
            
            logFrame->add(*logBox);
            mainBox->pack_start(*logFrame, Gtk::PACK_EXPAND_WIDGET);
            
            // Barre de statut
            m_statusBar = Gtk::manage(new Gtk::Statusbar());
            m_statusBar->set_margin_top(5);
            m_statusBar->push("Prêt");
            mainBox->pack_start(*m_statusBar, Gtk::PACK_SHRINK);
            
            add(*mainBox);
            
            // Mettre à jour les informations du thème sélectionné
            on_theme_changed();
            
            // Connecter le signal de changement de sélection
            m_themeCombo->signal_changed().connect(
                sigc::mem_fun(*this, &MainWindow::on_theme_changed));
            
            // Afficher tous les widgets
            show_all_children();
            
            LOG_INFO("Fenêtre principale initialisée avec succès");
            update_status("Application démarrée");
        } catch (const std::exception& e) {
            LOG_ERROR("Erreur lors de l'initialisation de la fenêtre: " + std::string(e.what()));
            throw;
        }
    }

private:
    // Mettre à jour la liste des profils dans la combobox
    void update_profile_combo() {
        if (!m_profileCombo) return;
        
        m_profileCombo->remove_all();
        const auto& profiles = openyolo::ProfileManager::instance().getProfiles();
        
        for (const auto& profile : profiles) {
            m_profileCombo->append(profile.name);
        }
        
        const auto* activeProfile = openyolo::ProfileManager::instance().getActiveProfile();
        if (activeProfile) {
            for (int i = 0; i < static_cast<int>(profiles.size()); ++i) {
                if (profiles[i].name == activeProfile->name) {
                    m_profileCombo->set_active(i);
                    m_currentProfileIndex = i;
                    break;
                }
            }
        }
    }
    
    // Mettre à jour la liste des thèmes dans la combobox
    void update_theme_combo() {
        if (!m_themeCombo) return;
        
        m_themeCombo->remove_all();
        const auto& themes = openyolo::ThemeManager::instance().getAvailableThemes();
        
        for (const auto& theme : themes) {
            m_themeCombo->append(theme.name);
        }
        
        // Sélectionner le thème du profil actif
        const auto* activeProfile = openyolo::ProfileManager::instance().getActiveProfile();
        if (activeProfile) {
            for (int i = 0; i < static_cast<int>(themes.size()); ++i) {
                if (themes[i].name == activeProfile->theme) {
                    m_themeCombo->set_active(i);
                    break;
                }
            }
        }
    }
    
    // Charger les paramètres du profil actif
    void load_current_profile() {
        const auto* profile = openyolo::ProfileManager::instance().getActiveProfile();
        if (!profile) return;
        
        // Mettre à jour l'interface avec les paramètres du profil
        update_theme_combo();
        
        // Mettre à jour les informations du thème
        on_theme_changed();
    }
    
    // Sauvegarder les paramètres dans le profil actif
    void save_current_profile() {
        auto& profileManager = openyolo::ProfileManager::instance();
        auto* profile = const_cast<openyolo::CursorProfile*>(profileManager.getActiveProfile());
        if (!profile) return;
        
        // Sauvegarder le thème sélectionné
        int themeIndex = m_themeCombo->get_active_row_number();
        if (themeIndex >= 0) {
            const auto& themes = openyolo::ThemeManager::instance().getAvailableThemes();
            if (themeIndex < static_cast<int>(themes.size())) {
                profile->theme = themes[themeIndex].name;
            }
        }
        
        // Sauvegarder le profil
        profileManager.saveCurrentProfile();
    }
    
    // Gestionnaire d'événements du changement de profil
    void on_profile_changed() {
        int index = m_profileCombo->get_active_row_number();
        if (index < 0 || index == m_currentProfileIndex) return;
        
        if (openyolo::ProfileManager::instance().setActiveProfile(index)) {
            m_currentProfileIndex = index;
            load_current_profile();
            update_status("Profil chargé : " + m_profileCombo->get_active_text());
        }
    }
    
    // Gestionnaire d'événements du bouton Nouveau profil
    void on_new_profile_clicked() {
        Gtk::Dialog dialog("Nouveau profil", *this);
        dialog.set_border_width(10);
        
        Gtk::Label label("Nom du nouveau profil :");
        Gtk::Entry entry;
        
        Gtk::Box* content = dialog.get_content_area();
        content->pack_start(label, Gtk::PACK_SHRINK);
        content->pack_start(entry, Gtk::PACK_SHRINK);
        
        dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Créer", Gtk::RESPONSE_OK);
        
        dialog.show_all_children();
        
        while (true) {
            int result = dialog.run();
            if (result != Gtk::RESPONSE_OK) {
                break;
            }
            
            std::string name = entry.get_text();
            if (name.empty()) {
                Gtk::MessageDialog error_dialog("Le nom du profil ne peut pas être vide", false, 
                                              Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                error_dialog.run();
                continue;
            }
            
            if (openyolo::ProfileManager::instance().createProfile(name)) {
                update_profile_combo();
                
                // Sélectionner le nouveau profil
                const auto& profiles = openyolo::ProfileManager::instance().getProfiles();
                for (int i = 0; i < static_cast<int>(profiles.size()); ++i) {
                    if (profiles[i].name == name) {
                        m_profileCombo->set_active(i);
                        m_currentProfileIndex = i;
                        break;
                    }
                }
                
                update_status("Nouveau profil créé : " + name);
            } else {
                Gtk::MessageDialog error_dialog("Impossible de créer le profil. Un profil avec ce nom existe peut-être déjà.", 
                                              false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                error_dialog.run();
                continue;
            }
            
            break;
        }
    }
    
    // Gestionnaire d'événements du bouton Supprimer profil
    void on_delete_profile_clicked() {
        int index = m_profileCombo->get_active_row_number();
        if (index < 0) return;
        
        const auto& profiles = openyolo::ProfileManager::instance().getProfiles();
        if (index >= static_cast<int>(profiles.size())) return;
        
        std::string name = profiles[index].name;
        
        // Ne pas permettre la suppression du dernier profil
        if (profiles.size() <= 1) {
            Gtk::MessageDialog dialog(*this, "Impossible de supprimer le dernier profil", false, 
                                    Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
            dialog.set_secondary_text("Vous devez avoir au moins un profil.");
            dialog.run();
            return;
        }
        
        // Demander confirmation
        Gtk::MessageDialog dialog(*this, "Confirmer la suppression", false, 
                                 Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
        dialog.set_secondary_text("Voulez-vous vraiment supprimer le profil \"" + name + "\" ?");
        
        if (dialog.run() == Gtk::RESPONSE_YES) {
            if (openyolo::ProfileManager::instance().deleteProfile(index)) {
                update_profile_combo();
                update_status("Profil supprimé : " + name);
                
                // S'assurer qu'un profil est sélectionné
                if (m_profileCombo->get_active_row_number() < 0 && 
                    m_profileCombo->get_model()->get_n_items() > 0) {
                    m_profileCombo->set_active(0);
                }
            } else {
                Gtk::MessageDialog error_dialog(*this, "Erreur lors de la suppression du profil", 
                                              false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                error_dialog.run();
            }
        }
    }
    
    // Mettre à jour les informations du thème sélectionné
    void on_theme_changed() {
        int active = m_themeCombo->get_active_row_number();
        if (active < 0) {
            m_themeInfo->set_text("Aucun thème sélectionné");
            return;
        }
        
        const auto& themes = openyolo::ThemeManager::instance().getAvailableThemes();
        if (active >= static_cast<int>(themes.size())) {
            return;
        }
        
        const auto& theme = themes[active];
        
        std::string info = "<b>" + theme.name + "</b>\n";
        
        if (!theme.comment.empty()) {
            info += "Description : " + theme.comment + "\n";
        }
        
        info += "Emplacement : " + theme.path + "\n";
        info += "Type : " + std::string(theme.isSystemTheme ? "Système" : "Utilisateur");
        
        m_themeInfo->set_markup(info);
        update_status("Thème sélectionné : " + theme.name);
        
        // Mettre à jour le profil actif
        save_current_profile();
    }
    
    // Gestionnaire d'événements du bouton d'application du thème
    void on_apply_theme_clicked() {
        // Sauvegarder d'abord le profil actuel
        save_current_profile();
        
        // Obtenir le thème sélectionné
        int active = m_themeCombo->get_active_row_number();
        if (active < 0) {
            update_status("Aucun thème sélectionné", "error");
            return;
        }
        
        const auto& themes = openyolo::ThemeManager::instance().getAvailableThemes();
        if (active >= static_cast<int>(themes.size())) {
            return;
        }
        
        const auto& theme = themes[active];
        
        try {
            if (openyolo::ThemeManager::instance().applyTheme(theme.name)) {
                std::string msg = "Thème appliqué avec succès : " + theme.name;
                LOG_INFO(msg);
                update_status(msg, "success");
                
                // Mettre à jour le profil actif avec le nouveau thème
                auto& profileManager = openyolo::ProfileManager::instance();
                auto* profile = const_cast<openyolo::CursorProfile*>(profileManager.getActiveProfile());
                if (profile) {
                    profile->theme = theme.name;
                    profileManager.saveCurrentProfile();
                }
            } else {
                std::string msg = "Échec de l'application du thème : " + theme.name;
                LOG_ERROR(msg);
                update_status(msg, "error");
            }
        } catch (const std::exception& e) {
            std::string msg = "Erreur lors de l'application du thème : " + std::string(e.what());
            LOG_ERROR(msg);
            update_status(msg, "error");
        }
    }
    
    // Gestionnaire d'événements du bouton d'effacement des logs
    void on_clear_logs_clicked() {
        auto buffer = m_logView->get_buffer();
        if (buffer) {
            buffer->set_text("");
        }
    }
    
    // Mettre à jour la barre de statut
    void update_status(const std::string& message, const std::string& type = "info") {
        if (!m_statusBar) return;
        
        m_statusBar->pop();
        m_statusBar->push(message);
        
        // Ajouter le message aux logs
        auto buffer = m_logView->get_buffer();
        if (buffer) {
            auto iter = buffer->end();
            std::string prefix;
            
            if (type == "error") {
                prefix = "[ERREUR] ";
            } else if (type == "success") {
                prefix = "[SUCCÈS] ";
            } else {
                prefix = "[INFO] ";
            }
            
            buffer->insert(iter, prefix + message + "\n");
            
            // Faire défiler vers le bas
            auto mark = buffer->create_mark("end", buffer->end());
            m_logView->scroll_to(mark);
        }
    }
    
    // Wprivate:
    // Éléments de l'interface
    Gtk::ComboBoxText* m_themeCombo = nullptr;
    Gtk::ComboBoxText* m_profileCombo = nullptr;
    Gtk::Button* m_applyButton = nullptr;
    Gtk::Button* m_newProfileButton = nullptr;
    Gtk::Button* m_deleteProfileButton = nullptr;
    Gtk::Label* m_themeInfo = nullptr;
    Gtk::ScrolledWindow* m_scrolledWindow = nullptr;
    Gtk::TextView* m_logView = nullptr;
    Gtk::Statusbar* m_statusBar = nullptr;
    
    // Données
    std::vector<openyolo::CursorProfile> m_profiles;
    int m_currentProfileIndex = -1;
    
    // Méthodes utilitaires
    void update_profile_combo();
    void update_theme_combo();
    void load_current_profile();
    void save_current_profile();
    void on_profile_changed();
    void on_new_profile_clicked();
    void on_delete_profile_clicked();
    
    // Méthodes existantes
    void on_theme_changed();
    void on_apply_theme_clicked();
    void on_clear_logs_clicked();
    void update_status(const std::string&message, const std::string& type = "info");
    
    // Méthodes manquantes
    void on_profile_combo_changed();
    void on_new_profile_dialog_response(Gtk::Dialog* dialog, int response_id);
    void on_delete_profile_dialog_response(Gtk::Dialog* dialog, int response_id);
};

int main(int argc, char* argv[]) {
    try {
        // Initialiser le logger
{{ ... }}
        LOG_INFO("Démarrage de l'application Open-Yolo");
        
        // Créer l'application
        auto app = Gtk::Application::create("org.openyolo.minimal");
        
        // Créer et afficher la fenêtre principale
        MainWindow window;
        
        LOG_INFO("Lancement de la boucle d'événements");
        
        // Exécuter l'application
        int result = app->run(window, argc, argv);
        
        LOG_INFO("Fermeture de l'application avec le code: " + std::to_string(result));
        return result;
        
    } catch (const std::exception& e) {
        LOG_FATAL("Erreur fatale: " + std::string(e.what()));
        return 1;
    } catch (...) {
        LOG_FATAL("Erreur fatale inconnue");
        return 1;
    }
}

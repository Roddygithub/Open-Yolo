#include "gui/MainWindow.hpp"
#include <gtkmm/application.h>
#include <gtkmm/dialog.h>
#include <iostream>
#include <sstream>

MainWindow::MainWindow(
    std::shared_ptr<cursor_manager::CursorManager> cursorManager,
    std::shared_ptr<DisplayManager> displayManager
) : cursorManager_(cursorManager),
    displayManager_(displayManager),
    mainBox_(Gtk::ORIENTATION_VERTICAL) {
    
    // Initialisation des membres de cursorTab_
    cursorTab_.fpsSpin = Gtk::make_managed<Gtk::SpinButton>(Gtk::Adjustment::create(60.0, 1.0, 144.0, 1.0, 10.0, 0.0));
    cursorTab_.scaleSlider = Gtk::make_managed<Gtk::Scale>(Gtk::ORIENTATION_HORIZONTAL, Gtk::Adjustment::create(1.0, 0.1, 3.0, 0.1, 0.5, 0.0));
    
    // Configuration de la fenêtre principale
    set_title("Open-Yolo - Gestionnaire de curseurs");
    set_default_size(800, 600);
    
    // Configuration des widgets
    setupWidgets();
    connectSignals();
    
    // Charger les paramètres
    loadSettings();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupWidgets() {
    // Configuration de la boîte principale
    set_child(mainBox_);
    mainBox_.set_margin(10);
    mainBox_.set_spacing(10);
    
    // Création du notebook pour les onglets
    notebook_ = Gtk::make_managed<Gtk::Notebook>();
    mainBox_.append(*notebook_);
    
    // Création des onglets
    auto cursorBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    auto shortcutsBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    auto displayBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    auto aboutBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    
    // Configuration des onglets
    setupCursorTab(cursorBox);
    setupShortcutsTab(shortcutsBox);
    setupDisplayTab(displayBox);
    setupAboutTab(aboutBox);
    
    // Ajout des onglets au notebook
    notebook_->append_page(*cursorBox, "Curseur");
    notebook_->append_page(*shortcutsBox, "Raccourcis");
    notebook_->append_page(*displayBox, "Affichage");
    notebook_->append_page(*aboutBox, "À propos");
    
    // Configuration de la barre d'état
    setupStatusBar();
}

void MainWindow::connectSignals() {
    // Connexion des signaux de la fenêtre
    signal_close_request().connect(
        sigc::mem_fun(*this, &MainWindow::on_close_request), false);
    
    // Connexion des signaux des onglets
    if (cursorTab_.fpsSpin) {
        cursorTab_.fpsSpin->signal_value_changed().connect(
            sigc::mem_fun(*this, &MainWindow::onFpsChanged));
    }
    
    if (cursorTab_.scaleSlider) {
        cursorTab_.scaleSlider->signal_value_changed().connect(
            sigc::mem_fun(*this, &MainWindow::onScaleChanged));
    }
}

void MainWindow::onCursorSelected() {
    if (!cursorTab_.fileChooserBtn) return;
    
    auto dialog = Gtk::FileChooserNative::create(
        "Sélectionner un curseur",
        *this,
        Gtk::FileChooser::Action::OPEN,
        "Sélectionner",
        "Annuler"
    );
    
    // Configuration du filtre pour les images
    auto filter = Gtk::FileFilter::create();
    filter->set_name("Images");
    filter->add_mime_type("image/*");
    dialog->add_filter(filter);
    
    // Affichage de la boîte de dialogue
    dialog->signal_response().connect([this, dialog](int response_id) {
        if (response_id == Gtk::ResponseType::ACCEPT) {
            auto filename = dialog->get_file()->get_path();
            if (!filename.empty()) {
                currentCursorPath_ = filename;
                updateStatus("Curseur sélectionné : " + filename);
                onCursorSettingsChanged();
            }
        }
    });
    
    dialog->show();
}

void MainWindow::onCursorFileSelected() {
    try {
        // Créer un sélecteur de fichiers
        auto dialog = Gtk::FileChooserNative::create(
            "Sélectionner un curseur",
            *this,
            Gtk::FileChooser::Action::OPEN,
            "Sélectionner",
            "Annuler"
        );
        
        // Filtrer les fichiers par extension
        auto filter = Gtk::FileFilter::create();
        filter->set_name("Fichiers de curseurs");
        filter->add_pattern("*.png");
        filter->add_pattern("*.svg");
        filter->add_pattern("*.cur");
        filter->add_pattern("*.ani");
        dialog->add_filter(filter);
        
        // Afficher la boîte de dialogue
        dialog->set_modal(true);
        dialog->signal_response().connect(
            [this, dialog](int response_id) {
                if (response_id == Gtk::ResponseType::ACCEPT) {
                    std::string filename = dialog->get_file()->get_path();
                    
                    // Vérifier que le fichier existe
                    if (Glib::file_test(filename, Glib::FileTest::EXISTS)) {
                        // Mettre à jour le chemin du curseur
                        currentCursorPath_ = filename;
                        
                        // Mettre à jour l'aperçu
                        updateCursorPreview();
                        
                        // Mettre à jour le statut
                        updateStatus("Curseur sélectionné: " + Glib::path_get_basename(filename));
                    } else {
                        showError("Le fichier sélectionné n'existe pas");
                    }
                }
                
                // Libérer les ressources
                dialog->hide();
            }
        );
        
        dialog->show();
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la sélection du fichier: " << e.what() << std::endl;
        showError("Impossible de sélectionner le fichier");
    }
}

void MainWindow::onApplyClicked() {
    try {
        // Vérifier qu'un curseur est sélectionné
        if (currentCursorPath_.empty()) {
            showError("Veuillez sélectionner un curseur");
            return;
        }
        
        // Appliquer les paramètres
        applySettings();
        
        // Si un gestionnaire de curseur est disponible, charger le nouveau curseur
        if (cursorManager_) {
            try {
                cursorManager_->loadFromFile(currentCursorPath_);
                updateStatus("Curseur appliqué avec succès");
            } catch (const std::exception& e) {
                std::cerr << "Erreur lors du chargement du curseur: " << e.what() << std::endl;
                showError("Impossible de charger le curseur: " + std::string(e.what()));
            }
        }
        
        // Sauvegarder les paramètres
        saveSettings();
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'application des modifications: " << e.what() << std::endl;
        showError("Impossible d'appliquer les modifications");
    }
}

bool MainWindow::on_close_request() {
    if (settingsModified_) {
        auto dialog = Gtk::MessageDialog(
            *this,
            "Enregistrer les modifications ?",
            false,
            Gtk::MessageType::QUESTION,
            Gtk::ButtonsType::YES_NO,
            true
        );
        
        dialog.set_secondary_text("Des modifications non enregistrées seront perdues.");
        dialog.add_button("Annuler", Gtk::ResponseType::CANCEL);
        
        int response = dialog.run();
        
        if (response == Gtk::ResponseType::YES) {
            saveSettings();
        } else if (response == Gtk::ResponseType::CANCEL) {
            return true; // Empêcher la fermeture
        }
    }
    
    // Autoriser la fermeture de la fenêtre
    return false;
}

void MainWindow::onFpsChanged() {
    if (!cursorTab_.fpsSpin) return;
    int fps = static_cast<int>(cursorTab_.fpsSpin->get_value());
    updateStatus("FPS défini à : " + std::to_string(fps));
    settingsModified_ = true;
}

void MainWindow::onScaleChanged() {
    if (!cursorTab_.scaleSlider) return;
    double scale = cursorTab_.scaleSlider->get_value();
    updateStatus("Échelle définie à : " + std::to_string(scale));
    settingsModified_ = true;
}

void MainWindow::updateStatus(const std::string& message) {
    statusBar_.pop(); // Effacer le message précédent
    statusBar_.push(message);
}

void MainWindow::showError(const std::string& message) {
    auto dialog = Gtk::MessageDialog(
        *this,
        "Erreur",
        false,
        Gtk::MessageType::ERROR,
        Gtk::ButtonsType::OK,
        true
    );
    dialog.set_secondary_text(message);
    dialog.run();
}

void MainWindow::showLicenseDialog() {
    try {
        // Créer une boîte de dialogue modale
        Gtk::Dialog dialog("Licence - Open-Yolo", *this);
        dialog.set_default_size(500, 400);
        
        // Zone de défilement pour le texte de la licence
        Gtk::ScrolledWindow scrolledWindow;
        scrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
        
        // Zone de texte pour afficher la licence
        Gtk::TextView textView;
        textView.set_editable(false);
        textView.set_cursor_visible(false);
        textView.set_wrap_mode(Gtk::WRAP_WORD);
        
        // Charger le texte de la licence
        std::string licenseText = 
            "Open-Yolo - Gestionnaire de curseurs personnalisables\n\n"
            "Copyright 2023 Équipe Open-Yolo\n\n"
            "Ce programme est un logiciel libre ; vous pouvez le redistribuer ou le modifier\n"
            "selon les termes de la Licence Publique Générale GNU telle que publiée par\n"
            "la Free Software Foundation ; soit la version 3 de la Licence, soit\n"
            "(à votre choix) toute version ultérieure.\n\n"
            "Ce programme est distribué dans l'espoir qu'il sera utile,\n"
            "mais SANS AUCUNE GARANTIE ; sans même la garantie implicite de\n"
            "QUALITÉ MARCHANDE ou d'ADÉQUATION À UN USAGE PARTICULIER.\n"
            "Consultez la Licence Publique Générale GNU pour plus de détails.\n\n"
            "Vous devriez avoir reçu une copie de la Licence Publique Générale GNU\n"
            "avec ce programme ; si ce n'est pas le cas, consultez :\n"
            "<http://www.gnu.org/licenses/>.";
        
        textView.get_buffer()->set_text(licenseText);
        
        // Configurer la zone de défilement
        scrolledWindow.set_child(textView);
        
        // Bouton de fermeture
        dialog.add_button("Fermer", Gtk::ResponseType::OK);
        
        // Ajouter la zone de défilement à la boîte de dialogue
        dialog.get_content_area()->append(scrolledWindow);
        
        // Afficher la boîte de dialogue
        dialog.set_modal(true);
        dialog.show();
        dialog.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'affichage de la licence: " << e.what() << std::endl;
        showError("Impossible d'afficher la licence");
    }
}

// Méthodes à implémenter
void MainWindow::setupCursorTab(Gtk::Box* parent) {
    parent->set_orientation(Gtk::ORIENTATION_VERTICAL);
    parent->set_spacing(10);
    parent->set_margin(10);
    
    // Groupe pour les paramètres du curseur
    auto frame = Gtk::make_managed<Gtk::Frame>("Paramètres du curseur");
    auto content = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    content->set_margin(10);
    frame->set_child(*content);
    
    // Sélection de fichier
    auto fileBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    auto fileLabel = Gtk::make_managed<Gtk::Label>("Fichier du curseur:");
    cursorTab_.fileChooserBtn = Gtk::make_managed<Gtk::Button>("Sélectionner...");
    cursorTab_.fileChooserBtn->signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::onCursorFileSelected));
    
    fileBox->append(*fileLabel);
    fileBox->append(*cursorTab_.fileChooserBtn);
    content->append(*fileBox);
    
    // Aperçu du curseur
    cursorTab_.previewArea = Gtk::make_managed<Gtk::DrawingArea>();
    cursorTab_.previewArea->set_size_request(64, 64);
    cursorTab_.previewArea->set_draw_func(
        sigc::mem_fun(*this, &MainWindow::onCursorPreviewDraw));
    content->append(*cursorTab_.previewArea);
    
    // Contrôles de configuration
    auto scaleBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    auto scaleLabel = Gtk::make_managed<Gtk::Label>("Échelle:");
    cursorTab_.scaleSlider->set_hexpand(true);
    cursorTab_.scaleSlider->set_range(0.1, 5.0);
    cursorTab_.scaleSlider->set_increments(0.1, 0.5);
    cursorTab_.scaleSlider->set_value(1.0);
    
    scaleBox->append(*scaleLabel);
    scaleBox->append(*cursorTab_.scaleSlider);
    content->append(*scaleBox);
    
    // Contrôle FPS
    auto fpsBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    auto fpsLabel = Gtk::make_managed<Gtk::Label>("Images par seconde:");
    cursorTab_.fpsSpin->set_range(1, 120);
    cursorTab_.fpsSpin->set_increments(1, 10);
    cursorTab_.fpsSpin->set_value(60);
    
    fpsBox->append(*fpsLabel);
    fpsBox->append(*cursorTab_.fpsSpin);
    content->append(*fpsBox);
    
    // Boutons d'action
    auto buttonBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    buttonBox->set_halign(Gtk::Align::END);
    
    auto applyBtn = Gtk::make_managed<Gtk::Button>("Appliquer");
    applyBtn->signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::onApplyClicked));
    
    auto cancelBtn = Gtk::make_managed<Gtk::Button>("Annuler");
    cancelBtn->signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::onCancelClicked));
    
    buttonBox->append(*cancelBtn);
    buttonBox->append(*applyBtn);
    content->append(*buttonBox);
    
    parent->append(*frame);
}

void MainWindow::setupShortcutsTab(Gtk::Box* parent) {
    parent->set_orientation(Gtk::ORIENTATION_VERTICAL);
    parent->set_spacing(10);
    parent->set_margin(10);
    
    auto frame = Gtk::make_managed<Gtk::Frame>("Raccourcis clavier");
    auto content = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    content->set_margin(10);
    frame->set_child(*content);
    
    // Création du modèle de données
    shortcutsTab_.listStore = Gtk::ListStore::create(shortcutsTab_.columns);
    
    // Création de la vue en arborescence
    shortcutsTab_.treeView = Gtk::make_managed<Gtk::TreeView>(shortcutsTab_.listStore);
    shortcutsTab_.treeView->set_headers_visible(true);
    
    // Colonne Description
    auto descCol = Gtk::make_managed<Gtk::TreeViewColumn>("Action");
    auto descRenderer = Gtk::make_managed<Gtk::CellRendererText>();
    descCol->pack_start(*descRenderer, true);
    descCol->add_attribute(descRenderer->property_text(), 
                          shortcutsTab_.columns.description);
    shortcutsTab_.treeView->append_column(*descCol);
    
    // Colonne Raccourci
    auto accelCol = Gtk::make_managed<Gtk::TreeViewColumn>("Raccourci");
    auto accelRenderer = Gtk::make_managed<Gtk::CellRendererAccel>(
        Gtk::CellRendererAccel::mode::GTK_CELL_RENDERER_MODE_EDITABLE);
    
    accelCol->pack_start(*accelRenderer, true);
    accelCol->set_cell_data_func(*accelRenderer, [this](
        Gtk::TreeViewColumn*, Gtk::CellRenderer* renderer,
        const Gtk::TreeModel::iterator& iter) {
        auto cell = dynamic_cast<Gtk::CellRendererAccel*>(renderer);
        if (!cell) return;
        
        // Configurer le rendu du raccourci
        // (l'implémentation complète nécessiterait plus de logique)
    });
    
    shortcutsTab_.treeView->append_column(*accelCol);
    
    // Configuration du défilement
    auto scrolledWindow = Gtk::make_managed<Gtk::ScrolledWindow>();
    scrolledWindow->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrolledWindow->set_vexpand(true);
    scrolledWindow->set_child(*shortcutsTab_.treeView);
    
    content->append(*scrolledWindow);
    parent->append(*frame);
    
    // Ajout des raccourcis par défaut
    addDefaultShortcuts();
}

void MainWindow::setupDisplayTab(Gtk::Box* parent) {
    parent->set_orientation(Gtk::ORIENTATION_VERTICAL);
    parent->set_spacing(10);
    parent->set_margin(10);
    
    // Groupe pour les paramètres d'affichage
    auto frame = Gtk::make_managed<Gtk::Frame>("Paramètres d'affichage");
    auto content = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    content->set_margin(10);
    frame->set_child(*content);
    
    // Liste des écrans
    auto displayFrame = Gtk::make_managed<Gtk::Frame>("Écrans disponibles");
    auto displayContent = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    displayContent->set_margin(5);
    
    // Création du modèle de données
    displayTab_.displayList = Gtk::ListStore::create(displayTab_.columns);
    
    // Création de la vue en arborescence
    displayTab_.displayTreeView = Gtk::make_managed<Gtk::TreeView>(displayTab_.displayList);
    displayTab_.displayTreeView->set_headers_visible(true);
    
    // Colonnes du tableau
    auto addColumn = [this](const Glib::ustring& title, const Gtk::TreeModelColumnBase& column) {
        auto col = Gtk::make_managed<Gtk::TreeViewColumn>(title, column);
        displayTab_.displayTreeView->append_column(*col);
        return col;
    };
    
    // Colonne de sélection
    auto selectCol = Gtk::make_managed<Gtk::TreeViewColumn>("Sélection");
    auto toggleRenderer = Gtk::make_managed<Gtk::CellRendererToggle>();
    toggleRenderer->property_activatable() = true;
    toggleRenderer->signal_toggled().connect(
        sigc::mem_fun(*this, &MainWindow::onDisplayToggled));
    selectCol->pack_start(*toggleRenderer, false);
    selectCol->add_attribute(toggleRenderer->property_active(), 
                           displayTab_.columns.isPrimary);
    displayTab_.displayTreeView->append_column(*selectCol);
    
    // Autres colonnes
    addColumn("Écran", displayTab_.columns.name);
    addColumn("Résolution", displayTab_.columns.resolution);
    
    // Colonne d'échelle avec rendu personnalisé
    auto scaleCol = Gtk::make_managed<Gtk::TreeViewColumn>("Échelle");
    auto scaleRenderer = Gtk::make_managed<Gtk::CellRendererText>();
    scaleCol->pack_start(*scaleRenderer, true);
    scaleCol->set_cell_data_func(*scaleRenderer, 
        sigc::mem_fun(*this, &MainWindow::onScaleCellData));
    displayTab_.displayTreeView->append_column(*scaleCol);
    
    // Configuration du défilement
    auto scrolledWindow = Gtk::make_managed<Gtk::ScrolledWindow>();
    scrolledWindow->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrolledWindow->set_vexpand(true);
    scrolledWindow->set_child(*displayTab_.displayTreeView);
    
    displayContent->append(*scrolledWindow);
    displayFrame->set_child(*displayContent);
    
    // Options supplémentaires
    displayTab_.followMouseCheck = Gtk::make_managed<Gtk::CheckButton>("Suivre la souris");
    displayTab_.followMouseCheck->signal_toggled().connect(
        sigc::mem_fun(*this, &MainWindow::onDisplaySettingsChanged));
    
    // Assemblage de l'interface
    content->append(*displayFrame);
    content->append(*displayTab_.followMouseCheck);
    parent->append(*frame);
    
    // Mise à jour de la liste des écrans
    updateDisplayList();
}

void MainWindow::setupAboutTab(Gtk::Box* parent) {
    parent->set_orientation(Gtk::ORIENTATION_VERTICAL);
    parent->set_spacing(10);
    parent->set_margin(10);
    
    // En-tête
    auto title = Gtk::make_managed<Gtk::Label>("Open-Yolo");
    auto titleFont = title->get_pango_context()->get_font_description();
    titleFont->set_size(24 * PANGO_SCALE);
    titleFont->set_weight(Pango::Weight::BOLD);
    title->override_font(Pango::FontDescription(titleFont));
    
    // Version
    auto version = Gtk::make_managed<Gtk::Label>("Version 1.0.0");
    
    // Description
    auto desc = Gtk::make_managed<Gtk::Label>(
        "Un gestionnaire de curseurs personnalisables pour Linux");
    desc->set_justify(Gtk::Justification::CENTER);
    
    // Bouton de licence
    auto licenseBtn = Gtk::make_managed<Gtk::Button>("Licence");
    licenseBtn->signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::showLicenseDialog));
    
    // Crédits
    auto credits = Gtk::make_managed<Gtk::Label>(
        " 2023 Équipe Open-Yolo\n"
        "Sous licence GPL-3.0");
    credits->set_justify(Gtk::Justification::CENTER);
    
    // Assemblage
    parent->append(*title);
    parent->append(*version);
    parent->append(*Gtk::make_managed<Gtk::Separator>());
    parent->append(*desc);
    parent->append(*Gtk::make_managed<Gtk::Separator>());
    parent->append(*licenseBtn);
    parent->append(*credits);
    
    // Style
    parent->set_valign(Gtk::Align::CENTER);
    parent->set_halign(Gtk::Align::CENTER);
}

void MainWindow::setupStatusBar() {
    // Configuration de la barre d'état
    mainBox_.append(statusBar_);
}

void MainWindow::applySettings() {
    try {
        // Appliquer les paramètres du curseur
        if (cursorManager_) {
            // Mettre à jour l'échelle du curseur si nécessaire
            if (cursorTab_.scaleSlider) {
                double scale = cursorTab_.scaleSlider->get_value();
                cursorManager_->setScale(scale);
            }
            
            // Mettre à jour les FPS si nécessaire
            if (cursorTab_.fpsSpin) {
                int fps = cursorTab_.fpsSpin->get_value_as_int();
                cursorManager_->setAnimationSpeed(fps);
            }
        }
        
        // Enregistrer les paramètres
        saveSettings();
        
        // Mettre à jour l'interface
        updateStatus("Paramètres appliqués avec succès");
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'application des paramètres: " << e.what() << std::endl;
        showError("Impossible d'appliquer les paramètres: " + std::string(e.what()));
    }
}

void MainWindow::loadSettings() {
    try {
        // Ici, on pourrait charger les paramètres depuis un fichier de configuration
        // Pour l'instant, on utilise des valeurs par défaut
        
        // Paramètres du curseur
        if (cursorTab_.scaleSlider) {
            cursorTab_.scaleSlider->set_value(1.0);
        }
        
        if (cursorTab_.fpsSpin) {
            cursorTab_.fpsSpin->set_value(60);
        }
        
        // Paramètres d'affichage
        if (displayTab_.followMouseCheck) {
            displayTab_.followMouseCheck->set_active(false);
        }
        
        // Mettre à jour l'interface
        updateDisplayList();
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement des paramètres: " << e.what() << std::endl;
        showError("Impossible de charger les paramètres: " + std::string(e.what()));
    }
}

void MainWindow::saveSettings() {
    try {
        // Ici, on pourrait sauvegarder les paramètres dans un fichier de configuration
        // Par exemple, en utilisant GKeyFile ou une autre méthode de sérialisation
        
        // Pour l'instant, on se contente de mettre à jour l'état actuel
        if (cursorManager_) {
            if (cursorTab_.scaleSlider) {
                double scale = cursorTab_.scaleSlider->get_value();
                // Sauvegarder l'échelle
            }
            
            if (cursorTab_.fpsSpin) {
                int fps = cursorTab_.fpsSpin->get_value_as_int();
                // Sauvegarder les FPS
            }
        }
        
        // Mettre à jour le statut
        updateStatus("Paramètres enregistrés");
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la sauvegarde des paramètres: " << e.what() << std::endl;
        showError("Impossible de sauvegarder les paramètres: " + std::string(e.what()));
    }
}

void MainWindow::onCursorSettingsChanged() {
    try {
        if (!cursorManager_) return;
        
        // Mettre à jour l'échelle du curseur
        if (cursorTab_.scaleSlider) {
            double scale = cursorTab_.scaleSlider->get_value();
            cursorManager_->setScale(scale);
        }
        
        // Mettre à jour la vitesse d'animation
        if (cursorTab_.fpsSpin) {
            int fps = cursorTab_.fpsSpin->get_value_as_int();
            cursorManager_->setAnimationSpeed(fps);
        }
        
        // Mettre à jour l'aperçu du curseur
        updateCursorPreview();
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour des paramètres du curseur: " << e.what() << std::endl;
        showError("Impossible de mettre à jour les paramètres du curseur");
    }
}

void MainWindow::onDisplaySettingsChanged() {
    try {
        if (!displayManager_) return;
        
        // Mettre à jour le suivi de la souris
        if (displayTab_.followMouseCheck) {
            bool followMouse = displayTab_.followMouseCheck->get_active();
            displayManager_->setFollowMouse(followMouse);
        }
        
        // Mettre à jour les paramètres d'affichage
        if (displayTab_.displayTreeView) {
            auto selection = displayTab_.displayTreeView->get_selection();
            if (selection) {
                Gtk::TreeModel::iterator iter = selection->get_selected();
                if (iter) {
                    Gtk::TreeModel::Row row = *iter;
                    // Mettre à jour l'affichage avec les paramètres sélectionnés
                    // (à implémenter selon les besoins spécifiques)
                }
            }
        }
        
        updateStatus("Paramètres d'affichage mis à jour");
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour des paramètres d'affichage: " << e.what() << std::endl;
        showError("Impossible de mettre à jour les paramètres d'affichage");
    }
}

void MainWindow::onShortcutEdited(const Glib::ustring& path, guint keyval, 
                                 Gdk::ModifierType modifiers, guint hardware_keycode) {
    try {
        if (!inputManager_) {
            std::cerr << "InputManager non initialisé" << std::endl;
            return;
        }
        
        // Convertir le chemin en itérateur
        Gtk::TreeModel::iterator iter = shortcutsTab_.listStore->get_iter(path);
        if (!iter) {
            std::cerr << "Élément non trouvé dans la liste des raccourcis" << std::endl;
            return;
        }
        
        Gtk::TreeModel::Row row = *iter;
        std::string name = row[shortcutsTab_.columns.name];
        
        // Vérifier si la combinaison de touches est déjà utilisée
        for (const auto& item : shortcutsTab_.listStore->children()) {
            if (item[shortcutsTab_.columns.keyval] == keyval && 
                item[shortcutsTab_.columns.modifiers] == modifiers) {
                showError("Cette combinaison de touches est déjà utilisée");
                return;
            }
        }
        
        // Mettre à jour le modèle
        row[shortcutsTab_.columns.keyval] = keyval;
        row[shortcutsTab_.columns.modifiers] = modifiers;
        
        // Formater l'affichage du raccourci
        gchar* accel_name = gtk_accelerator_name(keyval, modifiers);
        if (accel_name) {
            row[shortcutsTab_.columns.accelerator] = gtk_accelerator_get_label(keyval, modifiers);
            g_free(accel_name);
        }
        
        // Mettre à jour le raccourci dans InputManager
        updateShortcut(name, row[shortcutsTab_.columns.accelerator]);
        
        updateStatus("Raccourci mis à jour");
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la modification du raccourci: " << e.what() << std::endl;
        showError("Impossible de modifier le raccourci");
    }
}

void MainWindow::onDisplayChanged() {
    try {
        // Mettre à jour la liste des écrans disponibles
        updateDisplayList();
        
        // Mettre à jour l'interface utilisateur en conséquence
        if (displayManager_) {
            // Exemple: mettre à jour l'état du suivi de la souris
            if (displayTab_.followMouseCheck) {
                displayTab_.followMouseCheck->set_active(displayManager_->isFollowingMouse());
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour de l'affichage: " << e.what() << std::endl;
        showError("Impossible de mettre à jour l'affichage");
    }
}

void MainWindow::updateDisplayList() {
    try {
        if (!displayManager_ || !displayTab_.displayList) return;
        
        // Effacer la liste actuelle
        displayTab_.displayList->clear();
        
        // Obtenir la liste des écrans disponibles
        auto displays = displayManager_->getAvailableDisplays();
        
        // Remplir la liste avec les écrans disponibles
        for (const auto& display : displays) {
            Gtk::TreeModel::Row row = *(displayTab_.displayList->append());
            row[displayTab_.columns.name] = display.name;
            row[displayTab_.columns.resolution] = 
                std::to_string(display.width) + "x" + std::to_string(display.height);
            row[displayTab_.columns.isPrimary] = display.isPrimary;
            row[displayTab_.columns.scale] = display.scale;
            row[displayTab_.columns.displayPtr] = display.ptr;
        }
        
        // Sélectionner l'écran principal s'il existe
        if (!displays.empty()) {
            auto children = displayTab_.displayList->children();
            for (auto it = children.begin(); it != children.end(); ++it) {
                if ((*it)[displayTab_.columns.isPrimary]) {
                    displayTab_.displayTreeView->get_selection()->select(it);
                    break;
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour de la liste des affichages: " << e.what() << std::endl;
        showError("Impossible de mettre à jour la liste des affichages");
    }
}

void MainWindow::updateCursorPreview() {
    try {
        if (!cursorTab_.previewArea) {
            return;
        }
        
        // Forcer le redessin de la zone d'aperçu
        cursorTab_.previewArea->queue_draw();
        
        // Si un gestionnaire de curseur est disponible, mettre à jour l'aperçu
        if (cursorManager_) {
            // Mettre à jour l'échelle du curseur dans l'aperçu
            if (cursorTab_.scaleSlider) {
                cursorManager_->setScale(cursorTab_.scaleSlider->get_value());
            }
            
            // Mettre à jour la vitesse d'animation si nécessaire
            if (cursorTab_.fpsSpin) {
                cursorManager_->setAnimationSpeed(cursorTab_.fpsSpin->get_value_as_int());
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour de l'aperçu du curseur: " << e.what() << std::endl;
    }
}

void MainWindow::registerShortcut(const ShortcutInfo& shortcut) {
    try {
        if (!inputManager_) {
            std::cerr << "InputManager non initialisé" << std::endl;
            return;
        }
        
        // Vérifier si le raccourci existe déjà
        for (const auto& row : shortcutsTab_.listStore->children()) {
            if (row[shortcutsTab_.columns.name] == shortcut.name) {
                std::cerr << "Un raccourci avec ce nom existe déjà: " << shortcut.name << std::endl;
                return;
            }
        }
        
        // Ajouter le raccourci au modèle
        Gtk::TreeModel::Row row = *(shortcutsTab_.listStore->append());
        row[shortcutsTab_.columns.name] = shortcut.name;
        row[shortcutsTab_.columns.description] = shortcut.description;
        row[shortcutsTab_.columns.keyval] = shortcut.keyval;
        row[shortcutsTab_.columns.modifiers] = shortcut.modifiers;
        
        // Formater l'affichage du raccourci
        gchar* accel_name = gtk_accelerator_name(shortcut.keyval, shortcut.modifiers);
        if (accel_name) {
            row[shortcutsTab_.columns.accelerator] = gtk_accelerator_get_label(
                shortcut.keyval, shortcut.modifiers);
            g_free(accel_name);
        }
        
        // Enregistrer le raccourci dans InputManager
        inputManager_->registerShortcut(
            shortcut.name,
            shortcut.accelerator,
            [this, name = shortcut.name]() {
                // Gérer l'action du raccourci
                onShortcutTriggered(name);
            }
        );
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'enregistrement du raccourci: " << e.what() << std::endl;
        showError("Impossible d'enregistrer le raccourci");
    }
}

void MainWindow::updateShortcut(const std::string& name, const std::string& newAccelerator) {
    try {
        if (!inputManager_) {
            std::cerr << "InputManager non initialisé" << std::endl;
            return;
        }
        
        // Trouver le raccourci dans la liste
        for (auto& row : shortcutsTab_.listStore->children()) {
            if (row[shortcutsTab_.columns.name] == name) {
                // Mettre à jour l'accélérateur dans le modèle
                row[shortcutsTab_.columns.accelerator] = newAccelerator;
                
                // Parser le nouvel accélérateur
                guint keyval;
                Gdk::ModifierType mods;
                if (gtk_accelerator_parse(newAccelerator.c_str(), &keyval, (GdkModifierType*)&mods)) {
                    row[shortcutsTab_.columns.keyval] = keyval;
                    row[shortcutsTab_.columns.modifiers] = mods;
                    
                    // Mettre à jour le raccourci dans InputManager
                    inputManager_->removeShortcut(name);
                    inputManager_->registerShortcut(
                        name,
                        newAccelerator,
                        [this, name]() {
                            onShortcutTriggered(name);
                        }
                    );
                } else {
                    std::cerr << "Format d'accélérateur invalide: " << newAccelerator << std::endl;
                    showError("Format de raccourci invalide");
                }
                
                break;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour du raccourci: " << e.what() << std::endl;
        showError("Impossible de mettre à jour le raccourci");
    }
}

void MainWindow::showStatus(const std::string& message, bool isError) {
    updateStatus(message);
    if (isError) {
        showError(message);
    }
}

void MainWindow::onShortcutTriggered(const std::string& name) {
    try {
        // Journalisation du déclenchement du raccourci
        std::cout << "Raccourci déclenché : " << name << std::endl;
        
        // Gestion des différents types de raccourcis
        if (name == "apply_settings") {
            onApplyClicked();
        } 
        else if (name == "cancel_changes") {
            onCancelClicked();
        }
        else if (name == "select_cursor") {
            onCursorFileSelected();
        }
        else if (name == "toggle_fullscreen") {
            // Basculer le mode plein écran
            if (is_active()) {
                unfullscreen();
            } else {
                fullscreen();
            }
        }
        else if (name == "show_about") {
            showAboutDialog();
        }
        else if (name == "quit_application") {
            close();
        }
        
        // Mettre à jour la barre d'état
        updateStatus("Action effectuée : " + name);
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du traitement du raccourci " << name 
                 << ": " << e.what() << std::endl;
        showError("Erreur lors du traitement du raccourci");
    }
}

void MainWindow::addDefaultShortcuts() {
    try {
        if (!shortcutsTab_.listStore) {
            std::cerr << "Modèle de données des raccourcis non initialisé" << std::endl;
            return;
        }
        
        // Liste des raccourcis par défaut
        std::vector<ShortcutInfo> defaultShortcuts = {
            {"apply_settings", "Appliquer les paramètres", "<Primary>Return", 0, 0},
            {"cancel_changes", "Annuler les modifications", "<Primary>Escape", 0, 0},
            {"select_cursor", "Sélectionner un curseur", "<Primary>O", 0, 0},
            {"toggle_fullscreen", "Basculer en plein écran", "F11", 0, 0},
            {"show_about", "Afficher les informations", "F1", 0, 0},
            {"quit_application", "Quitter l'application", "<Primary>Q", 0, 0}
        };
        
        // Parser les accélérateurs et enregistrer les raccourcis
        for (auto& shortcut : defaultShortcuts) {
            // Parser l'accélérateur
            guint keyval;
            Gdk::ModifierType mods;
            if (gtk_accelerator_parse(
                shortcut.accelerator.c_str(), 
                &keyval, 
                (GdkModifierType*)&mods)) {
                
                shortcut.keyval = keyval;
                shortcut.modifiers = mods;
                
                // Enregistrer le raccourci
                registerShortcut(shortcut);
            } else {
                std::cerr << "Impossible de parser l'accélérateur : " 
                         << shortcut.accelerator << std::endl;
            }
        }
        
        updateStatus("Raccourcis par défaut chargés");
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'ajout des raccourcis par défaut: " 
                 << e.what() << std::endl;
        showError("Impossible de charger les raccourcis par défaut");
    }
}

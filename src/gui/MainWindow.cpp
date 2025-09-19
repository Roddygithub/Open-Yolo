#include "gui/MainWindow.hpp"
#include "cursormanager/CursorManager.hpp"
#include "displaymanager/DisplayManager.hpp"
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/gesturemultipress.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/notebook.h>
#include <gtkmm/scale.h>
#include <iomanip>  // Pour std::setprecision
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/accelmap.h>
#include <iostream>
#include <sstream>
#include <gtkmm/spinbutton.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treemodelsort.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treemodelfilter.h>
#include <gtkmm/treeview.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/messagedialog.h>
#include <gdk/gdk.h>  // Pour gtk_accelerator_name
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>
#include <glibmm/ustring.h>
#include <cairomm/context.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

// Colonnes du modèle de données pour la liste des raccourcis
class ShortcutColumns : public Gtk::TreeModel::ColumnRecord {
public:
    ShortcutColumns() {
        add(name);
        add(description);
        add(accelerator);
    }
    
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::ustring> description;
    Gtk::TreeModelColumn<Glib::ustring> accelerator;
};

// Colonnes du modèle de données pour la liste des écrans
class DisplayColumns : public Gtk::TreeModel::ColumnRecord {
public:
    DisplayColumns() {
        add(id);
        add(name);
        add(resolution);
        add(scale);
        add(isPrimary);
    }
    
    Gtk::TreeModelColumn<int> id;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::ustring> resolution;
    Gtk::TreeModelColumn<double> scale;
    Gtk::TreeModelColumn<bool> isPrimary;
};

MainWindow::MainWindow(
    std::shared_ptr<cursor_manager::CursorManager> cursorManager,
    std::shared_ptr<DisplayManager> displayManager,
    std::shared_ptr<InputManager> inputManager
) : cursorManager_(cursorManager),
    displayManager_(displayManager),
    inputManager_(inputManager),
    mainBox_(Gtk::ORIENTATION_VERTICAL, 5) {
    
    std::cout << "Début de la construction de MainWindow" << std::endl;
    
    // Configuration de la fenêtre principale
    std::cout << "Configuration de la fenêtre principale..." << std::endl;
    set_title("Open-Yolo - Gestionnaire de curseurs");
    set_default_size(800, 600);
    set_border_width(5);
    std::cout << "Fenêtre configurée" << std::endl;
    
    // Initialisation des membres de cursorTab_
    cursorTab_.fpsSpin = Gtk::make_managed<Gtk::SpinButton>(Gtk::Adjustment::create(60.0, 1.0, 144.0, 1.0, 10.0, 0.0));
    cursorTab_.scaleSlider = Gtk::make_managed<Gtk::Scale>(Gtk::ORIENTATION_HORIZONTAL);
    cursorTab_.scaleSlider->set_range(0.1, 5.0);
    cursorTab_.scaleSlider->set_increments(0.1, 0.5);
    cursorTab_.scaleSlider->set_value(1.0);
    cursorTab_.fileChooserBtn = Gtk::make_managed<Gtk::Button>("Sélectionner un curseur...");
    cursorTab_.previewArea = Gtk::make_managed<Gtk::DrawingArea>();
    cursorTab_.previewArea->set_size_request(64, 64);
    
    // L'initialisation de l'InputManager et l'ajout des raccourcis
    // seront effectués plus tard dans main.cpp
    std::cout << "Initialisation de l'InputManager différée..." << std::endl;
    if (!inputManager_) {
        std::cerr << "Erreur: inputManager_ est nul" << std::endl;
    }
    
    // Initialisation du notebook
    std::cout << "Initialisation du notebook..." << std::endl;
    notebook_ = Gtk::make_managed<Gtk::Notebook>();
    if (!notebook_) {
        std::cerr << "Erreur: Impossible de créer le notebook" << std::endl;
        throw std::runtime_error("Impossible de créer le notebook");
    }
    notebook_->set_vexpand(true);
    std::cout << "Notebook initialisé" << std::endl;
    
    // Création des onglets
    auto cursorFrame = Gtk::make_managed<Gtk::Frame>();
    auto shortcutsFrame = Gtk::make_managed<Gtk::Frame>();
    auto displayFrame = Gtk::make_managed<Gtk::Frame>();
    auto aboutFrame = Gtk::make_managed<Gtk::Frame>();
    
    // Configuration des onglets
    auto cursorBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    cursorBox->set_margin_top(5);
    cursorBox->set_margin_bottom(5);
    cursorBox->set_margin_start(5);
    cursorBox->set_margin_end(5);
    setupCursorTab(cursorBox);
    cursorFrame->add(*cursorBox);
    
    auto shortcutsBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    shortcutsBox->set_margin_top(5);
    shortcutsBox->set_margin_bottom(5);
    shortcutsBox->set_margin_start(5);
    shortcutsBox->set_margin_end(5);
    setupShortcutsTab(shortcutsBox);
    shortcutsFrame->add(*shortcutsBox);
    
    auto displayBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    displayBox->set_margin_top(5);
    displayBox->set_margin_bottom(5);
    displayBox->set_margin_start(5);
    displayBox->set_margin_end(5);
    setupDisplayTab(displayBox);
    displayFrame->add(*displayBox);
    
    auto aboutBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    aboutBox->set_margin_top(5);
    aboutBox->set_margin_bottom(5);
    aboutBox->set_margin_start(5);
    aboutBox->set_margin_end(5);
    setupAboutTab(aboutBox);
    aboutFrame->add(*aboutBox);
    
    // Ajout des onglets au notebook avec des marges
    notebook_->append_page(*cursorFrame, "Curseur");
    notebook_->append_page(*shortcutsFrame, "Raccourcis");
    notebook_->append_page(*displayFrame, "Affichage");
    notebook_->append_page(*aboutFrame, "À propos");
    
    // Configuration de la barre d'état
    statusBar_.set_halign(Gtk::ALIGN_START);
    statusBar_.set_margin_top(5);
    statusBar_.push("Prêt");
    
    // Configuration de la boîte principale
    std::cout << "Configuration de la boîte principale..." << std::endl;
    mainBox_.pack_start(*notebook_, Gtk::PACK_EXPAND_WIDGET);
    mainBox_.pack_start(statusBar_, Gtk::PACK_SHRINK);
    
    // Ajout de la boîte principale à la fenêtre
    std::cout << "Ajout de la boîte principale à la fenêtre..." << std::endl;
    add(mainBox_);
    std::cout << "Boîte principale ajoutée" << std::endl;
    
    // Connexion des signaux
    signal_delete_event().connect(sigc::mem_fun(*this, &MainWindow::on_delete_event), false);
    
    // Chargement des paramètres et mise à jour de l'interface
    loadSettings();
    updateDisplayList();
    updateCursorPreview();
    
    // Affichage de la fenêtre
    std::cout << "Affichage de tous les enfants..." << std::endl;
    show_all_children();
    std::cout << "Fin de la construction de MainWindow" << std::endl;
}

// Le destructeur est vide car les ressources sont gérées par des pointeurs intelligents
MainWindow::~MainWindow() = default;

// ... Reste du code ...

void MainWindow::setupCursorTab(Gtk::Box* parent) {
    auto grid = Gtk::make_managed<Gtk::Grid>();
    grid->set_row_spacing(5);
    grid->set_column_spacing(5);
    grid->set_margin_bottom(10);
    grid->set_margin_start(10);
    grid->set_margin_end(10);
    
    // Création du bouton de sélection de fichier
    cursorTab_.fileChooserBtn = Gtk::make_managed<Gtk::Button>("Sélectionner un fichier");
    cursorTab_.fileChooserBtn->set_halign(Gtk::ALIGN_FILL);
    
    // Création du sélecteur de fichier
    auto dialog = new Gtk::FileChooserDialog("Sélectionner un curseur",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog->set_transient_for(*this);
    
    // Ajout des boutons
    dialog->add_button("Annuler", Gtk::RESPONSE_CANCEL);
    dialog->add_button("Sélectionner", Gtk::RESPONSE_ACCEPT);
    
    // Configuration du filtre de fichiers
    auto filter = Gtk::FileFilter::create();
    filter->set_name("Images");
    filter->add_mime_type("image/*");
    filter->add_pattern("*.png");
    filter->add_pattern("*.svg");
    filter->add_pattern("*.cur");
    filter->add_pattern("*.ani");
    dialog->add_filter(filter);
    
    // Connexion du signal de réponse
    cursorTab_.fileChooserBtn->signal_clicked().connect([this, dialog]() {
        int result = dialog->run();
        if (result == Gtk::RESPONSE_ACCEPT) {
            std::string filename = dialog->get_filename();
            if (!filename.empty()) {
                currentCursorPath_ = filename;
                updateStatus("Curseur sélectionné : " + filename);
                onCursorSettingsChanged();
            }
        }
        dialog->hide();
    });
    
    // Configuration du sélecteur de FPS
    cursorTab_.fpsSpin = Gtk::make_managed<Gtk::SpinButton>();
    cursorTab_.fpsSpin->set_adjustment(Gtk::Adjustment::create(60.0, 1.0, 240.0, 1.0, 10.0, 0.0));
    cursorTab_.fpsSpin->set_digits(0);
    
    // Création du sélecteur d'échelle
    cursorTab_.scaleSlider = Gtk::make_managed<Gtk::Scale>(Gtk::Adjustment::create(1.0, 0.1, 3.0, 0.1, 0.5, 0.0));
    // set_orientation n'est pas disponible en GTK3, on utilise set_direction
    cursorTab_.scaleSlider->set_direction(Gtk::TEXT_DIR_LTR);
    cursorTab_.scaleSlider->set_digits(2);
    cursorTab_.scaleSlider->set_hexpand(true);
    
    // Configuration de la zone de prévisualisation
    cursorTab_.previewArea = Gtk::manage(new Gtk::DrawingArea());
    cursorTab_.previewArea->set_size_request(200, 200);
    cursorTab_.previewArea->set_halign(Gtk::ALIGN_CENTER);
    cursorTab_.previewArea->set_valign(Gtk::ALIGN_CENTER);
    
    // Création des étiquettes
    auto fileLabel = Gtk::make_managed<Gtk::Label>("Fichier du curseur:");
    fileLabel->set_halign(Gtk::ALIGN_START);
    
    auto fpsLabel = Gtk::make_managed<Gtk::Label>("FPS:");
    fpsLabel->set_halign(Gtk::ALIGN_START);
    
    auto scaleLabel = Gtk::make_managed<Gtk::Label>("Échelle:");
    scaleLabel->set_halign(Gtk::ALIGN_START);
    
    auto previewLabel = Gtk::make_managed<Gtk::Label>("Aperçu:");
    previewLabel->set_halign(Gtk::ALIGN_START);
    
    // Ajout des widgets à la grille
    grid->attach(*fileLabel, 0, 0, 1, 1);
    grid->attach(*cursorTab_.fileChooserBtn, 1, 0, 2, 1);
    
    grid->attach(*fpsLabel, 0, 1, 1, 1);
    grid->attach(*cursorTab_.fpsSpin, 1, 1, 1, 1);
    
    grid->attach(*scaleLabel, 0, 2, 1, 1);
    grid->attach(*cursorTab_.scaleSlider, 1, 2, 1, 1);
    
    grid->attach(*previewLabel, 0, 3, 1, 1);
    grid->attach(*cursorTab_.previewArea, 1, 3, 2, 1);
    
    // Création du cadre pour l'onglet
    auto frame = Gtk::make_managed<Gtk::Frame>();
    // Remplacer set_child par add en GTK3
    frame->add(*grid);
    
    // Ajout du cadre à l'onglet
    auto page = notebook_->get_nth_page(0);
    if (auto container = dynamic_cast<Gtk::Box*>(page)) {
        // Remplacer append par pack_start en GTK3
        container->pack_start(*frame, Gtk::PACK_EXPAND_WIDGET);
    }
}

void MainWindow::setupShortcutsTab(Gtk::Box* parent) {
    // Création du modèle de données s'il n'existe pas déjà
    if (!shortcutsTab_.listStore) {
        shortcutsTab_.listStore = Gtk::ListStore::create(shortcutsTab_.columns);
    }
    
    // Création de la vue en arborescence si elle n'existe pas déjà
    if (!shortcutsTab_.treeView) {
        shortcutsTab_.treeView = Gtk::make_managed<Gtk::TreeView>(shortcutsTab_.listStore);
        shortcutsTab_.treeView->set_headers_visible(true);
        
        // Création et configuration des colonnes
        {
            // Colonne Action
            auto renderer = Gtk::make_managed<Gtk::CellRendererText>();
            auto column = Gtk::make_managed<Gtk::TreeViewColumn>("Action", *renderer);
            column->add_attribute(renderer->property_text(), shortcutsTab_.columns.name);
            shortcutsTab_.treeView->append_column(*column);
        }
        
        {
            // Colonne Description
            auto renderer = Gtk::make_managed<Gtk::CellRendererText>();
            auto column = Gtk::make_managed<Gtk::TreeViewColumn>("Description", *renderer);
            column->add_attribute(renderer->property_text(), shortcutsTab_.columns.description);
            shortcutsTab_.treeView->append_column(*column);
        }
        
        {
            // Colonne Raccourci
            auto renderer = Gtk::make_managed<Gtk::CellRendererAccel>();
            auto column = Gtk::make_managed<Gtk::TreeViewColumn>("Raccourci", *renderer);
            
            // Configuration du rendu du raccourci
            renderer->property_editable() = true;
            renderer->signal_accel_edited().connect(
                sigc::mem_fun(*this, &MainWindow::onShortcutEdited));
                
            // Configuration de la colonne
            column->add_attribute(renderer->property_text(), shortcutsTab_.columns.accelerator);
            shortcutsTab_.treeView->append_column(*column);
        }
    }
    
    // Configuration du défilement
    auto scrolledWindow = Gtk::make_managed<Gtk::ScrolledWindow>();
    // Remplacer AUTOMATIC par ALWAYS en GTK3
    scrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    // Remplacer set_child par add en GTK3
    scrolledWindow->add(*shortcutsTab_.treeView);
    
    // Création du cadre pour l'onglet
    auto frame = Gtk::make_managed<Gtk::Frame>();
    frame->add(*scrolledWindow);
    
    // Ajout du cadre à l'onglet parent
    if (parent) {
        parent->pack_start(*frame, Gtk::PACK_EXPAND_WIDGET);
    }
}

void MainWindow::setupDisplayTab(Gtk::Box* parent) {
    if (!parent) return;
    
    // Création du modèle de données pour les écrans
    displayTab_.displayList = Gtk::ListStore::create(displayTab_.columns);
    
    // Configuration de la vue en arborescence
    displayTab_.displayTreeView = Gtk::make_managed<Gtk::TreeView>(displayTab_.displayList);
    displayTab_.displayTreeView->set_headers_visible(true);
    
    // Fonction utilitaire pour créer une colonne avec un renderer de texte
    auto create_text_column = [](const Glib::ustring& title, const Gtk::TreeModelColumn<Glib::ustring>& column) {
        auto col = Gtk::make_managed<Gtk::TreeView::Column>(title);
        auto renderer = Gtk::make_managed<Gtk::CellRendererText>();
        col->pack_start(*renderer);
        col->add_attribute(renderer->property_text(), column);
        return col;
    };
    
    // Création des colonnes
    auto idCol = Gtk::make_managed<Gtk::TreeView::Column>("ID");
    auto nameCol = Gtk::make_managed<Gtk::TreeView::Column>("Nom");
    auto resCol = Gtk::make_managed<Gtk::TreeView::Column>("Résolution");
    
    // Ajout des cellules de rendu
    auto idCell = Gtk::make_managed<Gtk::CellRendererText>();
    auto nameCell = Gtk::make_managed<Gtk::CellRendererText>();
    auto resCell = Gtk::make_managed<Gtk::CellRendererText>();
    
    // Configuration des colonnes
    idCol->pack_start(*idCell);
    nameCol->pack_start(*nameCell);
    resCol->pack_start(*resCell);
    
    // Association des colonnes aux données du modèle
    idCol->pack_start(*idCell);
    // Utiliser les bons noms de colonnes de la structure DisplayTab
    idCol->add_attribute(idCell->property_text(), displayTab_.columns.name);
    nameCol->add_attribute(nameCell->property_text(), displayTab_.columns.name);
    resCol->add_attribute(resCell->property_text(), displayTab_.columns.resolution);
    
    // Colonne pour l'échelle avec un rendu personnalisé
    auto scaleCol = Gtk::make_managed<Gtk::TreeView::Column>("Échelle");
    auto scaleCell = Gtk::make_managed<Gtk::CellRendererText>();
    scaleCol->pack_start(*scaleCell);
    scaleCol->set_cell_data_func(*scaleCell, 
        sigc::mem_fun(*this, &MainWindow::onScaleCellData));
    
    // Colonne pour l'écran principal
    auto primaryCol = Gtk::make_managed<Gtk::TreeView::Column>("Principal");
    auto primaryCell = Gtk::make_managed<Gtk::CellRendererToggle>();
    primaryCol->pack_start(*primaryCell);
    primaryCol->add_attribute(primaryCell->property_active(), displayTab_.columns.isPrimary);
    
    // Connexion du signal pour gérer le clic sur la case à cocher
    primaryCell->signal_toggled().connect([this](const Glib::ustring& path) {
        auto iter = displayTab_.displayList->get_iter(path);
        if (iter) {
bool current = (*iter)[displayTab_.columns.isPrimary];
(*iter)[displayTab_.columns.isPrimary] = !current;
            onDisplaySettingsChanged();
        }
    });
    
    // Ajout des colonnes à la vue
    displayTab_.displayTreeView->append_column(*idCol);
    displayTab_.displayTreeView->append_column(*nameCol);
    displayTab_.displayTreeView->append_column(*resCol);
    displayTab_.displayTreeView->append_column(*scaleCol);
    displayTab_.displayTreeView->append_column(*primaryCol);
    
    // Configuration du défilement
    auto scrolledWindow = Gtk::make_managed<Gtk::ScrolledWindow>();
    // Remplacer AUTOMATIC par ALWAYS en GTK3
    scrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledWindow->add(*displayTab_.displayTreeView);
    scrolledWindow->set_hexpand(true);
    scrolledWindow->set_vexpand(true);
    
    // Configuration du suivi de la souris
    displayTab_.followMouseCheck = Gtk::make_managed<Gtk::CheckButton>("Suivre la souris");
    displayTab_.followMouseCheck->signal_toggled().connect(
        sigc::mem_fun(*this, &MainWindow::onDisplaySettingsChanged));
    
    // Configuration de la mise en page
    auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    vbox->set_border_width(5);
    vbox->pack_start(*scrolledWindow, Gtk::PACK_EXPAND_WIDGET);
    vbox->pack_start(*displayTab_.followMouseCheck, Gtk::PACK_SHRINK);
    
    // Ajout au parent
    parent->pack_start(*vbox, Gtk::PACK_EXPAND_WIDGET);
}

void MainWindow::setupAboutTab(Gtk::Box* parent) {
    if (!parent) return;
    
    // Création des widgets
    auto logo = Gtk::make_managed<Gtk::Image>();
    try {
        // Utiliser la constante GTK_ICON_SIZE_DIALOG qui est similaire à LARGE en GTK3
    logo->set_from_icon_name("preferences-desktop-theme", Gtk::ICON_SIZE_DIALOG);
        logo->set_pixel_size(128);
    } catch (const Glib::Error& e) {
        std::cerr << "Erreur lors du chargement de l'icône: " << e.what() << std::endl;
    }
    
    auto nameLabel = Gtk::make_managed<Gtk::Label>("Open-Yolo");
    nameLabel->set_halign(Gtk::ALIGN_CENTER);
    nameLabel->set_margin_bottom(10);
    
    auto versionLabel = Gtk::make_managed<Gtk::Label>("Version 1.0.0");
    versionLabel->set_halign(Gtk::ALIGN_CENTER);
    versionLabel->set_margin_bottom(20);
    
    auto descriptionLabel = Gtk::make_managed<Gtk::Label>(
        "Open-Yolo est un gestionnaire de curseurs personnalisables pour Linux.\n"
        "Créez, personnalisez et appliquez facilement vos propres curseurs."
    );
    descriptionLabel->set_halign(Gtk::ALIGN_CENTER);
    descriptionLabel->set_justify(Gtk::JUSTIFY_CENTER);
    descriptionLabel->set_line_wrap(true);
    descriptionLabel->set_margin_bottom(20);
    descriptionLabel->set_max_width_chars(50);
    
    auto licenseButton = Gtk::make_managed<Gtk::Button>("Licence");
    licenseButton->set_halign(Gtk::ALIGN_CENTER);
    licenseButton->signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::showLicenseDialog));
    
    // Configuration des boutons
    auto buttonBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    buttonBox->set_halign(Gtk::ALIGN_END);
    buttonBox->set_valign(Gtk::ALIGN_CENTER);
    buttonBox->pack_start(*licenseButton, Gtk::PACK_SHRINK);
    
    // Configuration de la mise en page
    auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    vbox->set_valign(Gtk::ALIGN_CENTER);
    vbox->set_halign(Gtk::ALIGN_CENTER);
    vbox->set_border_width(20);
    
    // Ajout des widgets avec des marges
    vbox->pack_start(*logo, Gtk::PACK_SHRINK);
    vbox->pack_start(*nameLabel, Gtk::PACK_SHRINK);
    vbox->pack_start(*versionLabel, Gtk::PACK_SHRINK);
    vbox->pack_start(*descriptionLabel, Gtk::PACK_SHRINK);
    vbox->pack_start(*buttonBox, Gtk::PACK_SHRINK);
    
    // Ajout du conteneur principal au parent
    parent->pack_start(*vbox, Gtk::PACK_SHRINK);
}

void MainWindow::setupStatusBar() {
    statusBar_.set_halign(Gtk::ALIGN_START);
    statusBar_.set_margin_top(5);
    statusBar_.push("Prêt");
}

void MainWindow::connectSignals() {
    // Gestion de la fermeture de la fenêtre (GTK3)
    signal_delete_event().connect(
        sigc::mem_fun(*this, &MainWindow::on_delete_event), false);
    
    // Connexion des signaux des onglets
    if (cursorTab_.fpsSpin) {
        cursorTab_.fpsSpin->signal_value_changed().connect(
            sigc::mem_fun(*this, &MainWindow::onFpsChanged));
    }
    
    if (cursorTab_.scaleSlider) {
        cursorTab_.scaleSlider->signal_value_changed().connect(
            sigc::mem_fun(*this, &MainWindow::onScaleChanged));
    }
    
    // Connexion des boutons
    if (cursorTab_.fileChooserBtn) {
        cursorTab_.fileChooserBtn->signal_clicked().connect(
            sigc::mem_fun(*this, &MainWindow::onCursorFileSelected));
    }
    
    // La connexion du signal du bouton d'application est gérée ailleurs
    
    // Connexion des signaux de l'InputManager
    if (inputManager_) {
        // La connexion du signal de raccourci est gérée par le gestionnaire d'entrée
    }
    
    // Connexion des signaux de l'onglet Affichage
    if (displayTab_.displayTreeView) {
        auto selection = displayTab_.displayTreeView->get_selection();
        if (selection) {
            selection->signal_changed().connect(
                sigc::mem_fun(*this, &MainWindow::onDisplayChanged));
        }
    }
    
    if (displayTab_.followMouseCheck) {
        displayTab_.followMouseCheck->signal_toggled().connect(
            sigc::mem_fun(*this, &MainWindow::onDisplaySettingsChanged));
    }
    
    // Connexion des signaux de la fenêtre
    signal_delete_event().connect(
        sigc::mem_fun(*this, &MainWindow::on_delete_event), false);
}

void MainWindow::onCursorFileSelected() {
    try {
        // Créer un sélecteur de fichiers natif
        Gtk::FileChooserDialog dialog(*this, "Sélectionner un curseur", 
                                     Gtk::FILE_CHOOSER_ACTION_OPEN);
            
        // Ajouter les boutons
        dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
        
        // Filtrer les fichiers par extension
        auto filter = Gtk::FileFilter::create();
        filter->set_name("Fichiers de curseurs");
        filter->add_pattern("*.png");
        filter->add_pattern("*.svg");
        filter->add_pattern("*.cur");
        filter->add_pattern("*.ani");
        dialog.add_filter(filter);
        
        // Afficher la boîte de dialogue de manière synchrone
        dialog.set_modal(true);
        dialog.set_transient_for(*this);
        
        // Exécuter la boîte de dialogue
        int result = dialog.run();
        
        if (result == Gtk::RESPONSE_ACCEPT) {
            std::string filename = dialog.get_filename();
            
            // Vérifier que le fichier existe
            if (Glib::file_test(filename, Glib::FILE_TEST_EXISTS)) {
                // Mettre à jour le chemin du curseur
                currentCursorPath_ = filename;
                
                // Mettre à jour le gestionnaire de curseur
                if (cursorManager_) {
                    cursorManager_->setCursor(currentCursorPath_);
                    
                    // Mettre à jour l'aperçu du curseur
                    if (cursorTab_.previewArea) {
                        cursorTab_.previewArea->queue_draw();
                    }
                }
                
                // Mettre à jour l'interface utilisateur
                if (cursorTab_.fileChooserBtn) {
                    cursorTab_.fileChooserBtn->set_label(Glib::path_get_basename(currentCursorPath_));
                }
                
                // Mettre à jour le statut
                updateStatus("Curseur sélectionné: " + Glib::path_get_basename(filename));
            } else {
                showError("Le fichier sélectionné n'existe pas");
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la sélection du fichier: " << e.what() << std::endl;
        showError("Impossible de sélectionner le fichier: " + std::string(e.what()));
    }
}

void MainWindow::onCursorSettingsChanged() {
    if (!cursorManager_) {
        return;
    }
    
    try {
        // Mise à jour des paramètres du curseur
        if (cursorTab_.fpsSpin) {
            double fps = cursorTab_.fpsSpin->get_value();
            cursorManager_->setFps(static_cast<int>(fps));
        }
        
        // Mise à jour de l'opacité si nécessaire
        if (cursorTab_.scaleSlider) {
            double alpha = cursorTab_.scaleSlider->get_value();
            cursorManager_->setAlpha(static_cast<float>(alpha));
        }
        
        // Mise à jour de l'aperçu
        if (cursorTab_.previewArea) {
            cursorTab_.previewArea->queue_draw();
        }
        
        settingsModified_ = true;
        updateStatus("Paramètres du curseur mis à jour");
    } catch (const std::exception& e) {
        showError("Erreur lors de la mise à jour des paramètres du curseur : " + std::string(e.what()));
    }
}

void MainWindow::onDisplaySettingsChanged() {
    if (!displayManager_) {
        return;
    }
    
    try {
        // Mise à jour de l'affichage
        if (auto selection = displayTab_.displayTreeView->get_selection()) {
            auto iter = selection->get_selected();
            if (iter) {
                Glib::ustring displayName = (*iter)[displayTab_.columns.name];
                
                // Mettre à jour l'affichage principal si nécessaire
                if (displayManager_) {
                    // Mettre à jour l'interface utilisateur pour refléter les changements
                    for (auto& row : displayTab_.displayList->children()) {
                        bool isSelected = (row[displayTab_.columns.name] == displayName);
                        row[displayTab_.columns.isPrimary] = isSelected;
                    }
                    
                    settingsModified_ = true;
                    updateStatus("Paramètres d'affichage mis à jour");
                }
            }
        }
    } catch (const std::exception& e) {
        showError("Erreur lors de la configuration de l'affichage : " + std::string(e.what()));
    }
}

void MainWindow::onShortcutEdited(const Glib::ustring& path_string, guint keyval, Gdk::ModifierType modifiers, guint hardware_keycode) {
    Gtk::TreePath path(path_string);
    Gtk::TreeModel::iterator iter = shortcutsTab_.listStore->get_iter(path);
    
    if (!iter) {
        return;
    }
    
    Gtk::TreeRow row = *iter;
    
    // Vérifier si le raccourci est valide
    if (keyval == 0) {
        updateStatus("Raccourci invalide");
        return;
    }
    
    // Convertir le raccourci en chaîne lisible
    GdkModifierType mods = static_cast<GdkModifierType>(modifiers);
    char* accel_name_cstr = gtk_accelerator_name(keyval, mods);
    Glib::ustring accel_name = accel_name_cstr ? accel_name_cstr : "";
    g_free(accel_name_cstr);
    
    // Mettre à jour la ligne
    row[shortcutsTab_.columns.accelerator] = accel_name;
    
    // Mettre à jour le statut
    settingsModified_ = true;
    updateStatus("Raccourci mis à jour : " + accel_name);
}

void MainWindow::updateStatus(const std::string& message) {
    statusBar_.pop(); // Effacer le message précédent
    statusBar_.push(message);
}

void MainWindow::showError(const std::string& message) {
    Gtk::MessageDialog dialog(*this, "Erreur", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    dialog.set_secondary_text(message);
    dialog.run();
}

void MainWindow::showLicenseDialog() {
    Gtk::MessageDialog dialog(*this, "License", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
    dialog.set_secondary_text(
        "Open-Yolo is free software: you can redistribute it and/or modify\n"
        "it under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation, either version 3 of the License, or\n"
        "(at your option) any later version.\n\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program.  If not, see <https://www.gnu.org/licenses/>."
    );
    dialog.run();
}

void MainWindow::onScaleChanged() {
    if (!cursorTab_.scaleSlider) return;
    double scale = cursorTab_.scaleSlider->get_value();
    updateStatus("Échelle définie à : " + std::to_string(scale));
    settingsModified_ = true;
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
    // saveSettings();
}

void MainWindow::onCancelClicked()
{
    hide(); // Cache la fenêtre
}

void MainWindow::onFpsChanged() {
    if (!cursorTab_.fpsSpin) return;
    int fps = static_cast<int>(cursorTab_.fpsSpin->get_value());
    updateStatus("FPS défini à : " + std::to_string(fps));
    settingsModified_ = true;
}

void MainWindow::onScaleCellData(Gtk::CellRenderer* cell, const Gtk::TreeModel::const_iterator& iter) {
    auto renderer = dynamic_cast<Gtk::CellRendererText*>(cell);
    if (!renderer) {
        return;
    }
    
    try {
        // Utiliser Gtk::TreeRow directement au lieu de Gtk::TreeModel::Row
        const auto& row = *iter;
        double scale = row[displayTab_.columns.scale];
std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << scale;
        renderer->property_text() = oss.str();
    } catch (const std::exception& e) {
        std::cerr << "Erreur dans onScaleCellData: " << e.what() << std::endl;
        renderer->property_text() = "1.0";
    }
}

void MainWindow::addDefaultShortcuts()
{
    if (!inputManager_) {
        std::cerr << "Erreur: Gestionnaire d'entrée non initialisé" << std::endl;
        return;
    }

    try {
        // Raccourci pour quitter l'application (Ctrl+Q)
        inputManager_->registerShortcut(
            "quit",
            "<Control>q",
            [this]() {
                close();
            }
        );

        // Raccourci pour actualiser la liste des affichages (F5)
        inputManager_->registerShortcut(
            "refresh_displays",
            "F5",
            [this]() {
                if (displayManager_) {
                    displayManager_->updateDisplays();
                    updateDisplayList();
                    updateStatus("Liste des affichages actualisée");
                }
            }
        );

        // Raccourci pour ouvrir la boîte de dialogue À propos (F1)
        inputManager_->registerShortcut(
            "show_about",
            "F1",
            [this]() {
                showAboutDialog();
            }
        );

        // Raccourci pour appliquer les modifications (Ctrl+S)
        inputManager_->registerShortcut(
            "apply_changes",
            "<Control>s",
            [this]() {
                onApplyClicked();
            }
        );

    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'enregistrement des raccourcis: " << e.what() << std::endl;
        showError("Erreur lors de la configuration des raccourcis clavier");
    }
}

bool MainWindow::on_delete_event(GdkEventAny* event) {
    // Sauvegarder les paramètres avant de quitter
    saveSettings();
    
    // Appeler la méthode de la classe de base
    return Gtk::Window::on_delete_event(event);
}

void MainWindow::applySettings()
{
    // Appliquer les paramètres du curseur
    if (cursorManager_) {
        // Mettre à jour l'échelle
        if (cursorTab_.scaleSlider) {
            double scale = cursorTab_.scaleSlider->get_value();
            cursorManager_->setScale(scale);
        }
        
        // Mettre à jour les FPS
        if (cursorTab_.fpsSpin) {
            int fps = cursorTab_.fpsSpin->get_value_as_int();
            cursorManager_->setAnimationSpeed(fps);
        }
    }
    
    // Mettre à jour les paramètres d'affichage
    onDisplaySettingsChanged();
    
    // Mettre à jour l'aperçu du curseur
    updateCursorPreview();
    
    // Marquer les paramètres comme non modifiés
    settingsModified_ = false;
    
    // Afficher un message de confirmation
    updateStatus("Paramètres appliqués avec succès");
}

void MainWindow::loadSettings() {
    try {
        // Charger les paramètres depuis un fichier de configuration
        auto settings = Gio::Settings::create("org.openyolo");
        
        // Paramètres du curseur
        currentCursorPath_ = settings->get_string("cursor-path");
        if (cursorTab_.fpsSpin) {
            cursorTab_.fpsSpin->set_value(settings->get_double("cursor-fps"));
        }
        
        // Paramètres d'affichage
        if (displayTab_.followMouseCheck) {
            displayTab_.followMouseCheck->set_active(settings->get_boolean("follow-mouse"));
        }
        
        updateCursorPreview();
        updateDisplayList();
    } catch (const Glib::Error& e) {
        std::cerr << "Erreur lors du chargement des paramètres: " << e.what() << std::endl;
    }
}

void MainWindow::saveSettings() {
    try {
        // Sauvegarder les paramètres dans un fichier de configuration
        auto settings = Gio::Settings::create("org.openyolo");
        
        // Paramètres du curseur
        settings->set_string("cursor-path", currentCursorPath_);
        if (cursorTab_.fpsSpin) {
            settings->set_double("cursor-fps", cursorTab_.fpsSpin->get_value());
        }
        
        // Paramètres d'affichage
        if (displayTab_.followMouseCheck) {
            settings->set_boolean("follow-mouse", displayTab_.followMouseCheck->get_active());
        }
        
        // Pas besoin d'appeler sync(), les changements sont automatiquement sauvegardés
    } catch (const Glib::Error& e) {
        std::cerr << "Erreur lors de la sauvegarde des paramètres: " << e.what() << std::endl;
    }
}

void MainWindow::updateDisplayList() {
    if (!displayTab_.displayTreeView || !displayManager_) {
        return;
    }
    
    auto model = displayTab_.displayList;
    if (!model) {
        return;
    }
    
    model->clear();
    
    try {
        // Obtenir la liste des écrans disponibles
        auto displays = displayManager_->getDisplays();
        
        for (const auto& display : displays) {
            Gtk::TreeModel::Row row = *(model->append());
            row[displayTab_.columns.name] = display.name;
            row[displayTab_.columns.resolution] = std::to_string(display.width) + "x" + std::to_string(display.height);
            row[displayTab_.columns.isPrimary] = display.isPrimary;
            row[displayTab_.columns.scale] = display.scale;
            row[displayTab_.columns.displayPtr] = display.ptr;
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la mise à jour de la liste des écrans: " << e.what() << std::endl;
    }
}

void MainWindow::updateCursorPreview() {
    if (!cursorTab_.previewArea) {
        return;
    }
    
    // Forcer le redessin de la zone d'aperçu
    cursorTab_.previewArea->queue_draw();
}

void MainWindow::onDisplayChanged() {
    // Mettre à jour l'interface utilisateur lorsque l'affichage change
    updateDisplayList();
}

void MainWindow::showAboutDialog()
{
    Gtk::AboutDialog dialog;
    
    // Configurer la boîte de dialogue
    dialog.set_transient_for(*this);
    dialog.set_modal(true);
    dialog.set_program_name("Open-Yolo");
    dialog.set_version("1.0.0");
    dialog.set_copyright("Copyright 2023 Votre Nom");
    dialog.set_license_type(Gtk::LICENSE_GPL_3_0);
    dialog.set_website("https://github.com/votrecompte/open-yolo");
    dialog.set_website_label("Visitez notre dépôt GitHub");
    
    // Afficher la boîte de dialogue
    dialog.run();
}

void MainWindow::setAnimationSpeed(int fps)
{
    if (cursorManager_) {
        cursorManager_->setAnimationSpeed(fps);
        
        // Mettre à jour l'interface utilisateur si nécessaire
        if (cursorTab_.fpsSpin) {
            cursorTab_.fpsSpin->set_value(fps);
        }
        
        updateStatus("Vitesse d'animation définie à " + std::to_string(fps) + " FPS");
    }
}

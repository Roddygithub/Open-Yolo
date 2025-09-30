#!/usr/bin/env python3
"""
Script de génération des icônes pour OpenYolo
Ce script génère des icônes dans différentes tailles pour l'application OpenYolo.
"""

import os
import sys
from pathlib import Path
from typing import Tuple, Dict, Optional

# Vérifier que Pillow est installé
try:
    from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageEnhance
    from PIL.Image import Resampling
    HAS_PILLOW = True
except ImportError:
    HAS_PILLOW = False
    print("Erreur: Le module Pillow est requis pour générer les icônes.")
    print("Veuillez l'installer avec: pip install --upgrade Pillow")
    sys.exit(1)

# Chemins des répertoires
BASE_DIR = Path(__file__).parent.parent
RESOURCES_DIR = BASE_DIR / 'resources'
ICONS_DIR = RESOURCES_DIR / 'icons'

# Configuration des couleurs
THEME_COLORS = {
    'primary': '#4CAF50',    # Vert principal
    'primary_light': '#81C784',  # Vert clair
    'primary_dark': '#388E3C',   # Vert foncé
    'accent': '#8BC34A',     # Vert d'accent
    'text_light': '#FFFFFF',  # Texte clair
    'text_dark': '#1B5E20',  # Texte foncé
    'shadow': '#00000040',   # Ombre
    'highlight': '#FFFFFF40'  # Reflet
}

def hex_to_rgba(hex_color: str, alpha: int = 255) -> Tuple[int, int, int, int]:
    """Convertit une couleur hexadécimale en tuple RGBA."""
    hex_color = hex_color.lstrip('#')
    if len(hex_color) == 3:
        hex_color = ''.join([c * 2 for c in hex_color])
    r, g, b = (int(hex_color[i:i+2], 16) for i in (0, 2, 4))
    return (r, g, b, alpha)


def create_icon(size: int, output_path: str) -> bool:
    """
    Crée une icône avec le logo OpenYolo à la taille spécifiée.
    
    Args:
        size: Taille de l'icône en pixels (carrée)
        output_path: Chemin de sortie pour l'icône générée
        
    Returns:
        bool: True si la création a réussi, False sinon
    """
    if not HAS_PILLOW:
        print("Erreur: Le module Pillow est requis pour générer les icônes.")
        return False
    
    try:
        # Créer une nouvelle image avec fond transparent
        img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img, 'RGBA')
        
        # Calcul des dimensions
        margin = max(2, size // 20)  # Marge minimale de 2px ou 5% de la taille
        outer_radius = (size // 2) - margin
        center = (size // 2, size // 2)
        
        # 1. Créer une ombre portée
        if size > 32:  # Uniquement pour les icônes suffisamment grandes
            shadow_layer = Image.new('RGBA', (size, size), (0, 0, 0, 0))
            shadow_draw = ImageDraw.Draw(shadow_layer, 'RGBA')
            shadow_draw.ellipse(
                [
                    (margin + 2, margin + 2),
                    (size - margin + 2, size - margin + 2)
                ],
                fill=hex_to_rgba(THEME_COLORS['shadow'], 100)
            )
            shadow_layer = shadow_layer.filter(ImageFilter.GaussianBlur(radius=size//32))
            img = Image.alpha_composite(img, shadow_layer)
        
        # 2. Dessiner le cercle extérieur avec un dégradé
        # Pour les petites icônes, on utilise une couleur unie pour plus de simplicité
        if size <= 32:
            draw.ellipse(
                [(margin, margin), (size - margin, size - margin)],
                fill=THEME_COLORS['primary'],
                outline=THEME_COLORS['primary_dark'],
                width=max(1, size // 64)
            )
        else:
            # Pour les grandes icônes, on crée un dégradé
            for i in range(outer_radius, 0, -1):
                alpha = int(255 * (i / outer_radius) ** 2)
                color = hex_to_rgba(THEME_COLORS['primary'], alpha)
                draw.ellipse(
                    [
                        (center[0] - i, center[1] - i),
                        (center[0] + i, center[1] + i)
                    ],
                    outline=color,
                    width=1
                )
        
        # 3. Cercle intérieur avec effet de profondeur
        inner_radius = int(outer_radius * 0.7)
        if size <= 32:
            draw.ellipse(
                [
                    (center[0] - inner_radius, center[1] - inner_radius),
                    (center[0] + inner_radius, center[1] + inner_radius)
                ],
                fill=THEME_COLORS['primary_light']
            )
        else:
            # Dégradé pour le cercle intérieur
            for i in range(inner_radius, 0, -1):
                alpha = int(200 * (i / inner_radius) ** 0.7)
                color = hex_to_rgba(THEME_COLORS['primary_light'], alpha)
                draw.ellipse(
                    [
                        (center[0] - i, center[1] - i),
                        (center[0] + i, center[1] + i)
                    ],
                    outline=color,
                    width=1
                )
        
        # 4. Ajouter le symbole 'Y' au centre
        try:
            font_size = int(size * 0.5)
            font = None
            
            # Essayer de charger une police appropriée
            font_names = [
                'Arial', 'Arial Unicode MS', 'DejaVu Sans', 'Liberation Sans',
                'Nimbus Sans', 'FreeSans', 'Droid Sans', 'Roboto', 'Ubuntu'
            ]
            
            for font_name in font_names:
                try:
                    font = ImageFont.truetype(font_name, font_size)
                    break
                except (IOError, OSError):
                    continue
            
            if font is None:
                # Utiliser la police par défaut si aucune autre n'est disponible
                font = ImageFont.load_default()
                # Ajuster la taille pour la police par défaut
                font_size = size // 2
            
            # Positionner le texte 'Y' au centre
            text = 'Y'
            text_bbox = draw.textbbox((0, 0), text, font=font)
            text_width = text_bbox[2] - text_bbox[0]
            text_height = text_bbox[3] - text_bbox[1]
            
            # Calculer la position avec un léger décalage vers le haut pour un meilleur équilibre visuel
            text_x = (size - text_width) // 2
            text_y = (size - text_height) // 2 - (size // 20)
            
            # Ajouter une ombre portée au texte (pour les grandes icônes)
            if size > 48:
                shadow_color = hex_to_rgba(THEME_COLORS['shadow'], 150)
                draw.text(
                    (text_x + 1, text_y + 1), 
                    text, 
                    font=font, 
                    fill=shadow_color
                )
            
            # Dessiner le texte principal
            draw.text(
                (text_x, text_y),
                text,
                font=font,
                fill=THEME_COLORS['text_dark']
            )
            
            # Ajouter un effet de reflet (pour les grandes icônes)
            if size > 64:
                # Créer un masque de dégradé pour le reflet
                reflection = Image.new('L', (size, size), 0)
                reflection_draw = ImageDraw.Draw(reflection)
                
                # Dessiner un dégradé linéaire du haut vers le bas
                for y in range(size // 4):
                    alpha = int(50 * (1 - (y / (size // 4))))
                    reflection_draw.line(
                        [(0, y), (size, y)],
                        fill=alpha,
                        width=1
                    )
                
                # Appliquer le reflet à l'image
                reflection = reflection.rotate(180)
                img.putalpha(reflection)
        
        except Exception as e:
            print(f"Erreur lors de l'ajout du texte: {e}")
            return False
        
        # 5. Ajouter un effet de bordure lumineuse (pour les grandes icônes)
        if size > 48:
            border_layer = Image.new('RGBA', (size, size), (0, 0, 0, 0))
            border_draw = ImageDraw.Draw(border_layer, 'RGBA')
            
            # Dessiner un contour extérieur avec un dégradé
            for i in range(1, max(2, size // 64) + 1):
                alpha = int(100 * (1 - (i / (size // 32))))
                if alpha > 0:
                    border_draw.ellipse(
                        [
                            (margin - i, margin - i),
                            (size - margin + i, size - margin + i)
                        ],
                        outline=hex_to_rgba(THEME_COLORS['highlight'], alpha),
                        width=1
                    )
            
            # Fusionner avec l'image principale
            img = Image.alpha_composite(img, border_layer)
        
        # 6. Appliquer un léger flou pour adoucir les bords (pour les grandes icônes)
        if size > 64:
            img = img.filter(ImageFilter.SMOOTH)
        
        # Créer le répertoire de destination s'il n'existe pas
        output_dir = os.path.dirname(output_path)
        if output_dir:
            os.makedirs(output_dir, exist_ok=True)
        
        # Enregistrer l'icône avec une compression optimisée
        img.save(output_path, 'PNG', optimize=True, compress_level=9)
        print(f"Icône générée : {output_path}")
        return True
        
    except Exception as e:
        print(f"Erreur lors de la création de l'icône {size}x{size}: {e}")
        import traceback
        traceback.print_exc()
        return False

# Tailles standard pour les icônes Linux
def generate_icon_set() -> bool:
    """
    Génère un ensemble complet d'icônes pour différentes tailles.
    
    Returns:
        bool: True si toutes les icônes ont été générées avec succès, False sinon
    """
    if not HAS_PILLOW:
        print("Erreur: Le module Pillow est requis pour générer les icônes.")
        return False
    
    try:
        # Tailles standard pour les icônes Linux (selon la spécification freedesktop.org)
        icon_sizes = [
            16, 22, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 1024
        ]
        
        # Créer le répertoire de base des icônes
        ICONS_DIR.mkdir(parents=True, exist_ok=True)
        
        # Générer les icônes pour chaque taille
        success_count = 0
        for size in icon_sizes:
            # Déterminer le répertoire de destination en fonction de la taille
            if size <= 32:
                theme_dir = 'hicolor'
                size_dir = f"{size}x{size}"
            elif size <= 128:
                theme_dir = 'hicolor'
                size_dir = f"{size}x{size}"
            else:
                theme_dir = 'hicolor'
                size_dir = 'scalable'
            
            # Créer le chemin de sortie
            output_dir = ICONS_DIR / theme_dir / size_dir / 'apps'
            output_path = output_dir / 'OpenYolo.png'
            
            # Générer l'icône
            if create_icon(size, str(output_path)):
                success_count += 1
        
        # Créer également une version SVG pour une meilleure qualité
        svg_path = ICONS_DIR / 'hicolor' / 'scalable' / 'apps' / 'OpenYolo.svg'
        create_svg_icon(str(svg_path))
        
        # Créer un lien symbolique pour la version 256x256 (utilisée par défaut dans certains cas)
        try:
            target_icon = ICONS_DIR / 'hicolor' / '256x256' / 'apps' / 'OpenYolo.png'
            if not target_icon.exists() and (ICONS_DIR / 'hicolor' / '512x512' / 'apps' / 'OpenYolo.png').exists():
                img = Image.open(ICONS_DIR / 'hicolor' / '512x512' / 'apps' / 'OpenYolo.png')
                img = img.resize((256, 256), Resampling.LANCZOS)
                target_icon.parent.mkdir(parents=True, exist_ok=True)
                img.save(target_icon, 'PNG', optimize=True)
                print(f"Icône redimensionnée : {target_icon}")
        except Exception as e:
            print(f"Avertissement: Impossible de créer l'icône 256x256: {e}")
        
        print(f"\nGénération terminée: {success_count}/{len(icon_sizes)} tailles d'icônes créées avec succès.")
        return success_count == len(icon_sizes)
        
    except Exception as e:
        print(f"Erreur lors de la génération des icônes: {e}")
        import traceback
        traceback.print_exc()
        return False

def create_svg_icon(output_path: str) -> bool:
    """
    Crée une version vectorielle (SVG) de l'icône.
    
    Args:
        output_path: Chemin de sortie pour le fichier SVG
        
    Returns:
        bool: True si la création a réussi, False sinon
    """
    try:
        # Créer le répertoire de destination s'il n'existe pas
        output_dir = os.path.dirname(output_path)
        if output_dir:
            os.makedirs(output_dir, exist_ok=True)
        
        # Contenu du fichier SVG avec des effets avancés
        svg_content = f'''<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg width="1024" height="1024" viewBox="0 0 1024 1024" 
     xmlns="http://www.w3.org/2000/svg" 
     xmlns:xlink="http://www.w3.org/1999/xlink">
    
    <!-- Définitions des dégradés et filtres -->
    <defs>
        <!-- Dégradé pour le cercle extérieur -->
        <radialGradient id="outerGradient" cx="50%" cy="50%" r="70%" fx="30%" fy="30%">
            <stop offset="0%" stop-color="{THEME_COLORS['primary']}" stop-opacity="1" />
            <stop offset="100%" stop-color="{THEME_COLORS['primary_dark']}" stop-opacity="1" />
        </radialGradient>
        
        <!-- Dégradé pour le cercle intérieur -->
        <radialGradient id="innerGradient" cx="50%" cy="50%" r="70%" fx="40%" fy="40%">
            <stop offset="0%" stop-color="{THEME_COLORS['primary_light']}" stop-opacity="1" />
            <stop offset="100%" stop-color="{THEME_COLORS['primary']}" stop-opacity="1" />
        </radialGradient>
        
        <!-- Filtre pour l'ombre portée -->
        <filter id="dropShadow" x="-20%" y="-20%" width="140%" height="140%">
            <feGaussianBlur in="SourceAlpha" stdDeviation="15" result="blur" />
            <feOffset in="blur" dx="5" dy="5" result="offsetBlur" />
            <feComponentTransfer in="offsetBlur" result="shadow">
                <feFuncA type="linear" slope="0.2" />
            </feComponentTransfer>
            <feMerge> 
                <feMergeNode in="shadow" />
                <feMergeNode in="SourceGraphic" />
            </feMerge>
        </filter>
        
        <!-- Filtre pour le reflet -->
        <filter id="reflection" x="0" y="0" width="100%" height="50%">
            <feGaussianBlur in="SourceGraphic" stdDeviation="2" result="blur" />
            <feComponentTransfer in="blur" result="highlight">
                <feFuncA type="table" tableValues="0 0.1 0.3 0.1 0" />
            </feComponentTransfer>
        </filter>
    </defs>
    
    <!-- Groupe principal avec l'ombre portée -->
    <g filter="url(#dropShadow)">
        <!-- Cercle extérieur avec bordure -->
        <circle cx="512" cy="512" r="450" 
                fill="url(#outerGradient)" 
                stroke="{THEME_COLORS['primary_dark']}" 
                stroke-width="15" />
        
        <!-- Cercle intérieur avec dégradé -->
        <circle cx="512" cy="512" r="300" 
                fill="url(#innerGradient)" 
                stroke="#FFFFFF20" 
                stroke-width="5" />
        
        <!-- Symbole Y -->
        <path d="M512 250 L412 450 L462 450 L512 350 L562 450 L612 450 L512 250 Z" 
              fill="{THEME_COLORS['text_dark']}" 
              filter="drop-shadow(2px 2px 4px rgba(0,0,0,0.3))"/>
        
        <!-- Effet de reflet -->
        <ellipse cx="400" cy="400" rx="250" ry="120" 
                 fill="white" fill-opacity="0.15" 
                 transform="rotate(-30, 400, 400)" 
                 filter="url(#reflection)" />
    </g>
    
    <!-- Texte optionnel sous l'icône (pour les très grandes tailles) -->
    <text x="512" y="950" 
          font-family="Arial, sans-serif" 
          font-size="60" 
          text-anchor="middle" 
          fill="{THEME_COLORS['text_dark']}" 
          font-weight="bold">OpenYolo</text>
</svg>
'''
        # Écrire le fichier SVG
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(svg_content)
        
        print(f"Icône SVG générée : {output_path}")
        return True
        
    except Exception as e:
        print(f"Erreur lors de la création du fichier SVG: {e}")
        import traceback
        traceback.print_exc()
        return False

def main() -> int:
    """Fonction principale du script."""
    print("=" * 70)
    print(f"Génération des icônes OpenYolo dans : {ICONS_DIR}")
    print("=" * 70)
    
    # Vérifier les dépendances
    if not HAS_PILLOW:
        print("\nERREUR: Le module Pillow est requis pour générer les icônes.")
        print("Veuillez l'installer avec: pip install --upgrade Pillow")
        return 1
    
    # Générer les icônes
    success = generate_icon_set()
    
    if success:
        print("\n" + "=" * 70)
        print("GÉNÉRATION TERMINÉE AVEC SUCCÈS")
        print("=" * 70)
        print("\nPour installer les icônes, exécutez la commande suivante :")
        print(f"  sudo {BASE_DIR / 'scripts' / 'setup_icons.sh'}")
        return 0
    else:
        print("\n" + "!" * 70)
        print("ERREUR: La génération des icônes a échoué")
        print("!" * 70)
        return 1


if __name__ == "__main__":
    sys.exit(main())

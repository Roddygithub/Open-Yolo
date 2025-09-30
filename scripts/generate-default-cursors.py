#!/usr/bin/env python3
"""
Script pour générer les curseurs par défaut d'Open-Yolo
"""

from PIL import Image, ImageDraw, ImageFont
import os
import sys

def create_default_cursor(size=32):
    """Créer un curseur par défaut (flèche)"""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Points de la flèche
    points = [
        (2, 2),
        (2, size - 8),
        (size // 3, size - size // 3),
        (size // 2, size - 2),
        (size - 2, size // 2),
        (size // 2, size // 3),
        (size - 8, 2)
    ]
    
    # Bordure noire
    draw.polygon(points, fill=(0, 0, 0, 255))
    
    # Intérieur blanc
    points_inner = [(p[0] + 2, p[1] + 2) for p in points[:-1]]
    draw.polygon(points_inner, fill=(255, 255, 255, 255))
    
    return img

def create_pointer_cursor(size=32):
    """Créer un curseur de pointage (main)"""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Main simplifiée
    # Paume
    draw.ellipse([size//4, size//2, 3*size//4, size-4], fill=(255, 220, 180, 255), outline=(0, 0, 0, 255), width=2)
    
    # Index pointé
    draw.rectangle([size//2-3, size//4, size//2+3, size//2], fill=(255, 220, 180, 255), outline=(0, 0, 0, 255), width=2)
    draw.ellipse([size//2-4, size//4-8, size//2+4, size//4+4], fill=(255, 220, 180, 255), outline=(0, 0, 0, 255), width=2)
    
    return img

def create_animated_cursor(size=32, frames=8):
    """Créer un curseur animé (cercle rotatif)"""
    images = []
    
    for i in range(frames):
        img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        # Cercle de base
        center = size // 2
        radius = size // 3
        
        # Dessiner le cercle
        draw.ellipse([center - radius, center - radius, 
                     center + radius, center + radius], 
                     outline=(100, 100, 100, 255), width=2)
        
        # Arc coloré rotatif
        angle_start = (i * 360 // frames) % 360
        angle_end = (angle_start + 90) % 360
        
        # Couleurs arc-en-ciel
        colors = [
            (255, 0, 0),     # Rouge
            (255, 127, 0),   # Orange
            (255, 255, 0),   # Jaune
            (0, 255, 0),     # Vert
            (0, 0, 255),     # Bleu
            (75, 0, 130),    # Indigo
            (148, 0, 211),   # Violet
            (255, 0, 127),   # Rose
        ]
        
        color = colors[i % len(colors)]
        
        # Dessiner l'arc
        draw.arc([center - radius, center - radius, 
                 center + radius, center + radius],
                start=angle_start, end=angle_end,
                fill=color + (255,), width=4)
        
        # Point central
        draw.ellipse([center - 3, center - 3, center + 3, center + 3],
                    fill=(255, 255, 255, 255), outline=(0, 0, 0, 255))
        
        images.append(img)
    
    return images

def create_loading_cursor(size=32, frames=12):
    """Créer un curseur de chargement (spinner)"""
    images = []
    
    for i in range(frames):
        img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        center = size // 2
        radius = size // 3
        
        # Dessiner les points du spinner
        for j in range(12):
            angle = (i + j) * 30  # 30 degrés entre chaque point
            import math
            x = center + int(radius * math.cos(math.radians(angle)))
            y = center + int(radius * math.sin(math.radians(angle)))
            
            # Opacité décroissante
            alpha = int(255 * (1 - j / 12))
            size_dot = 3 if j < 3 else 2
            
            draw.ellipse([x - size_dot, y - size_dot, 
                         x + size_dot, y + size_dot],
                        fill=(100, 150, 255, alpha))
        
        images.append(img)
    
    return images

def save_gif(images, filename, duration=100):
    """Sauvegarder une liste d'images en GIF animé"""
    images[0].save(
        filename,
        save_all=True,
        append_images=images[1:],
        duration=duration,
        loop=0,
        optimize=True
    )

def main():
    # Créer le dossier de sortie
    output_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'assets', 'cursors')
    os.makedirs(output_dir, exist_ok=True)
    
    print("Generation des curseurs par defaut...")
    print(f"Dossier de sortie : {output_dir}")
    print()
    
    # Générer les curseurs statiques
    print("1. Generation du curseur par defaut...")
    default_cursor = create_default_cursor(32)
    default_cursor.save(os.path.join(output_dir, 'default.png'))
    print("   OK default.png cree")
    
    print("2. Generation du curseur pointer...")
    pointer_cursor = create_pointer_cursor(32)
    pointer_cursor.save(os.path.join(output_dir, 'pointer.png'))
    print("   OK pointer.png cree")
    
    # Générer les curseurs animés
    print("3. Generation du curseur anime...")
    animated_frames = create_animated_cursor(32, 8)
    save_gif(animated_frames, os.path.join(output_dir, 'animated-cursor.gif'), duration=100)
    print("   OK animated-cursor.gif cree (8 frames, 30 FPS)")
    
    print("4. Generation du curseur de chargement...")
    loading_frames = create_loading_cursor(32, 12)
    save_gif(loading_frames, os.path.join(output_dir, 'loading.gif'), duration=80)
    print("   OK loading.gif cree (12 frames, 37.5 FPS)")
    
    # Générer des versions HD (64x64)
    print("5. Generation des versions HD (64x64)...")
    default_hd = create_default_cursor(64)
    default_hd.save(os.path.join(output_dir, 'default-hd.png'))
    
    pointer_hd = create_pointer_cursor(64)
    pointer_hd.save(os.path.join(output_dir, 'pointer-hd.png'))
    
    animated_hd = create_animated_cursor(64, 8)
    save_gif(animated_hd, os.path.join(output_dir, 'animated-cursor-hd.gif'), duration=100)
    print("   OK Versions HD creees")
    
    print()
    print("Generation terminee avec succes !")
    print(f"7 curseurs crees dans {output_dir}")
    print()
    print("Curseurs disponibles :")
    print("   - default.png (32x32)")
    print("   - pointer.png (32x32)")
    print("   - animated-cursor.gif (32x32, anime)")
    print("   - loading.gif (32x32, anime)")
    print("   - default-hd.png (64x64)")
    print("   - pointer-hd.png (64x64)")
    print("   - animated-cursor-hd.gif (64x64, anime)")

if __name__ == '__main__':
    try:
        main()
    except ImportError as e:
        print("ERREUR : Pillow n'est pas installe")
        print("Installation : pip install Pillow")
        sys.exit(1)
    except Exception as e:
        print(f"ERREUR : {e}")
        sys.exit(1)

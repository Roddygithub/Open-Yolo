#!/usr/bin/env python3
"""
Script pour générer des curseurs personnalisés pour Open-Yolo
"""
import numpy as np
from PIL import Image, ImageDraw
import os


def create_pointer_cursor():
    """Crée un curseur de pointeur simple"""
    size = 32
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Dessiner un curseur de pointeur simple
    draw.polygon([(0, 0), (size - 1, 0), (0, size - 1)], fill="black")
    draw.polygon([(1, 1), (size - 2, 1), (1, size - 2)], fill="white")

    # Ajouter un contour noir fin
    draw.line([(0, 0), (size - 1, 0), (0, size - 1), (0, 0)], fill="black", width=1)

    return img


def create_hand_cursor():
    """Crée un curseur de main"""
    size = 32
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Dessiner une main simple
    draw.ellipse((8, 8, 24, 24), fill="white", outline="black")

    # Ajouter un contour
    draw.ellipse((8, 8, 24, 24), outline="black", width=1)

    return img


def create_crosshair_cursor():
    """Crée un curseur de viseur"""
    size = 32
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Dessiner un viseur simple
    center = size // 2
    radius = 10

    # Lignes du viseur
    draw.line(
        [(center - radius, center), (center + radius, center)], fill="white", width=2
    )
    draw.line(
        [(center, center - radius), (center, center + radius)], fill="white", width=2
    )

    # Contour noir pour la visibilité
    draw.line(
        [(center - radius - 1, center), (center + radius + 1, center)],
        fill="black",
        width=1,
    )
    draw.line(
        [(center, center - radius - 1), (center, center + radius + 1)],
        fill="black",
        width=1,
    )

    return img


def create_text_cursor():
    """Crée un curseur de texte (I-beam)"""
    size = 32
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Dessiner un curseur I-beam
    x = size // 2
    draw.rectangle([(x - 1, 4), (x + 1, size - 4)], fill="white", outline="black")

    return img


def save_cursor(img, filename):
    """Enregistre le curseur dans le répertoire des ressources"""
    os.makedirs("resources/cursors", exist_ok=True)
    img.save(f"resources/cursors/{filename}.png", "PNG")


def main():
    """Fonction principale"""
    print("Génération des curseurs personnalisés...")

    # Créer les curseurs
    cursors = [
        (create_pointer_cursor, "pointer"),
        (create_hand_cursor, "hand"),
        (create_crosshair_cursor, "crosshair"),
        (create_text_cursor, "text"),
    ]

    # Enregistrer les curseurs
    for cursor_func, name in cursors:
        img = cursor_func()
        save_cursor(img, name)
        print(f"Curseur généré: {name}.png")

    print("Tous les curseurs ont été générés avec succès dans resources/cursors/")


if __name__ == "__main__":
    main()

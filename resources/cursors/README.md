# Custom Cursors

This directory contains custom cursor files for the Open-Yolo application.

## Supported Formats
- PNG (recommended for best quality with transparency)
- XBM (legacy X11 format)
- CUR (Windows cursor format, may have limited support)

## Recommended Specifications
- Size: 32x32 or 64x64 pixels
- Format: PNG with alpha channel for transparency
- Hotspot: Should be set appropriately for the cursor type

## Included Cursors
- `hand_cursor.png`: A hand pointer cursor
- `crosshair_cursor.png`: A precision crosshair cursor
- `text_cursor.png`: A text I-beam cursor

## Adding Custom Cursors
1. Place your cursor files in this directory
2. Update the `CMakeLists.txt` to include the new files
3. Reference them in your code using the relative path: `resources/cursors/your_cursor.png`

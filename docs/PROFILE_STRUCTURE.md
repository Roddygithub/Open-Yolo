# Profile Structure

The profile files are stored in JSON format.

## Fields

*   `name`: The name of the profile (string).
*   `theme`: The name of the cursor theme (string).
*   `size`: The size of the cursor (integer).
*   `useCustomTheme`: Whether to use a custom theme (boolean).
*   `customThemePath`: The path to the custom theme (string, optional).
*   `cursorMapping`: A map of cursor types to cursor file paths (object, optional).
*   `checksum`: A CRC32 checksum of the profile data (integer).

## Example

```json
{
    "cursorMapping": {
        "Hand": "/path/to/hand.png",
        "Pointer": "/path/to/pointer.png"
    },
    "name": "My Profile",
    "size": 32,
    "theme": "Adwaita",
    "useCustomTheme": true,
    "customThemePath": "/path/to/my/theme",
    "checksum": 123456789
}
```

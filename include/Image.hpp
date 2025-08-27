#pragma once
#include <vector>
#include <string>

class Image {
public:
    Image() = default;
    ~Image() = default;

    // Ajoute les membres réels si connus, exemple :
    int width = 0;
    int height = 0;
    std::vector<unsigned char> pixels;
};


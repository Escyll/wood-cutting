#ifndef CATALOG_H
#define CATALOG_H

#include <map>
#include <string>
#include <filesystem>
#include <iostream>

#include "Renderer/Textures.h"

using TextureCatalog = std::map<std::string, unsigned int>;

TextureCatalog createTextureCatalog(const std::filesystem::path& location)
{
    namespace fs = std::filesystem;
    std::cerr << "\nBuilding catalog for " << location << std::endl;
    std::map<std::string, unsigned int> catalog;
    for (const auto& entry : fs::recursive_directory_iterator(location))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".png")
        {
            auto relative = fs::relative(entry.path(), location).string();
            std::cerr << "Loading " << relative << std::endl;
            unsigned int textureName;
            if (loadTexture(entry.path(), textureName))
            {
                catalog[relative] = textureName;
            }
            else 
            {
                std::cerr << "Failed to load" << std::endl;
            }
        }
    }
    std::cerr << "Done building catalog " << location << std::endl;
    return catalog;
}

inline unsigned int getTexture(TextureCatalog& catalog, const std::string& name)
{
    #ifndef NDEBUG
    if (not catalog.contains(name))
        std::cerr << "Catalog does not contain " << name << std::endl;
    #endif
    return catalog[name];
}

#endif

#ifndef CATALOG_H
#define CATALOG_H

#include <map>
#include <string>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>

#include "Renderer/Shaders.h" //TODO: Move readFile out of Shaders.h
#include "Renderer/Textures.h"

using TextureCatalog = std::map<std::string, unsigned int>;

TextureCatalog createTextureCatalog(const std::filesystem::path& location)
{
    namespace fs = std::filesystem;
    std::cerr << "\nBuilding texture catalog for " << location << std::endl;
    TextureCatalog catalog;
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
    std::cerr << "Done building texture catalog " << location << std::endl;
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

using AnimationCatalog = std::map<std::string, Animation>;

AnimationCatalog createAnimationCatalog(const std::filesystem::path& location)
{
    using json = nlohmann::json;
    namespace fs = std::filesystem;
    std::cerr << "\nBuilding animation catalog for " << location << std::endl;
    AnimationCatalog catalog;
    for (const auto& entry : fs::recursive_directory_iterator(location))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            auto relative = fs::relative(entry.path(), location);
            std::cerr << "Loading " << relative << std::endl;
            auto file = readFile(entry.path());
            auto jsonAtlasDescriptor = json::parse(file);
            std::vector<Frame> frames;
            for (auto &frame : jsonAtlasDescriptor["frames"])
            {
                auto jsonFrame = frame["frame"];
                frames.push_back({
                        jsonFrame["x"].template get<int>(),
                        jsonFrame["y"].template get<int>(),
                        jsonFrame["w"].template get<int>(),
                        jsonFrame["h"].template get<int>(),
                        frame["duration"].template get<int>() / 1000.f
                        });
            }
            for (auto &tag : jsonAtlasDescriptor["meta"]["frameTags"])
            {
                Animation animation;
                animation.name = relative.parent_path().string() + "/" + tag["name"].template get<std::string>();
                for (auto i = tag["from"].template get<int>(); i <= tag["to"].template get<int>(); i++)
                {
                    animation.frames.push_back(frames[i]);
                }
                auto durationFolder = [](float accum, Frame &frame) { return accum + frame.duration; };
                animation.duration = std::accumulate(animation.frames.begin(), animation.frames.end(), 0.f, durationFolder);
                animation.texture = jsonAtlasDescriptor["meta"]["image"].template get<std::string>();
                catalog[animation.name] = animation;
                std::cerr << "Added animation " << animation.name << std::endl;
                std::cerr << animation.name << " " << animation.texture << " " << animation.duration << std::endl;
            }
        }
    }
    std::cerr << "Done building animation catalog " << location << std::endl;
    return catalog;
}

Animation& getAnimation(AnimationCatalog& catalog, const std::string name)
{
    #ifndef NDEBUG
    if (not catalog.contains(name))
        std::cerr << "Catalog does not contain " << name << std::endl;
    #endif
    return catalog[name];
}

#endif

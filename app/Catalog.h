#pragma once

#include <map>
#include <string>
#include <filesystem>

#include "Renderer/Textures.h"

using TextureCatalog = std::map<std::string, unsigned int>;
using AnimationCatalog = std::map<std::string, AnimationSequence>;

TextureCatalog createTextureCatalog(const std::filesystem::path& location);
unsigned int getTexture(TextureCatalog& catalog, const std::string& name);

AnimationCatalog createAnimationCatalog(const std::filesystem::path& location);
AnimationSequence& getAnimation(AnimationCatalog& catalog, const std::string name);

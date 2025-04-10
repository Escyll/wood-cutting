#ifndef RENDERER_TEXTURES_H
#define RENDERER_TEXTURES_H

#include <glad/glad.h>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool loadTexture(const std::string& path, unsigned int& texture)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture" << std::endl;
        return false;
    }
    stbi_image_free(data);
    return true;
}

struct Texture {
    unsigned int handle;
    int width;
    int height;
};

struct Frame {
    int x;
    int y;
    int width;
    int height;
    float duration;
};

struct Animation {
    std::string name;
    std::string texture;
    float duration;
    std::vector<Frame> frames;
};

struct ActiveAnimation {
    std::string animation;
    float currentTime;
};

#endif

#pragma once

#include <string>
#include <map>
#include <deque>

struct BMFontInfo
{
    std::string face;
    int size;
    bool bold;
    bool italic;
    std::string charset;
    bool unicode;
    int stretchH;
    bool smooth;
    bool aa;
    int padding[4];
    int spacing[2];
};

struct BMFontCommon
{
    int lineHeight;
    int base;
    int scaleW;
    int scaleH;
    int pages;
    bool packed;
};

struct BMFontPage
{
    int id;
    std::string file;
};

struct BMFontChar
{
    char id;
    int x;
    int y;
    int width;
    int height;
    int xoffset;
    int yoffset;
    int xadvance;
    int page;
    int channel;
};

struct BMFont
{
    BMFontInfo info;
    BMFontCommon common;
    std::map<int, BMFontPage> pages;
    std::map<char, BMFontChar> chars;
};

std::deque<std::string> tokenize(const std::string& line);
std::pair<std::string, std::string> splitToken(const std::string& token);
BMFont loadBMFont(const std::string& font);

#include "FontRendering/BMFont.h"

#include <iostream>
#include <fstream>

std::deque<std::string> tokenize(const std::string& line)
{
    std::deque<std::string> tokens;
    std::string token;
    for (char ch : line)
    {
        if (ch == ' ')
        {
            tokens.push_back(token);
            token = "";
            continue;
        }
        token += ch;
    }
    tokens.push_back(token);
    return tokens;
}

std::pair<std::string, std::string> splitToken(const std::string& token)
{
    std::string key, value;
    bool processingKey = true;
    for (char ch : token)
    {
        if (ch == '=' && processingKey)
        {
            processingKey = false;
            continue;
        }
        if (processingKey)
        {
            key += ch;
        }
        else
        {
            value += ch;
        }
    }
    return {key, value};
}

BMFont loadBMFont(const std::string& font)
{
    BMFont bmFont;
    std::cerr << "Loading font: " << font << std::endl;
    std::fstream fin(font, std::fstream::in);
    std::string line;
    while (getline(fin, line))
    {
        auto tokens = tokenize(line);
        auto startToken = tokens.front();
        tokens.pop_front();
        if (startToken == "info")
        {
            BMFontInfo &info = bmFont.info;
            for (; !tokens.empty(); tokens.pop_front())
            {
                auto [key, value] = splitToken(tokens.front());
                if (key == "face")
                {
                    value.erase(0, 1);
                    value.erase(value.size() - 1);
                    info.face = value;
                }
                else if (key == "size")
                {
                    info.size = std::stoi(value);
                }
                else if (key == "bold")
                {
                    info.bold = std::stoi(value);
                }
                else if (key == "italic")
                {
                    info.italic = std::stoi(value);
                }
                else if (key == "charset")
                {
                    value.erase(0, 1);
                    value.erase(value.size() -1 );
                    info.charset = value;
                }
                else if (key == "unicode")
                {
                    info.unicode = std::stoi(value);
                }
                else if (key == "stretchH")
                {
                    info.stretchH = std::stoi(value);
                }
                else if (key == "smooth")
                {
                    info.smooth = std::stoi(value);
                }
                else if (key == "aa")
                {
                    info.aa = std::stoi(value);
                }
                else if (key == "padding")
                {
                    std::sscanf(value.c_str(), "%d,%d,%d,%d", &info.padding[0], &info.padding[1], &info.padding[2], &info.padding[3]);
                }
                else if (key == "spacing")
                {
                    std::sscanf(value.c_str(), "%d,%d", &info.spacing[0], &info.spacing[1]);
                }
            }
        }
        else if (startToken == "common")
        {
            BMFontCommon &common = bmFont.common;
            for (; !tokens.empty(); tokens.pop_front())
            {
                auto [key, value] = splitToken(tokens.front());
                if (key == "lineHeight")
                {
                    common.lineHeight = std::stoi(value);
                }
                else if (key == "base")
                {
                    common.base = std::stoi(value);
                }
                else if (key == "scaleW")
                {
                    common.scaleW = std::stoi(value);
                }
                else if (key == "scaleH")
                {
                    common.scaleH = std::stoi(value);
                }
                else if (key == "pages")
                {
                    common.pages = std::stoi(value);
                }
                else if (key == "packed")
                {
                    common.packed = std::stoi(value);
                }
            }
        }
        else if (startToken == "page")
        {
            BMFontPage page;
            for (; !tokens.empty(); tokens.pop_front())
            {
                auto [key, value] = splitToken(tokens.front());
                if (key == "id")
                {
                    page.id = std::stoi(value);
                }
                else if (key == "file")
                {
                    value.erase(0, 1);
                    value.erase(value.size() - 1 );
                    page.file = value;
                }
            }
            bmFont.pages[page.id] = page;
        }
        else if (startToken == "chars")
        {
            continue;
        }
        else if (startToken == "char")
        {
            bool ascii = true;
            BMFontChar fontChar;
            for (; !tokens.empty(); tokens.pop_front())
            {
                auto [key, value] = splitToken(tokens.front());
                if (key == "id")
                {
                   int id = std::stoi(value);
                   if (id > 255)
                   {
                       ascii = false;
                       break;
                   }
                   fontChar.id = id;
                }
                else if (key == "x")
                {
                    fontChar.x = std::stoi(value);
                }
                else if (key == "y")
                {
                    fontChar.y = std::stoi(value);
                }
                else if (key == "width")
                {
                    fontChar.width = std::stoi(value);
                }
                else if (key == "height")
                {
                    fontChar.height = std::stoi(value);
                }
                else if (key == "xoffset")
                {
                    fontChar.xoffset = std::stoi(value);
                }
                else if (key == "yoffset")
                {
                    fontChar.yoffset = std::stoi(value);
                }
                else if (key == "xadvance")
                {
                    fontChar.xadvance = std::stoi(value);
                }
                else if (key == "page")
                {
                    fontChar.page = std::stoi(value);
                }
                else if (key == "chnl")
                {
                    fontChar.channel = std::stoi(value);
                }
            }
            if (ascii)
            {
                bmFont.chars[fontChar.id] = fontChar;
            }
        }
    }
    std::cerr << "Done loading font" << std::endl;
    return bmFont;
}

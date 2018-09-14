#include <nlohmann/json.hpp>
#include <fstream>
#include "GGFont.h"
#include "_GGUtil.h"

namespace gg
{
GGFont::GGFont()
{
}

GGFont::~GGFont()
{
}

void GGFont::setSettings(const GGEngineSettings *settings)
{
    _settings = settings;
}

void GGFont::setTextureManager(TextureManager *textureManager)
{
    _textureManager = textureManager;
}

void GGFont::load(const std::string &path)
{
    _sprite.setTexture(_textureManager->get(path));

    _jsonFilename = _settings->getGamePath();
    _jsonFilename.append(path);
    _jsonFilename.append(".json");
}

void GGFont::draw(const std::string &text, sf::RenderTarget &target, sf::RenderStates states)
{
    nlohmann::json json;
    std::ifstream i(_jsonFilename);
    i >> json;

    std::vector<sf::IntRect> rects;
    std::vector<sf::IntRect> sourceRects;
    int maxHeight = 0;
    for (auto letter : text)
    {
        const auto &s = std::to_string((int)letter);
        auto rect = _toRect(json["frames"][s]["frame"]);
        sourceRects.push_back(_toRect(json["frames"][s]["spriteSourceSize"]));
        rects.push_back(rect);
        if (rect.height > maxHeight)
        {
            maxHeight = rect.height;
        }
    }

    int x = 0;
    float scale = 0.2f;
    for (auto i = 0; i < rects.size(); i++)
    {
        auto rect = rects[i];
        const auto& sourceRect = sourceRects[i];
        _sprite.setScale(scale, scale);
        _sprite.setTextureRect(rect);
        _sprite.setOrigin(-sourceRect.left, -sourceRect.top);
        _sprite.setColor(sf::Color(0x3ea4b5ff));
        _sprite.setPosition(x, 0);
        target.draw(_sprite, states);
        x += std::max(rect.width * scale, 10.f * scale);
    }
}

} // namespace gg

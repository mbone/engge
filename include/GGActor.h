#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "TextureManager.h"
#include "GGCostume.h"
#include "GGFont.h"
#include "GGEntity.h"

namespace gg
{
class GGActor : public GGEntity
{
public:
  explicit GGActor(TextureManager &textureManager);
  ~GGActor();

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  int getZOrder() const override { return _zorder; }

  void setCostume(const std::string &name, const std::string &sheet = "");
  GGCostume &getCostume() { return _costume; }
  const GGCostume &getCostume() const { return _costume; }

  void setTalkColor(sf::Color color) { _talkColor = color; }
  sf::Color getTalkColor() { return _talkColor; }
  void setColor(sf::Color color) { _color = color; }
  sf::Color getColor() { return _color; }
  void say(const std::string &text) { _sayText = text; }
  bool isTalking() const { return !_sayText.empty(); }

  void move(const sf::Vector2f &offset);
  sf::Vector2f getPosition() const { return _transform.transformPoint(0, 0); }
  void setPosition(const sf::Vector2f &pos);
  void setRenderOffset(const sf::Vector2i &offset) { _renderOffset = offset; }

  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const override;
  void update(const sf::Time &time);

private:
  const GGEngineSettings &_settings;
  GGCostume _costume;
  std::string _name;
  sf::Transform _transform;
  sf::Color _color;
  sf::Color _talkColor;
  sf::Vector2i _renderOffset;
  GGFont _font;
  std::string _sayText;
  int _zorder;
};
} // namespace gg

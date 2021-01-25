#pragma once

#include "CurlHelpers.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <rapidjson/document.h>

namespace disneymagic
{
class ContainerItem
{
public:
    ContainerItem(
        const rapidjson::Value& item,
        sf::RenderWindow& window,
        const sf::Font& font,
        double desired_image_width,
        double desired_image_height);

    void EnhanceScale(const sf::Vector2f& factors);
    void ResetScale();
    void Draw(const sf::Vector2f& position);

private:
    sf::Texture image;
    sf::Sprite sprite;
    sf::Text text;
    sf::RenderWindow& window;
    bool has_image;
    sf::Vector2f default_scale;
};

class Container
{
public:
    Container(const std::string& title);

    void AddElement(const ContainerItem& element);
    std::string GetTitle() const;
    size_t GetElementCount() const;
    const ContainerItem& GetElement(size_t index) const;

private:
    std::string title;
    std::vector<ContainerItem> elements;
};

}
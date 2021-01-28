#pragma once

#include "CurlHelpers.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
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
    Container(
        const rapidjson::Value& container,
        sf::RenderWindow& window,
        const sf::Font& font,
        double desired_image_width,
        double desired_image_height);

    std::string GetTitle() const;
    size_t GetItemCount() const;
    ContainerItem& GetItem(size_t index);

private:
    void PopulateItems(const rapidjson::Value& foo, sf::RenderWindow& window, const sf::Font& font, double desired_image_width, double desired_image_height);

    std::string title;
    std::vector<ContainerItem> items;
};

class ContainerFactory
{
public:
    ContainerFactory(
        sf::RenderWindow& window,
        const sf::Font& font,
        double desired_image_width,
        double desired_image_height);

    std::unique_ptr<Container> operator()(const rapidjson::Value& collection_set);

private:
    sf::RenderWindow& window;
    const sf::Font& font;
    double desired_image_width;
    double desired_image_height;
};

}
#include "Collection.h"
#include "CurlHelpers.h"
#include <iostream>
#include <exception>

namespace disneymagic
{

CollectionElement::CollectionElement(
    const rapidjson::Value& item,
    sf::RenderWindow& window,
    const sf::Font& font,
    double desired_image_width,
    double desired_image_height)
    :   image(),
        sprite(),
        text(),
        window(window),
        has_image(false),
        default_scale()
{
    auto& item_type = item["type"];
    std::string title_type_string;
    std::string image_type_string;
    if (strcmp(item_type.GetString(), "DmcSeries") == 0)
    {
        title_type_string = "series";
        image_type_string = "series";
    }
    else if (strcmp(item_type.GetString(), "DmcVideo") == 0)
    {
        title_type_string = "program";
        image_type_string = "program";
    }
    else if (strcmp(item_type.GetString(), "StandardCollection") == 0)
    {
        title_type_string = "collection";
        image_type_string = "default";
    }

    std::string title = item["text"]["title"]["full"][title_type_string.c_str()]["default"]["content"].GetString();
    std::string image_url = item["image"]["tile"]["1.78"][image_type_string.c_str()]["default"]["url"].GetString();

    text.setFillColor(sf::Color::White);
    text.setString(title);
    text.setFont(font);
    text.setCharacterSize(24);

    std::string image_buffer;
    try
    {
       curlhelpers::retrieve_file_from_URL(image_url.c_str(), image_buffer);
    }
    catch (std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
        std::cout << "Failed to retrieve image file at URL" + image_url << std::endl;
        return;
    }

    if (image.loadFromMemory(image_buffer.data(), image_buffer.size()))
    {
        default_scale.x = desired_image_width / image.getSize().x;
        default_scale.y = desired_image_height / image.getSize().y;
        sprite.setTexture(image);
        sprite.setScale(default_scale);
        has_image = true;
    }
}


void CollectionElement::EnhanceScale(const sf::Vector2f& factors)
{
    sf::Vector2f new_scale(default_scale.x * factors.x, default_scale.y * factors.y);
    sprite.setScale(new_scale);
}

void CollectionElement::ResetScale()
{
    sprite.setScale(default_scale);
}

void CollectionElement::Draw(const sf::Vector2f& position)
{
    if (has_image)
    {
        sprite.setPosition(position);
        window.draw(sprite);
    }
    else
    {
        text.setPosition(position);
        window.draw(text);
    }
}

Collection::Collection(const std::string& title) :
    title(title)
{}

void Collection::AddElement(const CollectionElement& element)
{
    elements.push_back(element);
}

std::string Collection::GetTitle() const
{
    return title;
}

size_t Collection::GetElementCount() const
{
    return elements.size();
}

const CollectionElement& Collection::GetElement(size_t index) const
{
    return elements.at(index);
}

}

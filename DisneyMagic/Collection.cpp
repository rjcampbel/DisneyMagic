#include "Collection.h"
#include "CurlHelpers.h"
#include <iostream>
#include <exception>

namespace disneymagic
{

CollectionElement::CollectionElement(
    const std::string& title,
    const std::string& image_url,
    double desired_image_width,
    double desired_image_height,
    sf::RenderWindow& window,
    const sf::Font& font) :
        image(),
        sprite(),
        text(title, font, 24),
        window(window),
        has_image(false),
        default_scale()
{
    text.setFillColor(sf::Color::White);

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

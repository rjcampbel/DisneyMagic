#include "Collection.h"
#include "CurlHelpers.h"
#include <iostream>

namespace disneymagic
{

CollectionElement::CollectionElement(
    const std::string& title, 
    const std::string& image_url, 
    double desired_image_width,
    double desired_image_height,
    sf::RenderWindow& window, 
    const sf::Font& font) :
        sprite(),
        text(),
        window(window),
        has_image(false),
        default_scale()
{
    std::string image_buffer;
    if (curlhelpers::retrieve_file_from_URL(image_url.c_str(), image_buffer) == CURLE_OK)
    {
        if (image.loadFromMemory(image_buffer.data(), image_buffer.size()))
        {
            sprite.setTexture(image);
            has_image = true;
        }
    }
    
    default_scale.x = desired_image_width / image.getSize().x;
    default_scale.y = desired_image_height / image.getSize().y;
    sprite.setScale(default_scale);
    text.setString(title);
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
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

sf::Vector2f CollectionElement::GetSize() const
{
    if (has_image)
    {
        return sf::Vector2f(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
    }
    else
    {
        return sf::Vector2f(text.getGlobalBounds().width, text.getGlobalBounds().height);
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
    return elements[index];
}

}

#include "Collection.h"
#include "CurlHelpers.h"

namespace disneymagic
{

CollectionElement::CollectionElement(const std::string& title, const std::string& image_url, sf::RenderWindow& window, const sf::Font& font) :
    title(title),
    image_url(image_url),
    window(window),
    font(font),
    has_image(false)
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

    text.setString(title);
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
}

void CollectionElement::SetScale(const sf::Vector2f& factors)
{
    sprite.setScale(factors);
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

CollectionElement Collection::GetElement(size_t index) const
{
    return elements[index];
}

}

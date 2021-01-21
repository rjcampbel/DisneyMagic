#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "CurlHelpers.h"

namespace disneymagic
{
class CollectionElement
{
public:
    CollectionElement(const std::string& title, const std::string& image_url, sf::RenderWindow& window, const sf::Font& font);

    void SetScale(const sf::Vector2f& factors);
    void Draw(const sf::Vector2f& position);
    sf::Vector2f GetSize() const;

private:
    sf::Texture image;
    sf::Sprite sprite;
    sf::Text text;
    sf::RenderWindow& window;
    bool has_image;
};

class Collection
{
public:
    Collection(const std::string& title);

    void AddElement(const CollectionElement& element);
    std::string GetTitle() const;
    size_t GetElementCount() const;
    const CollectionElement& GetElement(size_t index) const;

private:
    std::string title;
    std::vector<CollectionElement> elements;
};

}
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "ResourcePath.hpp"
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <rapidjson/document.h>
#include <algorithm>

size_t write_data(char *data, size_t memberSize, size_t memberCount, std::string *destination)
{
    size_t size = memberSize * memberCount;
    destination->append(data, size);
    return size;
}

int retrieve_file_from_URL(const std::string& url, std::string& fileBuffer)
{
    CURL *curl = curl_easy_init();
    if (curl != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return res;
    }
    return 1;
}

static const sf::Vector2f kDefaultImageScale(0.6f, 0.6f);
static const sf::Vector2f kEnhancedImageScale(0.62f, 0.62f);

class CollectionElement
{
public:
    CollectionElement(const std::string& title, const std::string& image_url, sf::RenderWindow& window, const sf::Font& font) :
        title(title),
        image_url(image_url),
        window(window),
        font(font),
        has_image(false)
    {
        std::string image_buffer;
        if (retrieve_file_from_URL(image_url.c_str(), image_buffer) == CURLE_OK)
        {
            if (image.loadFromMemory(image_buffer.data(), image_buffer.size()))
            {
                sprite.setTexture(image);
                sprite.setScale(kDefaultImageScale);
                has_image = true;
            }
        }

        text.setString(title);
        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
    }

    void Scale(const sf::Vector2f& factors)
    {
        sprite.setScale(factors);
    }

    void ResetScale()
    {
        sprite.setScale(kDefaultImageScale);
    }

    void Draw(const sf::Vector2f& position)
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

    sf::Vector2f GetSize() const
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

private:
    const std::string title;
    std::string image_url;
    sf::Texture image;
    sf::Sprite sprite;
    sf::Text text;
    sf::RenderWindow& window;
    const sf::Font& font;
    bool has_image;
};

class Collection
{
public:
    Collection(const std::string& title) : 
        title(title)
    {}

    void AddElement(const CollectionElement& element)
    {
        elements.push_back(element);
    }

    std::string GetTitle() const
    {
        return title;
    }

    size_t GetElementCount() const
    {
        return elements.size();
    }

    CollectionElement GetElement(size_t index) const
    {
        return elements[index];
    }

private:
    std::string title;
    std::vector<CollectionElement> elements;
};

int main()
{
    std::string homeApiUrl("https://cd-static.bamgrid.com/dp-117731241344/home.json");
    std::string homeApiContents;
    if (retrieve_file_from_URL(homeApiUrl, homeApiContents) != 0)
    {
        return EXIT_FAILURE;
    }
 
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Disney+");

    // Set the Icon
    sf::Image icon;
    if (!icon.loadFromFile(resourcePath() + "DisneyPlus.png")) 
    {
        return EXIT_FAILURE;
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // Create a graphical text to display
    sf::Font font;
    if (!font.loadFromFile(resourcePath() + "Avenir.ttc")) 
    {
        return EXIT_FAILURE;
    }

    rapidjson::Document apiDoc;
    apiDoc.Parse(homeApiContents.c_str());
    auto& data = apiDoc["data"];
    auto& collection = data["StandardCollection"];
    auto& containers = collection["containers"];

    std::vector<Collection> collections;
    for (const auto& container : containers.GetArray())
    {
        auto& collection_set = container["set"];
        auto& collection_type = collection_set["type"];
        if (strcmp(collection_type.GetString(), "SetRef") != 0)
        {
            auto& collection_text = collection_set["text"];
            auto& collection_title = collection_text["title"];
            auto& collection_full = collection_title["full"];
            auto& collection_title_set = collection_full["set"];
            auto& collection_title_default = collection_title_set["default"];
            auto& collection_title_content = collection_title_default["content"];

            collections.emplace_back(Collection(collection_title_content.GetString()));
            auto& items = collection_set["items"];
            for (const auto& item : items.GetArray())
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
                else
                {
                    std::cout << "Unknown type" << std::endl;
                }

                auto& text = item["text"];
                auto& title = text["title"];
                auto& full = title["full"];
                auto& title_type = full[title_type_string.c_str()];
                auto& title_default = title_type["default"];
                auto& title_content = title_default["content"];

                auto& image = item["image"];
                auto& tile_image = image["tile"];
                auto& image_element = tile_image["1.78"]; // ASSUMPTION: every "tile" contains an image with aspect ratio 1.78
                auto& image_series = image_element[image_type_string.c_str()];
                auto& image_default = image_series["default"];
                auto& image_url = image_default["url"];

                CollectionElement element(title_content.GetString(), image_url.GetString(), window, font);
                collections.back().AddElement(element);
            }
        }
    }

    int cursor_position { 0 };

    const size_t max_row_tile_count { 4 };
    
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed) 
            {
                window.close();
            }

            // Escape pressed: exit
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                    case sf::Keyboard::Escape:
                    {
                        window.close();
                        break;
                    }
                    case sf::Keyboard::Left:
                    {
                        if (cursor_position % max_row_tile_count > 0)
                        {
                            --cursor_position;
                        }
                        break;
                    }
                    case sf::Keyboard::Right:
                    {
                        if (cursor_position % max_row_tile_count < (max_row_tile_count - 1) )
                        {
                            ++cursor_position;
                        }
                        break;
                    }
                    case sf::Keyboard::Up:
                    {
                        if (cursor_position / max_row_tile_count > 0)
                        {
                            cursor_position -= max_row_tile_count;
                        }
                        break;
                    }
                    case sf::Keyboard::Down:
                    {
                        if (cursor_position / max_row_tile_count < (max_row_tile_count - 1))
                        {
                            cursor_position += max_row_tile_count;
                        }
                    }
                    default: break;
                }
            }
        }

        // Clear screen
        window.clear();

        size_t collection_index { 0 };
        const double row_offset { 10 };
        const double row_size { 250 };
        const double column_offset { 10 };

        for (auto& collection : collections)
        {
            double collection_row { row_offset + collection_index * row_size };
            double font_size { 24 };
            sf::Text collection_text(collection.GetTitle().c_str(), font, font_size);
            collection_text.setFillColor(sf::Color::White);
            collection_text.setPosition(column_offset, collection_row);
            window.draw(collection_text);

            const double element_width{ 325 };
            for (size_t element_index = 0; element_index < std::min(max_row_tile_count, collection.GetElementCount()); ++element_index)
            {
                double element_row { collection_row + font_size + 10 };
                double element_column { column_offset + element_index * element_width };
                auto element = collection.GetElement(element_index);

                if (cursor_position == collection_index * max_row_tile_count + element_index)
                {
                    element.Scale(kEnhancedImageScale);
                    
                    sf::RectangleShape selection_rect;
                    selection_rect.setFillColor(sf::Color::Transparent);
                    selection_rect.setOutlineColor(sf::Color::White);
                    selection_rect.setOutlineThickness(5.0f);
                    selection_rect.setSize(element.GetSize());   
                    selection_rect.setPosition(element_column, element_row);
                    window.draw(selection_rect);
                }
                else
                {
                    element.ResetScale();
                }
                
                element.Draw(sf::Vector2f(element_column, element_row));
            }
            collection_index++;
        }

        // Update the window
        window.display();
    }

    return EXIT_SUCCESS;
}

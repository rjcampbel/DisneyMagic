#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "ResourcePath.hpp"
#include "CurlHelpers.h"
#include "Collection.h"
#include <iostream>
#include <string>
#include <rapidjson/document.h>
#include <algorithm>

static const sf::Vector2f kDefaultImageScale(0.6f, 0.6f);
static const sf::Vector2f kEnhancedImageScale(0.62f, 0.62f);
static const size_t max_row_tile_count { 4 };
static const double row_offset { 10 };
static const double row_size { 250 };
static const double column_offset { 10 };
static const double font_size { 24 };
static const double tile_width{ 325 };

int main()
{
    std::string homeApiUrl("https://cd-static.bamgrid.com/dp-117731241344/home.json");
    std::string homeApiContents;
    if (curlhelpers::retrieve_file_from_URL(homeApiUrl, homeApiContents) != 0)
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

    std::vector<disneymagic::Collection> collections;
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

            collections.emplace_back(disneymagic::Collection(collection_title_content.GetString()));
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

                disneymagic::CollectionElement element(title_content.GetString(), image_url.GetString(), window, font);
                collections.back().AddElement(element);
            }
        }
    }

    int cursor_position { 0 };
   
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) 
            {
                window.close();
            }

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

        // Render row titles, tiles, and cursor
        size_t collection_index { 0 };
        for (auto& collection : collections)
        {
            double collection_row { row_offset + collection_index * row_size };

            // Render the title for current row
            sf::Text collection_text(collection.GetTitle().c_str(), font, font_size);
            collection_text.setFillColor(sf::Color::White);
            collection_text.setPosition(column_offset, collection_row);
            window.draw(collection_text);

            // Render the tiles and selection cursor
            for (size_t tile_index = 0; tile_index < std::min(max_row_tile_count, collection.GetElementCount()); ++tile_index)
            {
                double tile_row { collection_row + font_size + 10 };
                double tile_column { column_offset + tile_index * tile_width };
                auto element = collection.GetElement(tile_index);

                if (cursor_position == collection_index * max_row_tile_count + tile_index)
                {
                    element.SetScale(kEnhancedImageScale);
                    
                    sf::RectangleShape selection_rect;
                    selection_rect.setFillColor(sf::Color::Transparent);
                    selection_rect.setOutlineColor(sf::Color::White);
                    selection_rect.setOutlineThickness(5.0f);
                    selection_rect.setSize(element.GetSize());   
                    selection_rect.setPosition(tile_column, tile_row);
                    window.draw(selection_rect);
                }
                else
                {
                    element.SetScale(kDefaultImageScale);
                }
                
                element.Draw(sf::Vector2f(tile_column, tile_row));
            }
            collection_index++;
        }

        // Update display
        window.clear();
        window.display();
    }

    return EXIT_SUCCESS;
}
